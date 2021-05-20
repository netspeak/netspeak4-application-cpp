// pair_traits.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_PAIR_TRAITS_HPP
#define NETSPEAK_VALUE_PAIR_TRAITS_HPP

#include "netspeak/value/pair.hpp"
#include "netspeak/value/value_traits.hpp"

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// Partial specialization for value::pair<T1, T2>
// -----------------------------------------------------------------------------

template <typename T1, typename T2>
struct value_traits<pair<T1, T2> > {
  typedef pair<T1, T2> value_type;
  typedef uint16_t io_size_type;

  typedef value_traits<T1> T1Traits; // Compiles for arithmetic types only.
  typedef value_traits<T2> T2Traits; // Compiles for arithmetic types only.

  static inline size_t size_of(const value_type& pair) {
    return T1Traits::size_of(pair.e1()) + T2Traits::size_of(pair.e2());
  }

  static inline char* copy_to(const value_type& pair, char* buffer) {
    buffer = T1Traits::copy_to(pair.e1(), buffer);
    return T2Traits::copy_to(pair.e2(), buffer);
  }

  static inline const char* copy_from(value_type& pair, const char* buffer) {
    buffer = T1Traits::copy_from(pair.e1(), buffer);
    return T2Traits::copy_from(pair.e2(), buffer);
  }

  static inline bool write_to(const value_type& pair, FILE* file) {
    return T1Traits::write_to(pair.e1(), file) &&
           T2Traits::write_to(pair.e2(), file);
  }

  static inline bool read_from(value_type& pair, FILE* file) {
    return T1Traits::read_from(pair.e1(), file) &&
           T2Traits::read_from(pair.e2(), file);
  }

  static inline void print_to(const value_type& pair, std::ostream& os) {
    T1Traits::print_to(pair.e1(), os);
    os << tuple_element_separator;
    T2Traits::print_to(pair.e2(), os);
  }

  static inline void println_to(const value_type& pair, std::ostream& os) {
    print_to(pair, os);
    os << '\n';
  }

  static inline void parse_from(value_type& pair, std::istream& is) {
    T1Traits::parse_from(pair.e1(), is);
    T2Traits::parse_from(pair.e2(), is);
  }

  static inline std::string type_name() {
    return T1Traits::type_name() + T2Traits::type_name();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <typename T1, typename T2>
struct generator<pair<T1, T2> > {
  typedef pair<T1, T2> value_type;

  static void randomized(value_type& value) {
    generator<T1>::randomized(value.e1());
    generator<T2>::randomized(value.e2());
  }

  static void numbered(value_type& value, size_t number) {
    generator<T1>::numbered(value.e1(), number);
    generator<T2>::numbered(value.e2(), number);
  }
};

// -----------------------------------------------------------------------------
// Operators
// -----------------------------------------------------------------------------

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const pair<T1, T2>& value) {
  if (os)
    value_traits<pair<T1, T2> >::print_to(value, os);
  return os;
}

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_PAIR_TRAITS_HPP
