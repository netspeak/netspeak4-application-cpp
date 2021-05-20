#ifndef NETSPEAK_RETRIEVAL_STRATEGY_3_HPP
#define NETSPEAK_RETRIEVAL_STRATEGY_3_HPP

#include <memory>
#include <string>

#include "netspeak/PhraseDictionary.hpp"
#include "netspeak/RetrievalStrategy.hpp"
#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/Searcher.hpp"
#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/SearchOptions.hpp"
#include "netspeak/value/pair.hpp"

namespace netspeak {

namespace bfs = boost::filesystem;
using namespace model;

struct RetrievalStrategy3Tag {
  /**
   * The entry type of the phrase index, which encodes the following values:
   * entry = ( n-gram-frequency, n-gram-id )
   */
  typedef value::pair<uint32_t, uint32_t> index_entry_type;
  struct unit_metadata {
    size_t position;
    uint64_t frequency;
    uint32_t pruning;

    bool operator<(const unit_metadata& rhs) const {
      return frequency < rhs.frequency;
    }
  };
};

/**
 * Specialized traits template for \c RetrievalStrategy3Tag.
 */
template <>
struct index_entry_traits<RetrievalStrategy3Tag> {
  typedef RetrievalStrategy3Tag::index_entry_type value_type;
  typedef value_type::e1_type phrase_frequency_type;
  typedef value_type::e2_type phrase_id_type;

  static bool equal(const value_type& lhs, const value_type& rhs) {
    return hash(lhs) == hash(rhs);
  }

  static phrase_frequency_type get_phrase_frequency(const value_type& value) {
    return value.e1();
  }

  static phrase_id_type get_phrase_id(const value_type& value) {
    return value.e2();
  }

  static size_t hash(const value_type& value) {
    return get_phrase_id(value);
  }

  static void set_phrase_frequency(value_type& value,
                                   phrase_frequency_type frequency) {
    value.set_e1(frequency);
  }

  static void set_phrase_id(value_type& value, phrase_id_type id) {
    value.set_e2(id);
  }
};

/**
 * Implements the 3rd strategy to lookup and intersect postlists, which seems
 * to be optimal and should be used for best performance (production service).
 *
 * Special to this version:
 * - There is only one phrase index for all (2-5)-grams.
 * - The n-gram length and word position is encoded in the key.
 *   Example: "2:0_hello" gets all 2-grams with "hello" at first position.
 * - Postlists can be read starting at any offset, so one can skip not relevant
 *   entries. The offset will be derived from some n-gram frequency threshold,
 *   for that reason the postlists are indexed as well (postlist_index_).
 * - For the very first retrieved postlist (see method initialize_result_set)
 *   we can determine such (jumpin-) frequency from the query.
 */
template <>
class RetrievalStrategy<RetrievalStrategy3Tag> {
public:
  typedef RetrievalStrategy3Tag::index_entry_type index_entry_type;
  typedef RetrievalStrategy3Tag::unit_metadata unit_metadata;
  typedef index_entry_traits<RetrievalStrategy3Tag> traits;

  void initialize(const Configuration& config) {
    // check config
    config.get_required_path(Configuration::PATH_TO_PHRASE_DICTIONARY);
    config.get_required_path(Configuration::PATH_TO_POSTLIST_INDEX);
    config.get_required_path(Configuration::PATH_TO_PHRASE_INDEX);

    // Open ngram dictionary.
    const bfs::path dir =
        config.get_required_path(Configuration::PATH_TO_PHRASE_DICTIONARY);
    phrase_dictionary_.reset(
        PhraseDictionary::Open(dir, util::memory_type::min_required));

    // Open postlist index.
    invertedindex::Configuration index_config;
    index_config.set_max_memory_usage(util::memory_type::mb1024);
    index_config.set_index_directory(
        config.get_required_path(Configuration::PATH_TO_POSTLIST_INDEX)
            .string());
    util::log("Open postlist index in", index_config.index_directory());
    postlist_index_.open(index_config);

    // Open ngram index.
    index_config.set_index_directory(
        config.get_required_path(Configuration::PATH_TO_PHRASE_INDEX).string());
    util::log("Open phrase index in", index_config.index_directory());
    phrase_index_.open(index_config);
  }

