// test_PostlistReader.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <memory>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/invertedindex/PostlistBuilder.hpp"
#include "netspeak/invertedindex/PostlistReader.hpp"
#include "netspeak/util/systemio.hpp"

namespace ai = netspeak::invertedindex;
namespace au = netspeak::util;
namespace av = netspeak::value;
namespace bfs = boost::filesystem;

template <typename T>
void test_io_with_empty_postlist() {
  typedef T value_type;

  // -------------------------------------------------------------------------
  // Create empty postlist
  // -------------------------------------------------------------------------
  ai::PostlistBuilder<value_type> builder;
  auto postlist = builder.build();

  // -------------------------------------------------------------------------
  // Write empty postlist to file (1000 times)
  // -------------------------------------------------------------------------
  const bfs::path tmp_path("test_io_with_empty_postlist");
  FILE* tmp_fs(au::fopen(tmp_path, "wb+"));

  const unsigned num(1000);
  for (unsigned i(0); i != num; ++i) {
    postlist->write(tmp_fs);
  }
  BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), num * sizeof(postlist->head()));

  // -------------------------------------------------------------------------
  // Read empty postlist from file (1000 times)
  // -------------------------------------------------------------------------
  au::rewind(tmp_fs);
  for (unsigned i(0); i != num; ++i) {
    postlist = ai::PostlistReader<value_type>::read(tmp_path, tmp_fs);
  }
  BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), num * sizeof(postlist->head()));

  // -------------------------------------------------------------------------
  // Read empty postlist from file (1000 times with index_begin jump-in)
  // -------------------------------------------------------------------------
  au::rewind(tmp_fs);
  for (unsigned i(0); i != num; ++i) {
    postlist = ai::PostlistReader<value_type>::read(tmp_path, tmp_fs, 100);
  }
  BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), num * sizeof(postlist->head()));

  au::fclose(tmp_fs);
  bfs::remove(tmp_path);
}

template <typename T>
void test_io_with_full_postlist(size_t value_count) {
  typedef T value_type;

  // -------------------------------------------------------------------------
  // Create 10 postlists and write them to file
  // -------------------------------------------------------------------------
  const bfs::path tmp_path("test_io_with_full_postlist_reading");
  FILE* tmp_fs(au::fopen(tmp_path, "wb+"));

  const unsigned num(10);
  value_type actual_value;
  value_type expected_value;
  size_t expected_file_offset(0);
  ai::PostlistBuilder<value_type> builder;
  for (unsigned i(0); i != num; ++i) {
    for (unsigned j(0); j != value_count; ++j) {
      av::generator<value_type>::numbered(actual_value, i * num + j);
      builder.push_back(actual_value);
    }
    const auto plist = builder.build();
    expected_file_offset += plist->byte_size();
    plist->write(tmp_fs);
    BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), expected_file_offset);
  }

  // -------------------------------------------------------------------------
  // Read 10 full postlist from file and check their values
  // Range: [0, unlimited)
  // -------------------------------------------------------------------------
  au::rewind(tmp_fs);
  expected_file_offset = 0;
  for (unsigned i(0); i != num; ++i) {
    const auto plist = ai::PostlistReader<value_type>::read(tmp_path, tmp_fs);
    for (unsigned j(0); j != value_count; ++j) {
      av::generator<value_type>::numbered(expected_value, i * num + j);
      BOOST_REQUIRE(plist->next(actual_value));
      BOOST_REQUIRE_EQUAL(actual_value, expected_value);
    }
    BOOST_REQUIRE(!plist->next(actual_value));
    expected_file_offset += plist->byte_size();
    BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), expected_file_offset);
  }

  au::fclose(tmp_fs);
  bfs::remove(tmp_path);
}

template <typename T>
void test_io_with_partial_postlist(size_t value_count) {
  typedef T value_type;

  // -------------------------------------------------------------------------
  // Create 10 postlists and write them to file
  // -------------------------------------------------------------------------
  const bfs::path tmp_path("test_io_with_partial_postlist_reading");
  FILE* tmp_fs(au::fopen(tmp_path, "wb+"));

  const unsigned num(10);
  value_type actual_value;
  value_type expected_value;
  size_t expected_file_offset(0);
  ai::PostlistBuilder<value_type> builder;
  for (unsigned i(0); i != num; ++i) {
    for (unsigned j(0); j != value_count; ++j) {
      av::generator<value_type>::numbered(actual_value, i * num + j);
      builder.push_back(actual_value);
    }
    const auto plist = builder.build();
    expected_file_offset += plist->byte_size();
    plist->write(tmp_fs);
    BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), expected_file_offset);
  }

  // -------------------------------------------------------------------------
  // Read 10 partial postlist from file and check their values
  // Range: [begin, value_count)
  // -------------------------------------------------------------------------
  au::rewind(tmp_fs);
  const size_t begin(value_count / 2);
  for (unsigned i(0); i != num; ++i) {
    const auto plist =
        ai::PostlistReader<value_type>::read(tmp_path, tmp_fs, begin);
    for (unsigned j(begin); j != value_count; ++j) {
      av::generator<value_type>::numbered(expected_value, i * num + j);
      BOOST_REQUIRE(plist->next(actual_value));
      BOOST_REQUIRE_EQUAL(actual_value, expected_value);
    }
    BOOST_REQUIRE(!plist->next(actual_value));
  }
  BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), expected_file_offset);

  // -------------------------------------------------------------------------
  // Read 10 partial postlist from file and check their values
  // Range: [begin, begin + len)
  // -------------------------------------------------------------------------
  au::rewind(tmp_fs);
  const size_t len(value_count / 4);
  for (unsigned i(0); i != num; ++i) {
    const auto plist =
        ai::PostlistReader<value_type>::read(tmp_path, tmp_fs, begin, len);
    for (unsigned j(begin); j != begin + len; ++j) {
      av::generator<value_type>::numbered(expected_value, i * num + j);
      BOOST_REQUIRE(plist->next(actual_value));
      BOOST_REQUIRE_EQUAL(actual_value, expected_value);
    }
    BOOST_REQUIRE(!plist->next(actual_value));
  }
  BOOST_REQUIRE_EQUAL(au::ftell(tmp_fs), expected_file_offset);

  au::fclose(tmp_fs);
  bfs::remove(tmp_path);
}

template <typename T>
void run_test_case() {
  const size_t value_count_small_scale(10000);   // w/o swap file
  const size_t value_count_large_scale(3000000); // w/  swap file

  test_io_with_empty_postlist<T>();

  test_io_with_full_postlist<T>(value_count_small_scale);
  test_io_with_full_postlist<T>(value_count_large_scale);

  test_io_with_partial_postlist<T>(value_count_small_scale);
  test_io_with_partial_postlist<T>(value_count_large_scale);
}


BOOST_AUTO_TEST_SUITE(test_PostlistReader);

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
