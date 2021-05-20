#ifndef NETSPEAK_UTIL_CHAIN_CUTTER_HPP
#define NETSPEAK_UTIL_CHAIN_CUTTER_HPP

#include <algorithm>
#include <vector>

#include "netspeak/util/Math.hpp"

namespace netspeak {
namespace util {


/**
 * @brief This will iterate all possible cuts of a chain with a given length.
 *
 * Example: For a chain of length 4 with 2 cuts, all possible cuts will be
 * iterated as follows: (In the chain diagram, 0 represents a unit of the chain
 * and | represents a cut.)
 *
 * [4, 0, 0]  ->  0 0 0 0 | |
 * [3, 1, 0]  ->  0 0 0 | 0 |
 * [3, 0, 1]  ->  0 0 0 | | 0
 * [2, 2, 0]  ->  0 0 | 0 0 |
 * [2, 1, 1]  ->  0 0 | 0 | 0
 * [2, 0, 2]  ->  0 0 | | 0 0
 * [1, 3, 0]  ->  0 | 0 0 0 |
 * [1, 2, 1]  ->  0 | 0 0 | 0
 * [1, 1, 2]  ->  0 | 0 | 0 0
 * [1, 0, 3]  ->  0 | | 0 0 0
 * [0, 4, 0]  ->  | 0 0 0 0 |
 * [0, 3, 1]  ->  | 0 0 0 | 0
 * [0, 2, 2]  ->  | 0 0 | 0 0
 * [0, 1, 3]  ->  | 0 | 0 0 0
 * [0, 0, 4]  ->  | | 0 0 0 0
 *
 * @example
 * ChainCutter cutter(cuts, length);
 * do {
 *   // use segments
 * } while (cutter.next());
 */
class ChainCutter {
private:
  size_t cuts_;
  size_t length_;
  std::vector<size_t> state_;
  std::vector<size_t> output_;

private:
  void set_output() {
    size_t i = 0;
    size_t sum = 0;
    for (const auto& s : state_) {
      if (s == 0) {
        sum++;
      } else {
        output_[i++] = sum;
        sum = 0;
      }
    }
    output_[i] = sum;
  }

public:
  ChainCutter() = delete;
  ChainCutter(size_t cuts, size_t length)
      : cuts_(cuts), length_(length), state_(), output_() {
    // How does this class work?
    //
    // Internally, we represent each unit of the chain with a 0 and each cut
    // with a 1 in an array. Our state array will then contain length-many 0s
    // and cuts-many 1s. If we then go through all unique permutations of the
    // state array, we get exactly what we want. (Just look at the above state
    // diagram for the example. Each state disgram is exactly the internal state
    // used to derive the output.)

    // initialize states
    for (size_t i = 0; i < length; i++) {
      state_.push_back(0);
    }
    for (size_t i = 0; i < cuts; i++) {
      state_.push_back(1);
    }

    // initialize output
    output_.reserve(cuts + 1);
    for (size_t i = 0; i <= cuts; i++) {
      output_.push_back(0);
    }
    set_output();
  }

  /**
   * @brief Returns the number of unique segments that will be iterated through.
   */
  size_t size() const {
    return binomial(cuts_ + length_, std::min(cuts_, length_));
  }

  /**
   * @brief Returns the current output of the cutter iterator.
   *
   * The output is not the position of each cut but length of each cut segment
   * of the chain. E.g. for 2 cuts and a chain length of 4, an output of `[1, 0,
   * 3]` means that the first segment is 1 unit long, the second 0 units, and
   * the third (and last) 3 units (`0 | | 0 0 0`).
   *
   * Sum of all elements in the output will always be equal to the length of the
   * chain.
   *
   * @return const std::vector<size_t>&
   */
  const std::vector<size_t>& segments() const {
    return output_;
  }

  /**
   * @brief Returns whether there is another unique cut and computes the next
   * cuts.
   */
  bool next() {
    bool n = std::next_permutation(state_.begin(), state_.end());
    set_output();
    return n;
  }
};


} // namespace util
} // namespace netspeak


#endif
