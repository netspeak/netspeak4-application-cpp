#include "netspeak/Netspeak.hpp"

#include "boost/lexical_cast.hpp"

#include "netspeak/error.hpp"
#include "netspeak/util/Vec.hpp"


namespace netspeak {

namespace bfs = boost::filesystem;
using namespace model;

const std::string DEFAULT_REGEX_MAX_MATCHES = "100";
const std::string DEFAULT_REGEX_MAX_TIME = "20" /* ms */;
const std::string DEFAULT_CACHE_CAPCITY = "1000000";

Netspeak::search_config Netspeak::get_search_config(
    const Configuration& config) const {
  Netspeak::search_config sc = {
    // regex
    .regex_max_matches = boost::lexical_cast<size_t>(config.get(
        Configuration::SEARCH_REGEX_MAX_MATCHES, DEFAULT_REGEX_MAX_MATCHES)),
    .regex_max_time =
        std::chrono::milliseconds(boost::lexical_cast<size_t>(config.get(
            Configuration::SEARCH_REGEX_MAX_TIME, DEFAULT_REGEX_MAX_TIME))),
  };
  return sc;
}

void Netspeak::initialize(const Configuration& config) {
  search_config_ = get_search_config(config);

  const auto pc_dir =
      config.get_required_path(Configuration::PATH_TO_PHRASE_CORPUS);
  const auto pd_dir =
      config.get_required_path(Configuration::PATH_TO_PHRASE_DICTIONARY);
  const auto cache_cap =
      config.get(Configuration::CACHE_CAPACITY, DEFAULT_CACHE_CAPCITY);
  result_cache_.reserve(std::stoul(cache_cap));

  auto dir = bfs::path(pd_dir);
  util::log("Open phrase dictionary in", dir);
  phrase_dictionary_.reset(
      PhraseDictionary::Open(dir, util::memory_type::min_required));

  dir = bfs::path(pc_dir) / "bin";
  util::log("Open phrase corpus in", dir);
  phrase_corpus_.open(dir);

  // The hash dictionary is optional.
  hash_dictionary_ = std::make_shared<Dictionaries::Map>();
  auto sdd = config.get_optional_path(Configuration::PATH_TO_HASH_DICTIONARY);
  if (sdd && bfs::exists(*sdd)) {
    const bfs::directory_iterator end;
    for (bfs::directory_iterator it(*sdd); it != end; ++it) {
      util::log("Open hash dictionary in", *it);
      const auto dict = Dictionaries::read_from_file(*it);
      for (const auto& pair : dict) {
        hash_dictionary_->insert(pair);
      }
    }
  }

  // The regex vocabulary is optional.
  sdd = config.get_optional_path(Configuration::PATH_TO_REGEX_VOCABULARY);
  if (sdd && bfs::exists(*sdd)) {
    const bfs::directory_iterator end;
    for (bfs::directory_iterator it(*sdd); it != end; ++it) {
      util::log("Open regex vocabulary in", *it);
      bfs::ifstream ifs(*it);
      util::check(ifs.is_open(), error_message::cannot_open, *it);
      std::string regexwords((std::istreambuf_iterator<char>(ifs)),
                             (std::istreambuf_iterator<char>()));
      ifs.close();
      regex_index_ =
          std::make_shared<regex::DefaultRegexIndex>(std::move(regexwords));
      break;
    }
  }

  query_normalizer_ = QueryNormalizer({
      .regex_index = regex_index_,
      .dictionary = hash_dictionary_,
  });

  query_processor_.initialize(config);
}

Properties Netspeak::properties() const {
  // Should return phrase_index and postlist_index
  // properties as defined in Properties.hpp.
  auto properties = query_processor_.properties();

  // cache properties
  properties[Properties::cache_policy] = "least frequently used";
  properties[Properties::cache_size] = std::to_string(result_cache_.size());
  properties[Properties::cache_capacity] =
      std::to_string(result_cache_.capacity());
  properties[Properties::cache_access_count] =
      std::to_string(result_cache_.access_count());
  properties[Properties::cache_hit_rate] =
      std::to_string(result_cache_.hit_rate());
  std::ostringstream oss;
  result_cache_.list(oss, 100);
  properties[Properties::cache_top_100] = oss.str();

  // phrase corpus properties
  properties[Properties::phrase_corpus_1gram_count] =
      std::to_string(phrase_corpus_.count_phrases(1));
  properties[Properties::phrase_corpus_2gram_count] =
      std::to_string(phrase_corpus_.count_phrases(2));
  properties[Properties::phrase_corpus_3gram_count] =
      std::to_string(phrase_corpus_.count_phrases(3));
  properties[Properties::phrase_corpus_4gram_count] =
      std::to_string(phrase_corpus_.count_phrases(4));
  properties[Properties::phrase_corpus_5gram_count] =
      std::to_string(phrase_corpus_.count_phrases(5));

  // phrase dictionary properties
  properties[Properties::phrase_dictionary_size] =
      std::to_string(phrase_dictionary_->size());
  properties[Properties::phrase_dictionary_value_type] =
      value::value_traits<PhraseDictionary::Value>::type_name();

  // hash dictionary properties
  properties[Properties::hash_dictionary_size] =
      std::to_string(hash_dictionary_->size());
  properties[Properties::hash_dictionary_value_type] =
      value::value_traits<Dictionaries::Map::mapped_type>::type_name();

  // regex vocabulary properties
  // BEWARE: The index is optional!
  auto vocal_size = regex_index_ ? regex_index_->vocabulary().size() : 0;
  properties[Properties::regex_vocabulary_size] = std::to_string(vocal_size);
  properties[Properties::regex_vocabulary_value_type] =
      std::string(typeid(regex::DefaultRegexIndex).name());
  return properties;
}


service::Phrase::Word::Tag to_tag(const NormQuery::Unit& unit) {
  std::shared_ptr<const Query::Unit> source = unit.source().unit;
  while (source) {
    switch (source->tag()) {
      case Query::Unit::Tag::QMARK:
        return service::Phrase::Word::WORD_FOR_QMARK;
      case Query::Unit::Tag::STAR:
        return service::Phrase::Word::WORD_FOR_STAR;
      case Query::Unit::Tag::PLUS:
        return service::Phrase::Word::WORD_FOR_PLUS;
      case Query::Unit::Tag::REGEX:
        return service::Phrase::Word::WORD_FOR_REGEX;
      case Query::Unit::Tag::DICTSET:
        return service::Phrase::Word::WORD_IN_DICTSET;
      case Query::Unit::Tag::OPTIONSET:
        return service::Phrase::Word::WORD_IN_OPTIONSET;
      case Query::Unit::Tag::ORDERSET:
        return service::Phrase::Word::WORD_IN_ORDERSET;
      default:
        break;
    }
    source = source->parent();
  }
  return service::Phrase::Word::WORD;
}
/**
 * @brief Converts the given search result item into a (service) phrase.
 */
void set_response_phrase(service::Phrase& resp_phrase,
                         const SearchResult::Item& item) {
  resp_phrase.set_id(item.phrase.id());
  resp_phrase.set_frequency(item.phrase.freq());

  // words
  const auto& words = item.phrase.words();
  for (size_t i = 0; i != words.size(); i++) {
    auto resp_word = resp_phrase.add_words();
    resp_word->set_text(words[i]);
    resp_word->set_tag(to_tag(item.query->units()[i]));
  }
}

void Netspeak::search(const service::SearchRequest& request,
                      service::SearchResponse& response) throw() {
  try {
    // parse the query
    const auto query = antlr4::parse_query(request.query());

    // destruct the given request into options we can use
    const auto option_pair = to_options(request);
    const auto normalizer_options = option_pair.first;
    const auto search_options = option_pair.second;

    // perform the raw seach (returns phrases and phrase references)
    auto raw_result = search_raw_(normalizer_options, search_options, query);
    // resolve the phrase references and merge with the other phrases
    auto phrase_result = merge_raw_result_(search_options, *raw_result);

    // construct the result
    auto response_result = response.mutable_result();
    // add unknown words
    for (const auto& unknown : phrase_result->unknown_words()) {
      if (!phrase_corpus_.contains(unknown)) {
        response_result->add_unknown_words(unknown);
      }
    }
    // add phrases
    for (const auto& phrase : phrase_result->phrases()) {
      auto resp_phrase = response_result->add_phrases();
      set_response_phrase(*resp_phrase, phrase);
    }
  } catch (const invalid_query_error& e) {
    auto resp_error = response.mutable_error();
    resp_error->set_kind(service::SearchResponse::Error::INVALID_QUERY);
    resp_error->set_message(e.what());
  } catch (const std::logic_error& e) {
    auto resp_error = response.mutable_error();
    resp_error->set_kind(service::SearchResponse::Error::INTERNAL_ERROR);
    resp_error->set_message(e.what());
  } catch (const std::exception& e) {
    auto resp_error = response.mutable_error();
    resp_error->set_kind(service::SearchResponse::Error::UNKNOWN);
    resp_error->set_message(e.what());
  } catch (...) {
    auto resp_error = response.mutable_error();
    resp_error->set_kind(service::SearchResponse::Error::UNKNOWN);
    resp_error->set_message("Unknown error");
  }
}


std::pair<QueryNormalizer::Options, SearchOptions> Netspeak::to_options(
    const service::SearchRequest& request) {
  const auto& constraints = request.phrase_constraints();

  // The default min length is 1 because our indexes don't contain the empty
  // phrase
  uint32_t min_len = std::max(1U, constraints.words_min());

  // the maximum length of a phrase is given by the phrase corpus but may be
  // lowered by the search request.
  uint32_t max_len = phrase_corpus_.max_length();
  if (constraints.words_max() != 0 && constraints.words_max() < max_len) {
    max_len = constraints.words_max();
  }

  uint64_t max_freq = std::numeric_limits<uint64_t>::max();
  if (constraints.frequency_max() != 0) {
    max_freq = constraints.frequency_max();
  }

  SearchOptions s_options = {
    .max_phrase_count = request.max_phrases(),
    .max_phrase_frequency = max_freq,
    .phrase_length_min = min_len,
    .phrase_length_max = max_len,
    // TODO: Replace these with configurable values
    .pruning_high = 160000,
    .pruning_low = 130000,
  };

  QueryNormalizer::Options n_options = {
    // TODO: Replace this with configurable values
    .max_norm_queries = 10000,

    .min_length = min_len,
    .max_length = max_len,

    .max_regex_matches = search_config_.regex_max_matches,
    .max_regex_time = search_config_.regex_max_time,
  };

  return std::make_pair(n_options, s_options);
}


struct ref_ {
  std::shared_ptr<const NormQuery> query;
  Phrase::Id id;
  RawRefResult::Ref::Frequency freq;

