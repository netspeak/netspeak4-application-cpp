// value_traits.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_VALUE_TRAITS_HPP
#define NETSPEAK_VALUE_VALUE_TRAITS_HPP

#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// Template specializations to obtain the name of some primitive type.
// -----------------------------------------------------------------------------

template <typename T>
struct NameOf;

template <>
struct NameOf<int8_t> {
  static std::string value() {
    return "Byte";
  }
};

template <>
struct NameOf<uint8_t> {
  static std::string value() {
    return "UByte";
  }
};

template <>
struct NameOf<int16_t> {
  static std::string value() {
    return "Short";
  }
};

template <>
struct NameOf<uint16_t> {
  static std::string value() {
    return "UShort";
  }
};

template <>
struct NameOf<int32_t> {
  static std::string value() {
    return "Int";
  }
};

template <>
struct NameOf<uint32_t> {
  static std::string value() {
    return "UInt";
  }
};

template <>
struct NameOf<int64_t> {
  static std::string value() {
    return "Long";
  }
};

template <>
struct NameOf<uint64_t> {
  static std::string value() {
    return "ULong";
  }
};

template <>
struct NameOf<float> {
  static std::string value() {
    return "Float";
  }
};

template <>
struct NameOf<double> {
  static std::string value() {
    return "Double";
  }
};

// -----------------------------------------------------------------------------
// Primary template for arithmetic types (boolean, int, float, double)
// -----------------------------------------------------------------------------

template <typename T>
inline void print_to_os(T value, std::ostream& os) {
  os << value;
}

// It's safer to overload a function, rather than specializing the template.
// (http://www.gotw.ca/publications/mill17.htm)
inline void print_to_os(int8_t value, std::ostream& os) {
  os << static_cast<int16_t>(value);
}

// It's safer to overload a function, rather than specializing the template.
// (http://www.gotw.ca/publications/mill17.htm)
inline void print_to_os(uint8_t value, std::ostream& os) {
  os << static_cast<uint16_t>(value);
}

static const char tuple_element_separator = '\t';

template <typename T>
struct value_traits {
  typedef
      typename std::enable_if<std::is_arithmetic<T>::value, T>::type value_type;
  typedef uint16_t io_size_type; // not needed for types with constant size

  static constexpr std::size_t ByteSize() {
    return sizeof(T);
  }

  static inline size_t size_of(const value_type& value) {
    return sizeof(value);
  }

  static inline char* copy_to(const value_type& value, char* buffer) {
    std::memcpy(buffer, &value, sizeof(value));
    return buffer + sizeof(value);
  }

  static inline const char* copy_from(value_type& value, const char* buffer) {
    std::memcpy(&value, buffer, sizeof(value));
    return buffer + sizeof(value);
  }

  static inline bool write_to(const value_type& value, FILE* file) {
    return std::fwrite(&value, sizeof(value), 1, file) == 1;
  }

  static inline bool read_from(value_type& value, FILE* file) {
    return std::fread(&value, sizeof(value), 1, file) == 1;
  }

  static inline void print_to(const value_type& value, std::ostream& os) {
    print_to_os(value, os);
  }

  static inline void println_to(const value_type& value, std::ostream& os) {
    print_to(value, os);
    os << '\n';
  }

  /**
   * Parses and sets new data for the given value from the given stream.
   * The routine also extracts the {@link tuple_element_separator} if any.
   */
  static inline void parse_from(value_type& value, std::istream& is) {
    is >> value;
    if (is.fail()) {
      util::throw_runtime_error("Reading next " + std::string(type_name()) +
                                " from input stream failed.");
    }
    if (is.peek() == tuple_element_separator)
      is.get();
  }

  static inline std::string type_name() {
    return NameOf<value_type>::value();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <typename T>
struct generator {
  typedef T value_type;

  static void randomized(value_type& value) {
    value = ::rand();
  }

  static void numbered(value_type& value, size_t number) {
    value = number;
  }
};

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_VALUE_TRAITS_HPP
