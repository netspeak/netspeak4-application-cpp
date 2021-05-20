#include "netspeak/model/LengthRange.hpp"


namespace netspeak {
namespace model {


LengthRange& operator&=(LengthRange& lhs, const LengthRange& rhs) {
  // concatenation
  // This assumes that no overflows will occur
  if (lhs.empty()) {
    return lhs;
  }
  if (rhs.empty()) {
    lhs.min = UINT32_MAX;
    return lhs;
  }
  if (lhs.max == UINT32_MAX) {
    lhs.min += rhs.min;
    return lhs;
  }
  if (rhs.max == UINT32_MAX) {
    lhs.min += rhs.min;
    lhs.max = UINT32_MAX;
    return lhs;
  }
  lhs.min += rhs.min;
  lhs.max += rhs.max;
  return lhs;
}
LengthRange& operator|=(LengthRange& lhs, const LengthRange& rhs) {
  // union
  if (rhs.empty()) {
    return lhs;
  }
  if (lhs.empty()) {
    lhs = rhs;
    return lhs;
  }
  if (rhs.min < lhs.min) {
    lhs.min = rhs.min;
  }
  if (rhs.max > lhs.max) {
    lhs.max = rhs.max;
  }
  return lhs;
}

LengthRange operator&(const LengthRange& lhs, const LengthRange& rhs) {
  auto copy = lhs;
  copy |= rhs;
  return copy;
}
LengthRange operator|(const LengthRange& lhs, const LengthRange& rhs) {
  auto copy = lhs;
  copy |= rhs;
  return copy;
}

std::ostream& operator<<(std::ostream& out, const LengthRange& range) {
  if (range.empty()) {
    return out << "LengthRange(empty)";
  } else if (range.unbounded()) {
    return out << "LengthRange(" << range.min << ", unbounded)";
  } else {
    return out << "LengthRange(" << range.min << ", " << range.max << ")";
  }
}


} // namespace model
} // namespace netspeak
