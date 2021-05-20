#ifndef NETSPEAK_MODEL_SEARCH_OPTIONS_HPP
#define NETSPEAK_MODEL_SEARCH_OPTIONS_HPP

#include <cstdint>


namespace netspeak {
namespace model {


struct SearchOptions {
public:
  uint32_t max_phrase_count;
  uint64_t max_phrase_frequency;

  uint32_t phrase_length_min;
  uint32_t phrase_length_max;

  uint32_t pruning_high;
  uint32_t pruning_low;

  bool operator==(const SearchOptions& rhs) const {
    return max_phrase_count == rhs.max_phrase_count &&
           max_phrase_frequency == rhs.max_phrase_frequency &&
           phrase_length_min == rhs.phrase_length_min &&
           phrase_length_max == rhs.phrase_length_max &&
           pruning_high == rhs.pruning_high && pruning_low == rhs.pruning_low;
  }
  bool operator!=(const SearchOptions& rhs) const {
    return !(*this == rhs);
  }
};


} // namespace model
} // namespace netspeak


#endif
