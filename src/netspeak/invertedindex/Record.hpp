// Record.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_RECORD_HPP
#define NETSPEAK_INVERTEDINDEX_RECORD_HPP

#include <string>
#include <utility>

#include "netspeak/value/big_string_traits.hpp"
#include "netspeak/value/pair_traits.hpp"
#include "netspeak/value/quadruple_traits.hpp"
#include "netspeak/value/quintuple_traits.hpp"
#include "netspeak/value/sextuple_traits.hpp"
#include "netspeak/value/string_traits.hpp"
#include "netspeak/value/triple_traits.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class template to represent a key value pair.
 */
template <typename T>
class Record : public std::pair<std::string, T> {
public:
  typedef std::pair<std::string, T> base_type;
  typedef typename base_type::first_type key_type;
  typedef typename base_type::second_type value_type;

  static const char delimiter;

  Record() : base_type() {}

  Record(const key_type& key) : base_type(key, value_type()) {}

  Record(const key_type& key, const value_type& value)
      : base_type(key, value) {}

  Record(const Record& record) : base_type(record) {}

  ~Record() {}

  key_type& key() {
    return this->first;
  }

  const key_type& key() const {
    return this->first;
  }

  void set_key(const key_type& key) {
    this->first = key;
  }

  value_type& value() {
    return this->second;
  }

  const value_type& value() const {
    return this->second;
  }

  void set_value(const value_type& value) {
    this->second = value;
  }

  void set(const key_type& key, const value_type& value) {
    set_key(key);
    set_value(value);
  }

  bool read(FILE* file) {
    return value::value_traits<key_type>::read_from(this->first, file) &&
           value::value_traits<value_type>::read_from(this->second, file);
  }

  bool write(FILE* file) const {
    return value::value_traits<key_type>::write_to(this->first, file) &&
           value::value_traits<value_type>::write_to(this->second, file);
  }

  void parse(std::istream& is) {
    value::value_traits<key_type>::parse_from(this->first, is);
    value::value_traits<value_type>::parse_from(this->second, is);
  }

  void print(std::ostream& os) const {
    value::value_traits<key_type>::print_to(this->first, os);
    os << delimiter; // do not use traits<char>::print_to here b/c casting
    value::value_traits<value_type>::print_to(this->second, os);
  }
};

template <typename T>
const char Record<T>::delimiter('\t');

template <typename T>
std::ostream& operator<<(std::ostream& os, const Record<T>& record) {
  if (os)
    record.print(os);
  return os;
}

template <typename T>
std::istream& operator>>(std::istream& is, Record<T>& record) {
  if (is)
    record.parse(is);
  return is;
}

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_RECORD_HPP
