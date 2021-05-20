// pair.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_PAIR_HPP
#define NETSPEAK_VALUE_PAIR_HPP

#include <utility>

namespace netspeak {
namespace value {

template <typename T1, typename T2>
class pair : std::pair<T1, T2> {
public:
  typedef std::pair<T1, T2> base_type;
  typedef typename base_type::first_type e1_type;
  typedef typename base_type::second_type e2_type;

public:
  pair() : base_type() {}

  pair(const e1_type& e1, const e2_type& e2) : base_type(e1, e2) {}

  pair(const pair& rhs) : base_type(rhs) {}

  ~pair() {}

  e1_type& e1() {
    return this->first;
  }

  e2_type& e2() {
    return this->second;
  }

  const e1_type& e1() const {
    return this->first;
  }

  const e2_type& e2() const {
    return this->second;
  }

  void set_e1(const e1_type& e1) {
    this->first = e1;
  }

  void set_e2(const e2_type& e2) {
    this->second = e2;
  }

  void set(const e1_type& e1, const e2_type& e2) {
    this->first = e1;
    this->second = e2;
  }
};

template <typename T1, typename T2>
inline bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
  return lhs.e1() == rhs.e1() && lhs.e2() == rhs.e2();
}

template <typename T1, typename T2>
inline bool operator!=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
  return !(lhs == rhs);
}

template <typename T1, typename T2>
inline bool operator<(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
  return lhs.e1() < rhs.e1();
}

template <typename T1, typename T2>
inline bool operator>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
  return lhs.e1() > rhs.e1();
}

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_PAIR_HPP