  void initialize_query(const SearchOptions& options, const NormQuery& query,
                        std::vector<unit_metadata>& metadata) {
    PhraseDictionary::Value freq_id_pair;
    for (size_t i = 0; i != query.size(); ++i) {
      const auto& unit = query.units()[i];
      if (unit.tag() == NormQuery::Unit::Tag::WORD) {
        // Note that words not being in the ngram dictionary can still
        // have a postlist in the ngram index, so this no reason to
        // skip the entire query.
        metadata.push_back(unit_metadata());
        auto& meta = metadata[metadata.size() - 1];
        meta.position = i;

        // TODO: This operation is extremely costly and should be replaced by an
        // in-memory map of all words
        if (phrase_dictionary_->Get(*unit.text(), freq_id_pair)) {
          meta.frequency = freq_id_pair.e1();
        }
        // TODO: Better stopword detection
        if (meta.frequency > 1000000000u) { // is stopword
          meta.pruning = options.pruning_high;
        } else {
          meta.pruning = options.pruning_low;
        }
      }
    }
  }

  template <typename OutputIterator>
  const stats_type initialize_result_set(const unit_metadata& meta,
                                         const NormQuery& query,
                                         uint64_t max_phrase_frequency,
                                         uint64_t max_phrase_count,
                                         OutputIterator output) {
    max_phrase_frequency =
        std::min(max_phrase_frequency, compute_jumpin_frequency_(query));

    stats_type stats;
    std::shared_ptr<invertedindex::Postlist<index_entry_type> > postlist(
        search_(make_key(query, meta), max_phrase_frequency, meta.pruning));
    if (!postlist) {
      stats.unknown_word = *(query.units()[meta.position].text());
      return stats;
    }

    // The first of the following two _while_ loops has two tasks:
    // (1) Skip values of the postlist with invalid frequency
    // (2) Select the frequency of the first valid _index_entry_

    index_entry_type index_entry;
    while (postlist->next(index_entry) && max_phrase_count != 0) {
      // Depending on the resolution of the postlist index,
      // search_() can only roughly satisfy the _max_freq_ condition,
      // so we have to check this condition here again.
      ++stats.eval_index_entry_count;
      if (traits::get_phrase_frequency(index_entry) <= max_phrase_frequency) {
        --max_phrase_count;
        stats.max_phrase_frequency = traits::get_phrase_frequency(index_entry);
        *output = index_entry;
        ++output;
        break;
      }
    }
    // Copy all remaining index entries.
    while (postlist->next(index_entry) && max_phrase_count != 0) {
      --max_phrase_count;
      ++stats.eval_index_entry_count;
      *output = index_entry;
      ++output;
    }
    stats.min_phrase_frequency = traits::get_phrase_frequency(index_entry);
    return stats;
  }

  template <typename IntersectionSet, typename OutputIterator>
  const stats_type intersect_result_set(const IntersectionSet& input,
                                        const unit_metadata& meta,
                                        const NormQuery& query,
                                        size_t max_phrase_frequency,
                                        size_t max_phrase_count,
                                        OutputIterator output) {
    stats_type stats;
    std::shared_ptr<invertedindex::Postlist<index_entry_type> > postlist =
        search_(make_key(query, meta), max_phrase_frequency, meta.pruning);
    if (!postlist) {
      stats.unknown_word = *(query.units()[meta.position].text());
      return stats;
    }

    // The first of the following two _while_ loops has two reasons:
    // (1) Skip values of the postlist with invalid frequency
    // (2) Select the frequency of the first valid _index_entry_

    index_entry_type index_entry;
    while (postlist->next(index_entry) && max_phrase_count != 0) {
      // Depending on the resolution of the postlist index,
      // search_() can only roughly satisfy the _max_freq_ condition,
      // so we have to check this condition here again.
      ++stats.eval_index_entry_count;
      if (traits::get_phrase_frequency(index_entry) <= max_phrase_frequency &&
          input.find(index_entry) != input.end()) {
        --max_phrase_count;
        stats.max_phrase_frequency = traits::get_phrase_frequency(index_entry);
        *output = index_entry;
        ++output;
        break;
      }
    }
    // Copy all remaining index entries.
    while (postlist->next(index_entry) && max_phrase_count != 0) {
      ++stats.eval_index_entry_count;
      if (input.find(index_entry) != input.end()) {
        --max_phrase_count;
        *output = index_entry;
        ++output;
      }
    }
    stats.min_phrase_frequency = traits::get_phrase_frequency(index_entry);
    return stats;
  }

