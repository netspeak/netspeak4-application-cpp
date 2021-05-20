// quadruple_traits.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_QUADRUPLE_TRAITS_HPP
#define NETSPEAK_VALUE_QUADRUPLE_TRAITS_HPP

#include "netspeak/value/quadruple.hpp"
#include "netspeak/value/value_traits.hpp"

namespace netspeak {
namespace value {

// -----------------------------------------------------------------------------
// Partial specialization for value::quadruple<T1, T2, T3, T4>
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4>
struct value_traits<quadruple<T1, T2, T3, T4> > {
  typedef quadruple<T1, T2, T3, T4> value_type;
  typedef uint16_t io_size_type;

  typedef value_traits<T1> T1Traits; // Compiles for arithmetic types only.
  typedef value_traits<T2> T2Traits; // Compiles for arithmetic types only.
  typedef value_traits<T3> T3Traits; // Compiles for arithmetic types only.
  typedef value_traits<T4> T4Traits; // Compiles for arithmetic types only.

  static inline size_t size_of(const value_type& quadruple) {
    return T1Traits::size_of(quadruple.e1()) +
           T2Traits::size_of(quadruple.e2()) +
           T3Traits::size_of(quadruple.e3()) +
           T4Traits::size_of(quadruple.e4());
  }

  static inline char* copy_to(const value_type& quadruple, char* buffer) {
    buffer = T1Traits::copy_to(quadruple.e1(), buffer);
    buffer = T2Traits::copy_to(quadruple.e2(), buffer);
    buffer = T3Traits::copy_to(quadruple.e3(), buffer);
    return T4Traits::copy_to(quadruple.e4(), buffer);
  }

  static inline const char* copy_from(value_type& quadruple,
                                      const char* buffer) {
    buffer = T1Traits::copy_from(quadruple.e1(), buffer);
    buffer = T2Traits::copy_from(quadruple.e2(), buffer);
    buffer = T3Traits::copy_from(quadruple.e3(), buffer);
    return T4Traits::copy_from(quadruple.e4(), buffer);
  }

  static inline bool write_to(const value_type& quadruple, FILE* file) {
    return T1Traits::write_to(quadruple.e1(), file) &&
           T2Traits::write_to(quadruple.e2(), file) &&
           T3Traits::write_to(quadruple.e3(), file) &&
           T4Traits::write_to(quadruple.e4(), file);
  }

  static inline bool read_from(value_type& quadruple, FILE* file) {
    return T1Traits::read_from(quadruple.e1(), file) &&
           T2Traits::read_from(quadruple.e2(), file) &&
           T3Traits::read_from(quadruple.e3(), file) &&
           T4Traits::read_from(quadruple.e4(), file);
  }

  static inline void print_to(const value_type& quadruple, std::ostream& os) {
    T1Traits::print_to(quadruple.e1(), os);
    os << tuple_element_separator;
    T2Traits::print_to(quadruple.e2(), os);
    os << tuple_element_separator;
    T3Traits::print_to(quadruple.e3(), os);
    os << tuple_element_separator;
    T4Traits::print_to(quadruple.e4(), os);
  }

  static inline void println_to(const value_type& quadruple, std::ostream& os) {
    print_to(quadruple, os);
    os << '\n';
  }

  static inline void parse_from(value_type& quadruple, std::istream& is) {
    T1Traits::parse_from(quadruple.e1(), is);
    T2Traits::parse_from(quadruple.e2(), is);
    T3Traits::parse_from(quadruple.e3(), is);
    T4Traits::parse_from(quadruple.e4(), is);
  }

  static inline std::string type_name() {
    return T1Traits::type_name() + T2Traits::type_name() +
           T3Traits::type_name() + T4Traits::type_name();
  }
};

// -----------------------------------------------------------------------------
// Random and numbered instance generators needed for unit testing.
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4>
struct generator<quadruple<T1, T2, T3, T4> > {
  typedef quadruple<T1, T2, T3, T4> value_type;

  static void randomized(value_type& value) {
    generator<T1>::randomized(value.e1());
    generator<T2>::randomized(value.e2());
    generator<T3>::randomized(value.e3());
    generator<T4>::randomized(value.e4());
  }

  static void numbered(value_type& value, size_t number) {
    generator<T1>::numbered(value.e1(), number);
    generator<T2>::numbered(value.e2(), number);
    generator<T3>::numbered(value.e3(), number);
    generator<T4>::numbered(value.e4(), number);
  }
};

// -----------------------------------------------------------------------------
// Operators
// -----------------------------------------------------------------------------

template <typename T1, typename T2, typename T3, typename T4>
std::ostream& operator<<(std::ostream& os,
                         const quadruple<T1, T2, T3, T4>& value) {
  if (os)
    value_traits<quadruple<T1, T2, T3, T4> >::print_to(value, os);
  return os;
}

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_QUADRUPLE_TRAITS_HPP
