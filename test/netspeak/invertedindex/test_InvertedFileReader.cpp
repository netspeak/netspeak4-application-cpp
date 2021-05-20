// test_InvertedFileReader.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <boost/test/unit_test.hpp>

#include "netspeak/invertedindex/InvertedFileReader.hpp"
#include "netspeak/invertedindex/Record.hpp"
#include "netspeak/value/big_string.hpp"
#include "netspeak/value/pair.hpp"
#include "netspeak/value/triple.hpp"

namespace aii = netspeak::invertedindex;
namespace av = netspeak::value;

BOOST_AUTO_TEST_SUITE(test_InvertedFileReader);

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_from_empty_stream) {
  typedef int Value;

  std::stringstream ss;
  aii::Record<Value> actual_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_Int) {
  typedef int Value;

  std::stringstream ss;
  ss << "key1\t1\n";
  ss << "key2\t2\n";
  ss << "key1\t3\n";
  ss << "key2\t4";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(1));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(2));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(3));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(4));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_Int_with_final_newline) {
  typedef int Value;

  std::stringstream ss;
  ss << "key1\t1\n";
  ss << "key2\t2\n";
  ss << "key1\t3\n";
  ss << "key2\t4\n";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(1));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(2));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(3));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(4));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_Int_with_invalid_formatting) {
  typedef int Value;

  std::stringstream ss;
  ss << "key1\t1\n";
  ss << "key2\tabc\n";
  ss << "key3\t3";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(1));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE_THROW(reader.read(actual_record), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_BigString) {
  typedef av::big_string Value;

  std::stringstream ss;
  ss << "key1\tbig string\n";
  ss << "key2\t \n";
  ss << "key1\t\n";
  ss << "key2\tbig string";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value("big string"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(" "));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(""));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value("big string"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_IntDouble) {
  typedef av::pair<int, double> Value;

  std::stringstream ss;
  ss << "key1\t10\t2.0\t30\t4.0\n";
  ss << "key2\t50\t6.0\n";
  ss << "key1\t70\t8.0";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(10, 2.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(30, 4.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(50, 6.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(70, 8.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_IntDouble_old_separator) {
  typedef av::pair<int, double> Value;

  std::stringstream ss;
  ss << "key1\t10 2.0\t30 4.0\n";
  ss << "key2\t50 6.0\n";
  ss << "key1\t70 8.0";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(10, 2.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(30, 4.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(50, 6.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(70, 8.0));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_IntLongDouble) {
  typedef av::triple<int, long, double> Value;

  std::stringstream ss;
  ss << "key1\t10\t20\t10.20\t30\t40\t30.40\n";
  ss << "key2\t50\t60\t50.60\n";
  ss << "key1\t70\t80\t70.80";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(10, 20, 10.20));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(30, 40, 30.40));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(50, 60, 50.60));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(70, 80, 70.80));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_DoubleString) {
  typedef av::pair<double, std::string> Value;

  std::stringstream ss;
  ss << "key1\t10.20\ts10 20\t30.40\ts30 40\n";
  ss << "key2\t50.60\ts50 60\n";
  ss << "key1\t70.80\ts70 80";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(10.20, "s10 20"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(30.40, "s30 40"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(50.60, "s50 60"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(70.80, "s70 80"));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_CASE(test_InvertedFileReader_read_DoubleStringStringInt) {
  typedef av::quadruple<double, std::string, std::string, int> Value;

  std::stringstream ss;
  ss << "key1\t10.20\ts10 20\t10 20s\t20\t30.40\ts30 40\t30 40s\t40\n";
  ss << "key2\t50.60\ts50 60\t50 60s\t60\n";
  ss << "key1\t70.80\ts70 80\t70 80s\t80";

  aii::Record<Value> actual_record;
  aii::Record<Value> expected_record;
  aii::InvertedFileReader<Value> reader(ss);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(10.20, "s10 20", "10 20s", 20));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(30.40, "s30 40", "30 40s", 40));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key2", Value(50.60, "s50 60", "50 60s", 60));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(reader.read(actual_record));
  expected_record.set("key1", Value(70.80, "s70 80", "70 80s", 80));
  BOOST_REQUIRE_EQUAL(expected_record, actual_record);

  BOOST_REQUIRE(!reader.read(actual_record));
}

BOOST_AUTO_TEST_SUITE_END();
