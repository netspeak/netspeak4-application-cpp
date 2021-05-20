#include "netspeak/model/RawRefResult.hpp"

#include <algorithm>
#include <memory>

#include "netspeak/util/Vec.hpp"


namespace netspeak {
namespace model {


bool RawRefResult::disjoint_with(const RawRefResult& other) const {
  if (empty() || other.empty()) {
    // trivially disjoint
    return true;
  }

  // This assumes that both this and other are slices of a common postlist. This
  // means that we can check whether they are disjoint by checking the the start
  // and the end of both.
  const auto& this_first = *refs().begin();
  const auto& this_last = *(refs().end() - 1);
  const auto& other_first = *other.refs().begin();
  const auto& other_last = *(other.refs().end() - 1);

  if (this_first < other_first) {
    // [this: first..last] [other: first..last]
    return this_last < other_first;
  } else {
    // [other: first..last] [this: first..last]
    return other_last < this_first;
  }
}

std::shared_ptr<RawRefResult> RawRefResult::merge(
    const RawRefResult& other) const {
  auto result = std::make_shared<RawRefResult>();

  // merge phrase refs
  std::set_union(refs().begin(), refs().end(), other.refs().begin(),
                 other.refs().end(), std::back_inserter(result->refs()));

  // merge unknown words
  util::vec_append(result->unknown_words(), unknown_words());
  util::vec_append(result->unknown_words(), other.unknown_words());
  std::sort(result->unknown_words().begin(), result->unknown_words().end());
  util::vec_sorted_filter_dups(result->unknown_words());

  return result;
}


} // namespace model
} // namespace netspeak
