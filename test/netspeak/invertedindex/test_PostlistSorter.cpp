// test_PostlistSorter.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <memory>

#include <boost/test/unit_test.hpp>

#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/PostlistBuilder.hpp"
#include "netspeak/invertedindex/PostlistSorter.hpp"

namespace ai = netspeak::invertedindex;
namespace av = netspeak::value;

template <typename T, typename Comp>
void test_sort(size_t value_count) {
  typedef T value_type;
  typedef Comp compare_type;

  value_type value;
  std::vector<value_type> values;
  ai::PostlistBuilder<value_type> builder;
  for (unsigned i(0); i != value_count; ++i) {
    av::generator<value_type>::numbered(value, i);
    builder.push_back(value);
    values.push_back(value);
  }

  const auto postlist = builder.build();
  const auto sorted_postlist =
      ai::PostlistSorter<value_type, compare_type>::sort(*postlist, 10);
  std::sort(values.begin(), values.end(), compare_type());
  BOOST_REQUIRE_EQUAL(values.size(), sorted_postlist->size());
  for (unsigned i(0); i != values.size(); ++i) {
    BOOST_REQUIRE(sorted_postlist->next(value));
    BOOST_REQUIRE_EQUAL(values.at(i), value);
  }
}

// TEST CASES

template <typename T>
void run_test_case() {
  const size_t value_count_small_scale(10000);   // w/o swap file
  const size_t value_count_large_scale(3000000); // w/  swap file

  test_sort<T, std::less<T>>(value_count_small_scale);
  test_sort<T, std::greater<T>>(value_count_small_scale);

  test_sort<T, std::less<T>>(value_count_large_scale);
  test_sort<T, std::greater<T>>(value_count_large_scale);
}


BOOST_AUTO_TEST_SUITE(test_PostlistSorter);

using std::string;

// -----------------------------------------------------------------------------
// Test singles
// -----------------------------------------------------------------------------

#define RUN_SINGLE(t1)                     \
  BOOST_AUTO_TEST_CASE(test_single_##t1) { \
    run_test_case<t1>();                   \
  }

RUN_SINGLE(uint16_t)
RUN_SINGLE(uint32_t)
RUN_SINGLE(uint64_t)
RUN_SINGLE(float)
RUN_SINGLE(double)
RUN_SINGLE(string)

// -----------------------------------------------------------------------------
// Test pairs
// -----------------------------------------------------------------------------

#define RUN_PAIR(t1, t2)                         \
  BOOST_AUTO_TEST_CASE(test_pair_##t1##__##t2) { \
    run_test_case<av::pair<t1, t2>>();           \
  }

RUN_PAIR(uint16_t, uint16_t)
RUN_PAIR(uint32_t, uint32_t)
RUN_PAIR(uint64_t, uint64_t)
RUN_PAIR(float, float)
RUN_PAIR(double, double)
RUN_PAIR(string, string)

RUN_PAIR(uint16_t, string)
RUN_PAIR(uint32_t, double)
RUN_PAIR(uint64_t, float)
RUN_PAIR(float, uint64_t)
RUN_PAIR(double, uint32_t)
RUN_PAIR(string, uint16_t)

// -----------------------------------------------------------------------------
// Test triples
// -----------------------------------------------------------------------------

#define RUN_TRIPLE(t1, t2, t3)                              \
  BOOST_AUTO_TEST_CASE(test_triple__##t1##__##t2##__##t3) { \
    run_test_case<av::triple<t1, t2, t3>>();                \
  }

RUN_TRIPLE(uint16_t, uint16_t, uint16_t)
RUN_TRIPLE(uint32_t, uint32_t, uint32_t)
RUN_TRIPLE(uint64_t, uint64_t, uint64_t)
RUN_TRIPLE(float, float, float)
RUN_TRIPLE(double, double, double)
RUN_TRIPLE(string, string, string)

RUN_TRIPLE(uint16_t, string, uint16_t)
RUN_TRIPLE(uint32_t, double, uint32_t)
RUN_TRIPLE(uint64_t, float, uint64_t)
RUN_TRIPLE(float, uint64_t, float)
RUN_TRIPLE(double, uint32_t, double)
RUN_TRIPLE(string, uint16_t, string)

RUN_TRIPLE(uint16_t, uint16_t, string)
RUN_TRIPLE(uint32_t, uint32_t, double)
RUN_TRIPLE(uint64_t, uint64_t, float)
RUN_TRIPLE(float, float, uint64_t)
RUN_TRIPLE(double, double, uint32_t)
RUN_TRIPLE(string, string, uint16_t)


// -----------------------------------------------------------------------------
// Test quadrupels
// -----------------------------------------------------------------------------

#define RUN_QUADRUPLE(t1, t2, t3, t4)                                  \
  BOOST_AUTO_TEST_CASE(test_quadruple__##t1##__##t2##__##t3##__##t4) { \
    run_test_case<av::quadruple<t1, t2, t3, t4>>();                    \
  }

RUN_QUADRUPLE(uint16_t, uint16_t, uint16_t, uint16_t)
RUN_QUADRUPLE(uint32_t, uint32_t, uint32_t, uint32_t)
RUN_QUADRUPLE(uint64_t, uint64_t, uint64_t, uint64_t)
RUN_QUADRUPLE(float, float, float, float)
RUN_QUADRUPLE(double, double, double, double)
RUN_QUADRUPLE(string, string, string, string)

RUN_QUADRUPLE(uint16_t, string, uint16_t, uint16_t)
RUN_QUADRUPLE(uint32_t, double, uint32_t, uint32_t)
RUN_QUADRUPLE(uint64_t, float, uint64_t, uint64_t)
RUN_QUADRUPLE(float, uint64_t, float, float)
RUN_QUADRUPLE(double, uint32_t, double, double)
RUN_QUADRUPLE(string, uint16_t, string, string)

RUN_QUADRUPLE(uint16_t, uint16_t, string, uint16_t)
RUN_QUADRUPLE(uint32_t, uint32_t, double, uint32_t)
RUN_QUADRUPLE(uint64_t, uint64_t, float, uint64_t)
RUN_QUADRUPLE(float, float, uint64_t, float)
RUN_QUADRUPLE(double, double, uint32_t, double)
RUN_QUADRUPLE(string, string, uint16_t, string)

RUN_QUADRUPLE(uint16_t, uint16_t, uint16_t, string)
RUN_QUADRUPLE(uint32_t, uint32_t, uint32_t, double)
RUN_QUADRUPLE(uint64_t, uint64_t, uint64_t, float)
RUN_QUADRUPLE(float, float, float, uint64_t)
RUN_QUADRUPLE(double, double, double, uint32_t)
RUN_QUADRUPLE(string, string, string, uint16_t)

BOOST_AUTO_TEST_SUITE_END();
