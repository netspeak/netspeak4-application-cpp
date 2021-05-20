// test_ManagedIndexer.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <map>
#include <memory>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "../ManagedDirectory.hpp"

#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/InvertedFileWriter.hpp"
#include "netspeak/invertedindex/ManagedIndexer.hpp"
#include "netspeak/invertedindex/Searcher.hpp"

namespace aii = netspeak::invertedindex;
namespace av = netspeak::value;
namespace bfs = boost::filesystem;

template <typename T>
const std::multimap<std::string, T> write_inverted_file(
    const bfs::path& inv_file, size_t record_count,
    aii::key_sorting_type sorting) {
  // an in-memory record store for comparison
  std::multimap<std::string, T> expected_records;

  std::string key;
  T value;
  aii::Record<T> record;
  bfs::ofstream inv_file_ofs(inv_file);
  aii::InvertedFileWriter<T> writer(inv_file_ofs);
  for (unsigned i(0); i != record_count; ++i) {
    switch (sorting) {
      case aii::key_sorting_type::sorted:
        av::generator<std::string>::numbered(key, i / 100);
        break;
      case aii::key_sorting_type::unsorted:
        av::generator<std::string>::numbered(key, i % 100);
        break;
    }
    av::generator<T>::numbered(value, i % 100);
    record.set(key, value);
    BOOST_REQUIRE(writer.write(record));

    expected_records.insert(std::make_pair(record.key(), record.value()));
  }
  return expected_records;
}

template <typename T>
void build_inverted_index(const bfs::path& input_dir,
                          const bfs::path& index_dir,
                          aii::key_sorting_type sorting) {
  aii::Configuration config;
  config.set_input_directory(input_dir.string());
  config.set_index_directory(index_dir.string());
  config.set_key_sorting(sorting);

  aii::ManagedIndexer<T>::index(config);
}

template <typename T>
void check_inverted_index(
    const bfs::path& index_dir,
    const std::multimap<std::string, T>& expected_records) {
  aii::Configuration config;
  config.set_index_directory(index_dir.string());
  aii::Searcher<T> searcher(config);

  std::string current_key;
  T current_value;
  std::unique_ptr<aii::Postlist<T>> postlist;
  for (auto it(expected_records.begin()); it != expected_records.end(); ++it) {
    if (it->first != current_key) {
      current_key = it->first;
      postlist = searcher.search_postlist(current_key);
      BOOST_REQUIRE(postlist);
      BOOST_REQUIRE_EQUAL(expected_records.count(current_key),
                          postlist->size());
    }
    BOOST_REQUIRE(postlist->next(current_value));
    BOOST_REQUIRE_EQUAL(current_value, it->second);
  }
  searcher.close();
}

template <typename T>
void run_test_case() {
  const size_t record_count(10000);
  std::multimap<std::string, T> expected_records;

  // -------------------------------------------------------------------------
  // Test ManagedIndexer with sorted keys
  // -------------------------------------------------------------------------
  {
    test::ManagedDirectory input("input");
    test::ManagedDirectory index("index");
    expected_records =
        write_inverted_file<T>(input.dir() / "records.in", record_count,
                               aii::key_sorting_type::sorted);
    build_inverted_index<T>(input.dir(), index.dir(),
                            aii::key_sorting_type::sorted);
    check_inverted_index<T>(index.dir(), expected_records);
  }

  // -------------------------------------------------------------------------
  // Test ManagedIndexer with unsorted keys
  // -------------------------------------------------------------------------
  {
    test::ManagedDirectory input("input");
    test::ManagedDirectory index("index");
    expected_records =
        write_inverted_file<T>(input.dir() / "records.in", record_count,
                               aii::key_sorting_type::unsorted);
    build_inverted_index<T>(input.dir(), index.dir(),
                            aii::key_sorting_type::unsorted);
    check_inverted_index<T>(index.dir(), expected_records);
  }
}


BOOST_AUTO_TEST_SUITE(test_ManagedIndexer);

// -----------------------------------------------------------------------------
// Test Configuration
// -----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_ManagedIndexer_index) {
  typedef aii::ManagedIndexer<int> Indexer;

  aii::Configuration config;
  BOOST_REQUIRE_THROW(Indexer::index(config), std::invalid_argument);

  config.set_input_file("some_file");
  config.set_input_directory("some_directory");
  BOOST_REQUIRE_THROW(Indexer::index(config), std::invalid_argument);
}

// -----------------------------------------------------------------------------
// Test rehasing scheme with expected record count
// -----------------------------------------------------------------------------

// BOOST_AUTO_TEST_CASE(test_rehashing_scheme)
//{
//  typedef value::DoubleLongLongInt value_type;
//
//  const size_t record_count(50000000);
//  const bfs::path index_dir("index");
//
//  std::string key;
//  value_type value;
//  Configuration config;
//  config.set_index_directory(index_dir.string());
//  config.set_key_sorting(util::key_sorting::unsorted);
//  config.set_expected_record_count(record_count); // avoids rehashing
//
//  BOOST_REQUIRE(bfs::create_directory(index_dir));
//  Indexer<value_type> indexer(config);
//  for (unsigned i(0); i != record_count; ++i)
//  {
//    generator<std::string>::numbered(key, i % 1000);
//    generator<value_type>::numbered(value, i % 1000);
//    indexer.insert(Record<value_type>(key, value));
//  }
//  const Properties properties(indexer.index());
//  BOOST_REQUIRE_EQUAL(properties.key_count, 1000);
//  BOOST_REQUIRE_EQUAL(properties.value_count, record_count);
//  bfs::remove_all(index_dir);
//}

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
