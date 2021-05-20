// triple_traits.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_TRIPLE_TRAITS_HPP
#define NETSPEAK_VALUE_TRIPLE_TRAITS_HPP

#include "netspeak/value/triple.hpp"
#include "netspeak/value/value_traits.hpp"

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// Partial specialization for value::triple<T1, T2, T3>
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3>
struct value_traits<triple<T1, T2, T3> > {
  typedef triple<T1, T2, T3> value_type;
  typedef uint16_t io_size_type;

  typedef value_traits<T1> T1Traits; // Compiles for arithmetic types only.
  typedef value_traits<T2> T2Traits; // Compiles for arithmetic types only.
  typedef value_traits<T3> T3Traits; // Compiles for arithmetic types only.

  static inline size_t size_of(const value_type& triple) {
    return T1Traits::size_of(triple.e1()) + T2Traits::size_of(triple.e2()) +
           T3Traits::size_of(triple.e3());
  }

  static inline char* copy_to(const value_type& triple, char* buffer) {
    buffer = T1Traits::copy_to(triple.e1(), buffer);
    buffer = T2Traits::copy_to(triple.e2(), buffer);
    return T3Traits::copy_to(triple.e3(), buffer);
  }

  static inline const char* copy_from(value_type& triple, const char* buffer) {
    buffer = T1Traits::copy_from(triple.e1(), buffer);
    buffer = T2Traits::copy_from(triple.e2(), buffer);
    return T3Traits::copy_from(triple.e3(), buffer);
  }

  static inline bool write_to(const value_type& triple, FILE* file) {
    return T1Traits::write_to(triple.e1(), file) &&
           T2Traits::write_to(triple.e2(), file) &&
           T3Traits::write_to(triple.e3(), file);
  }

  static inline bool read_from(value_type& triple, FILE* file) {
    return T1Traits::read_from(triple.e1(), file) &&
           T2Traits::read_from(triple.e2(), file) &&
           T3Traits::read_from(triple.e3(), file);
  }

  static inline void print_to(const value_type& triple, std::ostream& os) {
    T1Traits::print_to(triple.e1(), os);
    os << tuple_element_separator;
    T2Traits::print_to(triple.e2(), os);
    os << tuple_element_separator;
    T3Traits::print_to(triple.e3(), os);
  }

  static inline void println_to(const value_type& triple, std::ostream& os) {
    print_to(triple, os);
    os << '\n';
  }

  static inline void parse_from(value_type& triple, std::istream& is) {
    T1Traits::parse_from(triple.e1(), is);
    T2Traits::parse_from(triple.e2(), is);
    T3Traits::parse_from(triple.e3(), is);
  }

  static inline std::string type_name() {
    return T1Traits::type_name() + T2Traits::type_name() +
           T3Traits::type_name();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3>
struct generator<triple<T1, T2, T3> > {
  typedef triple<T1, T2, T3> value_type;

  static void randomized(value_type& value) {
    generator<T1>::randomized(value.e1());
    generator<T2>::randomized(value.e2());
    generator<T3>::randomized(value.e3());
  }

  static void numbered(value_type& value, size_t number) {
    generator<T1>::numbered(value.e1(), number);
    generator<T2>::numbered(value.e2(), number);
    generator<T3>::numbered(value.e3(), number);
  }
};

// -----------------------------------------------------------------------------
// Operators
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3>
std::ostream& operator<<(std::ostream& os, const triple<T1, T2, T3>& value) {
  if (os)
    value_traits<triple<T1, T2, T3> >::print_to(value, os);
  return os;
}

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_TRIPLE_TRAITS_HPP