  Properties properties() const {
    // Should insert phrase_index and postlist_index
    // properties as defined in Properties.hpp.
    Properties properties;
    properties[Properties::phrase_index_value_type] =
        phrase_index_.properties().value_type;
    properties[Properties::phrase_index_key_count] =
        util::to_string(phrase_index_.properties().key_count);
    properties[Properties::phrase_index_value_sorting] =
        util::to_string(phrase_index_.properties().value_sorting);
    properties[Properties::phrase_index_value_count] =
        util::to_string(phrase_index_.properties().value_count);
    properties[Properties::phrase_index_total_size] =
        util::to_string(phrase_index_.properties().total_size);

    properties[Properties::postlist_index_value_type] =
        postlist_index_.properties().value_type;
    properties[Properties::postlist_index_key_count] =
        util::to_string(postlist_index_.properties().key_count);
    properties[Properties::postlist_index_value_sorting] =
        util::to_string(postlist_index_.properties().value_sorting);
    properties[Properties::postlist_index_value_count] =
        util::to_string(postlist_index_.properties().value_count);
    properties[Properties::postlist_index_total_size] =
        util::to_string(postlist_index_.properties().total_size);
    return properties;
  }

private:
  static const std::string make_key(const NormQuery& query,
                                    const unit_metadata& meta) {
    std::ostringstream oss;
    oss << query.size() << ':' << meta.position << '_'
        << *(query.units()[meta.position].text());
    return oss.str();
  }

  /**
   * @brief Returns a list of the longest substrings (plural!) of the query.
   *
   * Each of the longest substrings will be stored as a single string where
   * words are joined using a single space.
   *
   * E.g. The query `foo bar ? baz boo ? fuzz` will return
   * `["foo bar", "baz boo"]`.
   *
   * @param query
   * @return std::vector<std::string>
   */
  static std::vector<std::string> extract_longest_substrings(
      const NormQuery& query) {
    size_t max_word_count = 1;
    std::vector<std::string> substrings;
    std::vector<std::string> word_buffer;
    for (const auto& unit : query.units()) {
      if (unit.tag() == NormQuery::Unit::Tag::WORD) {
        // TODO: Can the empty case every happen?
        if (!unit.text()->empty()) {
          word_buffer.push_back(*unit.text());
        }
      } else {
        if (word_buffer.size() >= max_word_count) {
          const auto substring = boost::join(word_buffer, " ");
          if (word_buffer.size() > max_word_count) {
            max_word_count = word_buffer.size();
            substrings.clear();
          }
          substrings.push_back(substring);
        }
        word_buffer.clear();
      }
    }
    if (word_buffer.size() >= max_word_count) {
      const auto substring = boost::join(word_buffer, " ");
      if (word_buffer.size() > max_word_count) {
        substrings.clear();
      }
      substrings.push_back(substring);
    }
    return substrings;
  }

  uint64_t compute_jumpin_frequency_(const NormQuery& query) {
    PhraseDictionary::Value freq_id_pair;
    uint64_t min_frequency = std::numeric_limits<uint64_t>::max();
    for (const auto& substring : extract_longest_substrings(query)) {
      // Note that substrings (n-grams) not being in the ngram dictionary
      // can still be contained in the ngram index as (n+x)-grams.
      // Therefore this is no reason to skip the entire query.
      if (phrase_dictionary_->Get(substring, freq_id_pair)) {
        min_frequency = std::min(min_frequency, freq_id_pair.e1());
      }
    }
    return min_frequency;
  }

  std::shared_ptr<invertedindex::Postlist<index_entry_type> > search_(
      const std::string& key, size_t max_freq, uint32_t pruning) {
    // Get index_begin from max_freq (jumpin frequency).
    size_t idx_begin = 0;
    postlist_index_value_type prev_index_value;
    postlist_index_value_type cur_index_value;
    std::unique_ptr<invertedindex::Postlist<postlist_index_value_type> >
        meta_postlist(postlist_index_.search_postlist(key));
    if (meta_postlist) {
      while (meta_postlist->next(cur_index_value)) {
        if (cur_index_value.e2() == max_freq) {
          idx_begin = cur_index_value.e1();
          break;
        }
        if (cur_index_value.e2() < max_freq) {
          // choose the value before, otherwise
          // entries will be lost in some cases
          idx_begin = prev_index_value.e1();
          break;
        }
        prev_index_value = cur_index_value;
      }
    }

    // Read postlist from ngram index.
    return std::shared_ptr<invertedindex::Postlist<index_entry_type> >(
        phrase_index_.search_postlist(key, idx_begin, pruning));
  }

  typedef value::pair<uint32_t, uint32_t> postlist_index_value_type;

  std::unique_ptr<PhraseDictionary> phrase_dictionary_;
  invertedindex::Searcher<postlist_index_value_type> postlist_index_;
  invertedindex::Searcher<index_entry_type> phrase_index_;
};

} // namespace netspeak

#endif // NETSPEAK_RETRIEVAL_STRATEGY_3_HPP
