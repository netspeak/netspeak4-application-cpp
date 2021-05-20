#ifndef NETSPEAK_RETRIEVAL_STRATEGY_HPP
#define NETSPEAK_RETRIEVAL_STRATEGY_HPP

#include "netspeak/Configuration.hpp"
#include "netspeak/Properties.hpp"
#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/SearchOptions.hpp"

namespace netspeak {

/**
 * Primary template to extract the phrase frequency and phrase id from certain
 * index entry type used by specialized classes of \c RetrievalStrategy.
 */
template <typename IndexEntryT>
struct index_entry_traits {
  typedef IndexEntryT value_type;

  static bool equal(const value_type& a, const value_type& b);

  static uint32_t get_phrase_frequency(const value_type& value);

  static uint32_t get_phrase_id(const value_type& value);

  static size_t hash(const value_type& value);

  static void set_phrase_frequency(value_type& value, uint32_t frequency);

  static void set_phrase_id(value_type& value, uint32_t id);
};

struct stats_type {
  stats_type()
      : eval_index_entry_count(),
        min_phrase_frequency(),
        max_phrase_frequency() {}
  uint32_t eval_index_entry_count;
  uint64_t min_phrase_frequency;
  uint64_t max_phrase_frequency;
  std::string unknown_word;
};

/**
 * Primary template to define the common interface of retrieval strategies.
 * Customized strategies have to specialize this template by providing a unique
 * type representing some RetrievalStrategyTag. This technique is inspired by
 * the STL's interator category tag classes.
 */
template <typename RetrievalStrategyTag>
class RetrievalStrategy {
public:
  void initialize(const Configuration& config);

  void initialize_query(
      const SearchOptions& options, const model::NormQuery& query,
      std::vector<typename RetrievalStrategyTag::unit_metadata>& metadata);

  template <typename OutputIterator>
  const stats_type initialize_result_set(
      const typename RetrievalStrategyTag::unit_metadata& unit_meta,
      const model::NormQuery& query, uint64_t max_phrase_frequency,
      uint64_t max_phrase_count, OutputIterator output);

  template <typename IntersectionSet, typename OutputIterator>
  const stats_type intersect_result_set(
      const IntersectionSet& input,
      const typename RetrievalStrategyTag::unit_metadata& unit_meta,
      const model::NormQuery& query, size_t max_phrase_frequency,
      size_t max_phrase_count, OutputIterator& output);

  Properties properties() const;
};

} // namespace netspeak

#endif // NETSPEAK_RETRIEVAL_STRATEGY_HPP
