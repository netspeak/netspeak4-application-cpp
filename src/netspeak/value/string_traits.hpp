// string_traits.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_STRING_TRAITS_HPP
#define NETSPEAK_VALUE_STRING_TRAITS_HPP

#include <cstdio>
#include <string>

#include <boost/algorithm/string/trim.hpp>

#include "netspeak/util/exception.hpp"
#include "netspeak/value/value_traits.hpp"

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// The base template is defined in netspeak/value/value_traits.hpp.
// -----------------------------------------------------------------------------

template <>
struct NameOf<std::string> {
  static std::string value() {
    return "String";
  }
};

// -----------------------------------------------------------------------------
// Full specialization for std::string
// -----------------------------------------------------------------------------

template <>
struct value_traits<std::string> {
  typedef std::string value_type;
  typedef uint16_t io_size_type;
  typedef value_type::size_type score_type;

  static inline score_type score(const value_type& value) {
    return value.size();
  }

  static inline size_t size_of(const value_type& value) {
    return sizeof(io_size_type) + value.size();
  }

  static inline char* copy_to(const value_type& value, char* buffer) {
    buffer = value_traits<io_size_type>::copy_to(value.size(), buffer);
    value.copy(buffer, value.size());
    return buffer + value.size();
  }

  static inline const char* copy_from(value_type& value, const char* buffer) {
    io_size_type size;
    buffer = value_traits<io_size_type>::copy_from(size, buffer);
    value.assign(buffer, size);
    return buffer + size;
  }

  static inline bool write_to(const value_type& value, FILE* file) {
    return value_traits<io_size_type>::write_to(value.size(), file) &&
           std::fwrite(value.data(), 1, value.size(), file) == value.size();
  }

  static inline bool read_from(value_type& value, FILE* file) {
    io_size_type size;
    if (!value_traits<io_size_type>::read_from(size, file))
      return false;
    value.resize(size);
    return std::fread(&value[0], 1, size, file) == size;
  }

  static inline void print_to(const value_type& value, std::ostream& os) {
    if (value.find(tuple_element_separator) != std::string::npos) {
      util::throw_invalid_argument(
          "std::string tuple element contains delimiter character", value);
    }
    os << value;
  }

  static inline void println_to(const value_type& value, std::ostream& os) {
    print_to(value, os);
    os << '\n';
  }

  // Clears the given string and appends characters read from the given stream.
  // The appending stops when either '\r', '\n', EOF or tuple_element_separator
  // occur. Except of tuple_element_separator the terminating characters will
  // not be extracted from the stream so that the outer parser is still able to
  // detect line endings.
  static inline void parse_from(value_type& value, std::istream& is) {
    value.clear();
    while (is.good()) {
      switch (is.peek()) {
        case '\r':
        case '\n':
        case std::istream::traits_type::eof():
          return;
        case tuple_element_separator:
          is.get();
          return;
        default:
          value.push_back(is.get());
      }
    }
  }

  static inline std::string type_name() {
    return NameOf<value_type>::value();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <>
struct generator<std::string> {
  typedef std::string value_type;

  static void randomized(value_type& value) {
    value.assign("some_string");
    value += std::to_string(::rand());
  }

  static void numbered(value_type& value, size_t number) {
    value.assign("some_string");
    value += std::to_string(number);
  }
};

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_STRING_TRAITS_HPP
