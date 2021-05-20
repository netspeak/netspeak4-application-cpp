#ifndef NETSPEAK_MODEL_LENGTH_RANGE_HPP
#define NETSPEAK_MODEL_LENGTH_RANGE_HPP

#include <ostream>

namespace netspeak {
namespace model {


/**
 * A length range is an interval of lengths where both ends (min and max) are
 * inclusive.
 */
struct LengthRange {
public:
  /**
   * @brief Returns the minimum length of the range.
   *
   * If the minimum is \c UINT32_MAX, then this range is empty.
   */
  uint32_t min;
  /**
   * @brief Returns the maximum length of a phrase that can (theoretically) be
   * matched (= accepted) by this unit.
   *
   * If the maximum is 0, then this unit either doesn't accept any
   * phrases at all or only the empty phrase.
   *
   * If the maximum is \c UINT32_MAX, then the range is
   * unbounded.
   */
  uint32_t max;

  LengthRange() : min(UINT32_MAX), max(0) {}
  LengthRange(uint32_t min) : min(min), max(UINT32_MAX) {}
  LengthRange(uint32_t min, uint32_t max) : min(min), max(max) {}

  /**
   * @brief Returns whether this range is empty.
   *
   * An empty is range is the empty set. This means no length is part of the
   * range. In most cases, this means that no words/phrases of any length are
   * accepted. If the range describes a range of distances (e.g. to an element),
   * then the element cannot be reached.
   */
  bool empty() const {
    return min == UINT32_MAX;
  }

  /**
   * @brief Returns whether this range is unbounded.
   *
   * If a range is unbounded, it will accept all lengths >= its minimum.
   */
  bool unbounded() const {
    return min != UINT32_MAX && max == UINT32_MAX;
  }

  bool accepts(uint32_t length) const {
    return !empty() && min <= length && length <= max;
  }
};

LengthRange& operator&=(LengthRange& lhs, const LengthRange& rhs);
LengthRange& operator|=(LengthRange& lhs, const LengthRange& rhs);

LengthRange operator&(const LengthRange& lhs, const LengthRange& rhs);
LengthRange operator|(const LengthRange& lhs, const LengthRange& rhs);

std::ostream& operator<<(std::ostream& out, const LengthRange& range);


} // namespace model
} // namespace netspeak


#endif
