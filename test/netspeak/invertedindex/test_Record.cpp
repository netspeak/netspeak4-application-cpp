// test_Record.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <boost/test/unit_test.hpp>

#include "netspeak/invertedindex/Record.hpp"
#include "netspeak/value/pair.hpp"

BOOST_AUTO_TEST_SUITE(record);

namespace ai = netspeak::invertedindex;
namespace av = netspeak::value;

typedef av::pair<int, double> value_type;

BOOST_AUTO_TEST_CASE(default_ctor) {
  value_type value;
  ai::Record<value_type> record;
  BOOST_CHECK(record.key().empty());
  BOOST_CHECK_EQUAL(record.value(), value);
}

BOOST_AUTO_TEST_CASE(explicit_ctor) {
  value_type value(1, 2);
  std::string key("key");
  ai::Record<value_type> record(key, value);
  BOOST_CHECK_EQUAL(record.key(), key);
  BOOST_CHECK_EQUAL(record.value(), value);
}

BOOST_AUTO_TEST_CASE(copy_ctor) {
  value_type value(1, 2);
  std::string key("key");
  ai::Record<value_type> record(key, value);
  ai::Record<value_type> record_copy(record);
  BOOST_CHECK_EQUAL(record_copy, record);
}

BOOST_AUTO_TEST_CASE(equalitiy_operator) {
  value_type value(1, 2);
  std::string key("key");
  ai::Record<value_type> record(key, value);
  ai::Record<value_type> record_copy(record);
  BOOST_CHECK_EQUAL(record_copy, record);
}

BOOST_AUTO_TEST_CASE(assignment_operator) {
  value_type value(1, 2);
  std::string key("key");
  ai::Record<value_type> record(key, value);
  ai::Record<value_type> record_copy;
  record_copy = record;
  BOOST_CHECK_EQUAL(record_copy, record);
}

BOOST_AUTO_TEST_SUITE_END();
