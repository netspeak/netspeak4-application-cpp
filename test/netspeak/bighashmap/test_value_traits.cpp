// test_value_traits.cpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "netspeak/util/systemio.hpp"
#include "netspeak/value/pair_traits.hpp"
#include "netspeak/value/quadruple_traits.hpp"
#include "netspeak/value/triple_traits.hpp"

using namespace netspeak;
namespace bfs = boost::filesystem;

static const bfs::path k_tmp_file("test_value_traits.tmp");

// Primary template matches for build-in types
template <typename T>
void test(unsigned record_count) {
  typedef T value_type;
  typedef value::value_traits<value_type> traits_type;

  // Test constant size of build-in types
  value_type test_value;
  BOOST_REQUIRE_EQUAL(sizeof(value_type), traits_type::size_of(test_value));

  // Test file I/O
  FILE* fs(util::fopen(k_tmp_file, "wb"));
  for (unsigned i(0); i != record_count; ++i) {
    test_value = i;
    BOOST_REQUIRE(traits_type::write_to(test_value, fs));
  }
  util::fclose(fs);

  // Read back from file
  value_type expected_value;
  fs = util::fopen(k_tmp_file, "rb");
  for (unsigned i(0); i != record_count; ++i) {
    expected_value = i;
    BOOST_REQUIRE(traits_type::read_from(test_value, fs));
    BOOST_REQUIRE_EQUAL(expected_value, test_value);
  }
  BOOST_REQUIRE(!traits_type::read_from(test_value, fs));
  util::fclose(fs);
  bfs::remove(k_tmp_file);

  // Test buffer I/O
  // Note that copy_to and copy_from do no range check
  const size_t buffer_size(record_count * sizeof(value_type));
  char* buffer_begin(new char[buffer_size]);
  char* buffer_pos_old(buffer_begin);
  const char* buffer_pos_new(buffer_begin);
  for (unsigned i(0); i != record_count; ++i) {
    test_value = i;
    buffer_pos_new = traits_type::copy_to(test_value, buffer_pos_old);
    BOOST_REQUIRE_EQUAL(traits_type::size_of(test_value),
                        buffer_pos_new - buffer_pos_old);
    buffer_pos_old = const_cast<char*>(buffer_pos_new);
  }
  BOOST_REQUIRE_EQUAL(buffer_size, buffer_pos_old - buffer_begin);

  // Read back from buffer
  buffer_pos_old = buffer_begin;
  buffer_pos_new = buffer_begin;
  for (unsigned i(0); i != record_count; ++i) {
    expected_value = i;
    buffer_pos_new = traits_type::copy_from(test_value, buffer_pos_old);
    BOOST_REQUIRE_EQUAL(traits_type::size_of(expected_value),
                        buffer_pos_new - buffer_pos_old);
    BOOST_REQUIRE_EQUAL(expected_value, test_value);
    buffer_pos_old = const_cast<char*>(buffer_pos_new);
  }
  BOOST_REQUIRE_EQUAL(buffer_size, buffer_pos_old - buffer_begin);
  delete[] buffer_begin;
}

// Specialization template for std::string
// template<>
// void test_type(const std::string& value, unsigned loop_count)
//{
//  typedef std::string           value_type;
//  typedef io_traits<value_type> traits_type;
//
//  // Test variable size of std::string
//  BOOST_REQUIRE_EQUAL(sizeof(traits_type::io_size_type) + value.size(),
//      traits_type::byte_size(value));
//
//  // Test FILE* I/O
//  char suffix[16];
//  value_type test_value;
//  FILE* file(io::open(tmp_file, "wb"));
//  for (unsigned i(0); i != loop_count; ++i)
//  {
//    std::sprintf(suffix, "%u", i);
//    test_value = value + suffix;
//    BOOST_REQUIRE_EQUAL(true, traits_type::write_to(test_value, file));
//  }
//  io::close(file);
//
//  value_type expected_value;
//  file = io::open(tmp_file, "rb");
//  for (unsigned i(0); i != loop_count; ++i)
//  {
//    std::sprintf(suffix, "%u", i);
//    expected_value = value + suffix;
//    BOOST_REQUIRE_EQUAL(true, traits_type::read_from(test_value, file));
//    BOOST_REQUIRE_EQUAL(expected_value, test_value);
//  }
//  BOOST_REQUIRE_EQUAL(false, traits_type::read_from(test_value, file));
//  io::close(file);
//  bfs::remove(tmp_file);
//
//  // Test char* I/O
//  // Note that copy_to and copy_from do no range check
//  unsigned offset(0);
//  const size_t suffix_length(10); // The max. length of the suffix string
//  const size_t value_size_max(traits_type::byte_size(value) + suffix_length);
//  const size_t buffer_size(loop_count * value_size_max);
//  char* buffer(new char[buffer_size]);
//  for (unsigned i(0); i != loop_count; ++i)
//  {
//    std::sprintf(suffix, "%u", i);
//    test_value = value + suffix;
//    BOOST_REQUIRE_EQUAL(traits_type::byte_size(test_value),
//        traits_type::copy_to(test_value, buffer + offset));
//    offset += traits_type::byte_size(test_value);
//  }
//
//  offset = 0;
//  for (unsigned i(0); i != loop_count; ++i)
//  {
//    std::sprintf(suffix, "%u", i);
//    expected_value = value + suffix;
//    BOOST_REQUIRE_EQUAL(traits_type::byte_size(expected_value),
//        traits_type::copy_from(test_value, buffer + offset));
//    BOOST_REQUIRE_EQUAL(expected_value, test_value);
//    offset += traits_type::byte_size(test_value);
//  }
//  delete[] buffer;
//}

// TEST CASES

BOOST_AUTO_TEST_SUITE(test_value_traits)

static const size_t k_record_count(100000);

BOOST_AUTO_TEST_CASE(test_i16) {
  test<int16_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_ui16) {
  test<uint16_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_i32) {
  test<int32_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_ui32) {
  test<uint32_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_i64) {
  test<int64_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_ui64) {
  test<uint64_t>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_dbl) {
  test<double>(k_record_count);
}

BOOST_AUTO_TEST_CASE(test_fl) {
  test<float>(k_record_count);
}

// BOOST_AUTO_TEST_CASE(test_string)
//{
//  test_type<std::string>("this is a test", 42);
//}

BOOST_AUTO_TEST_SUITE_END()
