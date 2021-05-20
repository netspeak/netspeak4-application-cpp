// test_big_hash_map.cpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <ostream>
#include <string>
#include <utility>

#include <boost/test/unit_test.hpp>
#include <boost/type_traits.hpp>

#include "../ManagedDirectory.hpp"

#include "netspeak/bighashmap/BigHashMap.hpp"
#include "netspeak/bighashmap/Builder.hpp"
#include "netspeak/util/conversion.hpp"
#include "netspeak/util/memory.hpp"

using namespace netspeak;
using namespace value;
namespace bfs = boost::filesystem;

template <typename Value>
struct RandomRecordGenerator {
  typedef std::pair<std::string, Value> Record;

  RandomRecordGenerator() : counter(0) {}

  const Record operator()() {
    Record record;
    record.first.assign("key");
    record.first += util::to_string(counter++);
    generator<Value>::randomized(record.second);
    return record;
  }

  unsigned counter;
};

namespace std {

template <typename Value>
ostream& operator<<(ostream& os, const std::pair<std::string, Value>& record) {
  if (os) {
    os << record.first << '\t';
    value::value_traits<Value>::print_to(record.second, os);
  }
  return os;
}
} // namespace std

template <typename Value>
void RunTest(util::memory_type mode, size_t num_records) {
  typedef std::pair<std::string, Value> Record;
  typedef bighashmap::BigHashMap<Value> Map;
  // -------------------------------------------------------------------------
  // Create records and write them to a text file.
  // -------------------------------------------------------------------------
  std::vector<Record> records(num_records);
  std::generate(records.begin(), records.end(), RandomRecordGenerator<Value>());
  test::ManagedDirectory input("bhm_test_input");
  test::ManagedDirectory output("bhm_test_output");
  const bfs::path input_file = input.dir() / "records";
  bfs::ofstream ofs(input_file);
  BOOST_REQUIRE(ofs);
  // enable fixed floating-point notation
  ofs.setf(std::ios::fixed, std::ios::floatfield);
  std::copy(records.begin(), records.end(),
            std::ostream_iterator<Record>(ofs, "\n"));
  ofs.close();
  // -------------------------------------------------------------------------
  // Build a BigHashMap from the input record file.
  // -------------------------------------------------------------------------
  bighashmap::Builder<Value>::Build(input_file.parent_path(), output.dir());
  // -------------------------------------------------------------------------
  // Open the BigHashMap and lookup the entire known key set.
  // -------------------------------------------------------------------------
  Value value;
  Map* map = Map::Open(output.dir(), mode);
  for (auto it = records.begin(); it != records.end(); ++it) {
    BOOST_REQUIRE(map->Get(it->first, value));
    BOOST_REQUIRE_EQUAL(it->second, value);
  }
  // -------------------------------------------------------------------------
  // Lookup a set of unknown keys (should not find anything).
  // -------------------------------------------------------------------------
  for (std::size_t i = 0; i != num_records; ++i) {
    BOOST_REQUIRE(!map->Get(util::to_string(std::rand()), value));
  }
  delete map;
}

// TEST CASES

BOOST_AUTO_TEST_SUITE(test_big_hash_map);

static const size_t NumRecords = 100000;

BOOST_AUTO_TEST_CASE(test_i32_external) {
  RunTest<int32_t>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i32_internal) {
  RunTest<int32_t>(util::memory_type::mb4096, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i64_external) {
  RunTest<int64_t>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i64_internal) {
  RunTest<int64_t>(util::memory_type::mb4096, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_dbl_external) {
  RunTest<double>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_dbl_internal) {
  RunTest<double>(util::memory_type::mb4096, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i32_i64_external) {
  typedef value::pair<int32_t, int64_t> value_type;
  RunTest<value_type>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i32_i64_internal) {
  typedef value::pair<int32_t, int64_t> value_type;
  RunTest<value_type>(util::memory_type::mb4096, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i32_dbl_external) {
  typedef value::pair<int32_t, double> value_type;
  RunTest<value_type>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i32_dbl_internal) {
  typedef value::pair<int32_t, double> value_type;
  RunTest<value_type>(util::memory_type::mb4096, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i64_dbl_external) {
  typedef value::pair<int64_t, double> value_type;
  RunTest<value_type>(util::memory_type::min_required, NumRecords);
}

BOOST_AUTO_TEST_CASE(test_i64_dbl_internal) {
  typedef value::pair<int64_t, double> value_type;
  RunTest<value_type>(util::memory_type::mb4096, NumRecords);
}

// (Michael Schmidt) These tests take way too long

// // (Martin Trenkmann) This test takes some time
// BOOST_AUTO_TEST_CASE(test_i32_dbl_external_large_scale) {
//   typedef value::pair<int32_t, double> value_type;
//   RunTest<value_type>(util::memory_type::min_required, 10000000);
// }
//
// // (Martin Trenkmann) This test takes not so much time
// BOOST_AUTO_TEST_CASE(test_i32_dbl_internal_large_scale) {
//   typedef value::pair<int32_t, double> value_type;
//   RunTest<value_type>(util::memory_type::mb4096, 10000000);
// }

BOOST_AUTO_TEST_SUITE_END();
