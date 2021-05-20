#include <sstream>
#include <string>

#include <boost/test/unit_test.hpp>

#include "netspeak/util/ChainCutter.hpp"

namespace netspeak {

BOOST_AUTO_TEST_SUITE(ChainCutter)

BOOST_AUTO_TEST_CASE(test_output_size) {
  size_t cuts = 2;
  size_t length = 5;
  size_t expected_size = 21;

  util::ChainCutter cutter(cuts, length);
  BOOST_CHECK_EQUAL(cutter.size(), expected_size);

  std::vector<std::vector<size_t>> actual;
  do {
    actual.push_back(cutter.segments());
  } while (cutter.next());

  BOOST_CHECK_EQUAL(cutter.size(), actual.size());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
