#include <boost/test/unit_test.hpp>

#include "netspeak/model/Phrase.hpp"

namespace netspeak {

BOOST_AUTO_TEST_SUITE(test_phrase)

BOOST_AUTO_TEST_CASE(test_sort) {
  // sorted by descending frequency and ascending id
  model::Phrase a(model::Phrase::Id(3, 1), 1000);
  model::Phrase b(model::Phrase::Id(123, 3), 100);
  model::Phrase c(model::Phrase::Id(124, 3), 100);
  model::Phrase d(model::Phrase::Id(125, 3), 90);
  model::Phrase e(model::Phrase::Id(3, 4), 30);

  std::vector<model::Phrase> expected{ a, b, c, d, e };

  std::vector<model::Phrase> actual{ e, d, c, b, a };
  std::sort(actual.begin(), actual.end());

  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(),
                                expected.end());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
