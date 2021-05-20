// big_string_traits.hpp -*- C++ -*-
// Copyright (C) 2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_BIG_STRING_TRAITS_HPP
#define NETSPEAK_VALUE_BIG_STRING_TRAITS_HPP

#include <cstdio>
#include <string>

#include <boost/algorithm/string/trim.hpp>

#include "netspeak/util/exception.hpp"
#include "netspeak/value/big_string.hpp"
#include "netspeak/value/string_traits.hpp"
#include "netspeak/value/value_traits.hpp"

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// The base template is defined in netspeak/value/value_traits.hpp.
// -----------------------------------------------------------------------------

template <>
struct NameOf<big_string> {
  static std::string value() {
    return "BigString";
  }
};

// -----------------------------------------------------------------------------
// Full specialization of struct value_traits<> for big_string.
// -----------------------------------------------------------------------------

template <>
struct value_traits<big_string> {
  typedef big_string value_type;
  typedef uint32_t io_size_type;
  typedef std::size_t score_type;

  static inline score_type score(const value_type& value) {
    return value.str.size();
  }

  static inline size_t size_of(const value_type& value) {
    return sizeof(io_size_type) + value.str.size();
  }

  static inline char* copy_to(const value_type& value, char* buffer) {
    buffer = value_traits<io_size_type>::copy_to(value.str.size(), buffer);
    value.str.copy(buffer, value.str.size());
    return buffer + value.str.size();
  }

  static inline const char* copy_from(value_type& value, const char* buffer) {
    io_size_type size;
    buffer = value_traits<io_size_type>::copy_from(size, buffer);
    value.str.assign(buffer, size);
    return buffer + size;
  }

  static inline bool write_to(const value_type& value, FILE* file) {
    return value_traits<io_size_type>::write_to(value.str.size(), file) &&
           std::fwrite(value.str.data(), 1, value.str.size(), file) ==
               value.str.size();
  }

  static inline bool read_from(value_type& value, FILE* file) {
    io_size_type size;
    if (!value_traits<io_size_type>::read_from(size, file))
      return false;
    value.str.resize(size);
    return std::fread(&value.str[0], 1, size, file) == size;
  }

  static inline void print_to(const value_type& value, std::ostream& os) {
    value_traits<std::string>::print_to(value.str, os);
  }

  static inline void println_to(const value_type& value, std::ostream& os) {
    print_to(value, os);
    os << '\n';
  }

  static inline void parse_from(value_type& value, std::istream& is) {
    value_traits<std::string>::parse_from(value.str, is);
  }

  static inline std::string type_name() {
    return NameOf<value_type>::value();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <>
struct generator<big_string> {
  typedef big_string value_type;

  static void randomized(value_type& value) {
    value.str.assign("some_big_string");
    value.str += std::to_string(::rand());
  }

  static void numbered(value_type& value, size_t number) {
    value.str.assign("some_big_string");
    value.str += std::to_string(number);
  }
};

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_BIG_STRING_TRAITS_HPP
