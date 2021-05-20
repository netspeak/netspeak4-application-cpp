#ifndef NETSPEAK_QUERY_PROCESSOR_HPP
#define NETSPEAK_QUERY_PROCESSOR_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <unordered_set>
#include <vector>

#include "netspeak/Configuration.hpp"
#include "netspeak/RetrievalStrategy.hpp"
#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/RawRefResult.hpp"
#include "netspeak/model/SearchOptions.hpp"
#include "netspeak/util/check.hpp"
#include "netspeak/util/logging.hpp"

namespace netspeak {

using namespace model;

template <typename RetrievalStrategyTag>
class QueryProcessor {
private:
  typedef RetrievalStrategy<RetrievalStrategyTag> retrieval_strategy;
  typedef typename retrieval_strategy::index_entry_type index_entry_type;
  typedef index_entry_traits<RetrievalStrategyTag> traits;

  /**
   * A hash function object for hashing index entries.
   */
  struct hash : public std::unary_function<index_entry_type, size_t> {
    size_t operator()(const index_entry_type& entry) const {
      return traits::hash(entry);
    }
  };

  /**
   * A comparator function object for comparing ngram references.
   */
  struct equal
      : std::binary_function<index_entry_type, index_entry_type, bool> {
    bool operator()(const index_entry_type& a,
                    const index_entry_type& b) const {
      return traits::equal(a, b);
    }
  };

  typedef std::unordered_set<index_entry_type, hash, equal>
      intersection_set_type;

public:
  void initialize(const Configuration& config) {
    strategy_.initialize(config);
  }

  Properties properties() const {
    // Should return phrase_index and postlist_index
    // properties as defined in NetspeakProperties.hpp.
    return strategy_.properties();
  }

  std::shared_ptr<RawRefResult> process(const SearchOptions& options,
                                        const NormQuery& query) {
    auto query_result = std::make_shared<RawRefResult>();
    process_(options, *query_result, query);
    return query_result;
  }

private:
  void process_(const SearchOptions& options, RawRefResult& query_result,
                const NormQuery& query) {
    std::vector<typename RetrievalStrategyTag::unit_metadata> unit_metadata;
    strategy_.initialize_query(options, query, unit_metadata);
    std::sort(unit_metadata.begin(), unit_metadata.end());

    intersection_set_type src_set;
    intersection_set_type dst_set;
    intersection_set_type* src_set_ptr(&src_set);
    intersection_set_type* dst_set_ptr(&dst_set);

    std::vector<index_entry_type> index_entries;
    uint64_t cur_max_phrase_frequency = options.max_phrase_frequency;

    for (auto it = unit_metadata.begin(); it != unit_metadata.end(); ++it) {
      if (it == unit_metadata.begin()) {
        // this is the first word
        if (it == unit_metadata.end() - 1) {
          // ...and the last word
          const stats_type stats(strategy_.initialize_result_set(
              *it, query, cur_max_phrase_frequency, options.max_phrase_count,
              std::back_inserter(index_entries)));
          if (!stats.unknown_word.empty()) {
            query_result.unknown_words().push_back(stats.unknown_word);
          }
        } else {
          // ...but not the last word
          const stats_type stats(strategy_.initialize_result_set(
              *it, query, cur_max_phrase_frequency,
              std::numeric_limits<size_t>::max(),
              std::inserter(*src_set_ptr, src_set_ptr->end())));
          cur_max_phrase_frequency = stats.max_phrase_frequency;
          if (!stats.unknown_word.empty()) {
            query_result.unknown_words().push_back(stats.unknown_word);
          }
        }
      } else if (it == unit_metadata.end() - 1) {
        // perform last intersection and copy
        // matches directly into final result vector
        const stats_type stats(strategy_.intersect_result_set(
            *src_set_ptr, *it, query, cur_max_phrase_frequency,
            options.max_phrase_count, std::back_inserter(index_entries)));
        if (!stats.unknown_word.empty()) {
          query_result.unknown_words().push_back(stats.unknown_word);
        }
      } else {
        // perform intermediate intersection
        const stats_type stats(strategy_.intersect_result_set(
            *src_set_ptr, *it, query, cur_max_phrase_frequency,
            std::numeric_limits<size_t>::max(),
            std::inserter(*dst_set_ptr, dst_set_ptr->end())));
        cur_max_phrase_frequency = stats.max_phrase_frequency;
        if (!stats.unknown_word.empty()) {
          query_result.unknown_words().push_back(stats.unknown_word);
        }
        std::swap(src_set_ptr, dst_set_ptr);
        dst_set_ptr->clear();
      }
      if (src_set_ptr->empty())
        break;
    }

    for (const auto& index_entry : index_entries) {
      uint32_t freq = traits::get_phrase_frequency(index_entry);
      uint32_t id = traits::get_phrase_id(index_entry);
      query_result.refs().push_back(RawRefResult::Ref(id, freq));
    }
  }

private:
  retrieval_strategy strategy_;
};

} // namespace netspeak

#endif // NETSPEAK_QUERY_PROCESSOR_HPP