  ref_() = delete;
  ref_(std::shared_ptr<const NormQuery> query, Phrase::Id id,
       RawRefResult::Ref::Frequency freq)
      : query(query), id(id), freq(freq) {}

  inline bool operator==(const ref_& rhs) const {
    return id == rhs.id;
  }
  inline bool operator!=(const ref_& rhs) const {
    return !(*this == rhs);
  }
  bool operator<(const ref_& rhs) const {
    if (freq != rhs.freq) {
      // sort by descending frequency
      return freq > rhs.freq;
    } else {
      return id < rhs.id;
    }
  }
};

std::unique_ptr<std::set<ref_>> merge_phrase_refs(
    const std::vector<RawResult::RefItem>& raw_references) {
  auto set = std::make_unique<std::set<ref_>>();

  for (const auto& ref_item : raw_references) {
    const auto& query = ref_item.query;
    Phrase::Id::Length len = query->size();
    for (const auto& raw_ref : ref_item.result->refs()) {
      set->insert(ref_(query, Phrase::Id(len, raw_ref.id()), raw_ref.freq()));
    }
  }

  return set;
}
std::unique_ptr<SearchResult> Netspeak::merge_raw_result_(
    const SearchOptions& options, const RawResult& raw_result) {
  auto search_result = std::make_unique<SearchResult>();
  util::vec_append(search_result->unknown_words(), raw_result.unknown_words());

  size_t max_phrase_count = options.max_phrase_count;
  if (max_phrase_count == 0) {
    return search_result;
  }

  // Extract the top k unique phrase refs.
  // TODO: This can done more efficiently
  const auto unique_refs = merge_phrase_refs(raw_result.refs());
  size_t top_k = std::min(unique_refs->size(), max_phrase_count);
  std::vector<ref_> top_k_refs;
  std::copy_n(unique_refs->begin(), top_k, std::back_inserter(top_k_refs));

  // Read all top k phrases found by wildcard queries.
  std::vector<Phrase::Id> tok_k_ref_ids;
  for (const auto& ref : top_k_refs) {
    tok_k_ref_ids.push_back(ref.id);
  }
  auto ref_phrases = phrase_corpus_.read_phrases(tok_k_ref_ids);

  // Copy all phrases into a final *sorted* merge set.
  std::set<SearchResult::Item> final_phrases;
  // phrases from references
  for (size_t i = 0; i != top_k_refs.size(); i++) {
    final_phrases.insert(
        SearchResult::Item(top_k_refs[i].query, ref_phrases[i]));
  }
  // phrases from the raw result set
  for (const auto& phrase_item : raw_result.phrases()) {
    const auto& query = phrase_item.query;
    for (const auto& phrase : phrase_item.result->phrases()) {
      final_phrases.insert(SearchResult::Item(query, phrase));
    }
  }

  // Copy the final top k phrases into the search result.
  top_k = std::min(final_phrases.size(), max_phrase_count);
  std::copy_n(final_phrases.begin(), top_k,
              std::back_inserter(search_result->phrases()));

  return search_result;
}


/**
 * @brief Returns a unique string representation for the given query.
 *
 * Note: This assumes that words do not contain spaces.
 */
std::string norm_query_to_key(const NormQuery& query) {
  std::string out;
  auto it = query.units().begin();
  auto end = query.units().end();
  if (it != end) {
    if (it->tag() == NormQuery::Unit::Tag::WORD) {
      out.append(*it->text());
    } else {
      out.push_back('?');
    }
    it++;
  }
  for (; it != end; it++) {
    out.push_back(' ');
    if (it->tag() == NormQuery::Unit::Tag::WORD) {
      out.append(*it->text());
    } else {
      out.push_back('?');
    }
  }
  return out;
}

bool is_prunable_from(const SearchOptions& superset,
                      const SearchOptions& options) {
  return superset.max_phrase_frequency == options.max_phrase_frequency &&
         superset.max_phrase_count >= options.max_phrase_count &&
         superset.phrase_length_min <= options.phrase_length_min &&
         superset.phrase_length_max >= options.phrase_length_max &&
         superset.pruning_low >= options.pruning_low &&
         superset.pruning_high >= options.pruning_high;
}
std::shared_ptr<const RawRefResult> prune(const RawRefResult& result,
                                          const SearchOptions& options) {
  auto pruned = std::make_shared<RawRefResult>();

  // Copy refs
  if (options.max_phrase_count > 0) {
    for (const auto& ref : result.refs()) {
      if (ref.freq() <= options.max_phrase_frequency) {
        pruned->refs().push_back(ref);
        if (pruned->refs().size() >= options.max_phrase_count) {
          break;
        }
      }
    }
  }

  // Copy unknown words
  util::vec_append(pruned->unknown_words(), result.unknown_words());

  return pruned;
}

std::shared_ptr<const RawRefResult> Netspeak::process_wildcard_query_(
    const SearchOptions& options, const NormQuery& query) {
  const auto query_key = norm_query_to_key(query);
  const auto cached_result = result_cache_.find(query_key);

  if (cached_result && cached_result->options == options) {
    // exact cache hit
    return cached_result->result;
  } else if (cached_result &&
             is_prunable_from(cached_result->options, options)) {
    // TODO: This very loose compatibility check may result in pathetically
    // small result sets.
    return prune(*cached_result->result, options);
  } else {
    // can't serve from cache
    auto final_result = query_processor_.process(options, query);
    if (cached_result && !final_result->disjoint_with(*cached_result->result)) {
      // extend the cached phrase refences

      // TODO: These cached reference lists may grow to enormous sizes. Let's
      // assume that someone wanted to get all results for the query `the ? ?`
      // and it's easy to see that the cached result set may be gigabytes in
      // size.
      auto merged = final_result->merge(*cached_result->result);
      auto item = std::make_shared<result_cache_item>(options, merged);
      result_cache_.update(query_key, item);
    } else {
      // add this to the cache
      auto item = std::make_shared<result_cache_item>(options, final_result);
      result_cache_.insert(query_key, item);
    }
    return final_result;
  }
}

std::shared_ptr<const RawPhraseResult> Netspeak::process_non_wildcard_query_(
    const SearchOptions& options, const NormQuery& query) {
  auto result = std::make_shared<RawPhraseResult>();

  PhraseDictionary::Value freq_id_pair;
  if (options.max_phrase_count > 0 &&
      phrase_dictionary_->Get(norm_query_to_key(query), freq_id_pair)) {
    // found the phrase
    uint64_t freq = freq_id_pair.e1();
    if (freq <= options.max_phrase_frequency) {
      Phrase::Id id(query.size(), freq_id_pair.e2());
      Phrase phrase(id, freq);
      for (const auto& unit : query.units()) {
        phrase.words().push_back(*unit.text());
      }
      result->phrases().push_back(std::move(phrase));
    }
  } else {
    // unknown phrase or no phrases requested (max_phrase_count == 0)
    // in this case, the query may contain unknown words that have to be
    // reported
    // TODO: Use the phrase corpus for that
    for (const auto& unit : query.units()) {
      const auto& text = *unit.text();
      if (!phrase_corpus_.contains(text)) {
        result->unknown_words().push_back(text);
      }
    }
  }
  return result;
}

std::unique_ptr<RawResult> Netspeak::search_raw_(
    const QueryNormalizer::Options& normalizer_options,
    const SearchOptions& options, std::shared_ptr<Query> query) {
  // create norm queries
  std::vector<NormQuery> normQueries;
  query_normalizer_.normalize(query, normalizer_options, normQueries);

  // process the norm queries
  auto result = std::make_unique<RawResult>();
  for (const auto& query : normQueries) {
    if (query.has_qmarks()) {
      result->add_item(query, process_wildcard_query_(options, query));
    } else {
      result->add_item(query, process_non_wildcard_query_(options, query));
    }
  }

  return result;
}


} // namespace netspeak
