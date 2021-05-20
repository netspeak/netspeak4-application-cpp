// quintuple.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_VALUE_QUINTUPLE_HPP
#define NETSPEAK_VALUE_QUINTUPLE_HPP

namespace netspeak {
namespace value {

template <typename T1, typename T2, typename T3, typename T4, typename T5>
class quintuple {
public:
  typedef T1 e1_type;
  typedef T2 e2_type;
  typedef T3 e3_type;
  typedef T4 e4_type;
  typedef T5 e5_type;

public:
  quintuple() : e1_(), e2_(), e3_(), e4_(), e5_() {}

  quintuple(const e1_type& e1, const e2_type& e2, const e3_type& e3,
            const e4_type& e4, const e5_type& e5)
      : e1_(e1), e2_(e2), e3_(e3), e4_(e4), e5_(e5) {}

  quintuple(const quintuple& other)
      : e1_(other.e1_),
        e2_(other.e2_),
        e3_(other.e3_),
        e4_(other.e4_),
        e5_(other.e5_) {}

  ~quintuple() {}

  e1_type& e1() {
    return e1_;
  }
  e2_type& e2() {
    return e2_;
  }
  e3_type& e3() {
    return e3_;
  }
  e4_type& e4() {
    return e4_;
  }
  e5_type& e5() {
    return e5_;
  }

  const e1_type& e1() const {
    return e1_;
  }
  const e2_type& e2() const {
    return e2_;
  }
  const e3_type& e3() const {
    return e3_;
  }
  const e4_type& e4() const {
    return e4_;
  }
  const e5_type& e5() const {
    return e5_;
  }

  void set_e1(const e1_type& e1) {
    e1_ = e1;
  }
  void set_e2(const e2_type& e2) {
    e2_ = e2;
  }
  void set_e3(const e3_type& e3) {
    e3_ = e3;
  }
  void set_e4(const e4_type& e4) {
    e4_ = e4;
  }
  void set_e5(const e5_type& e5) {
    e5_ = e5;
  }

  void set(const e1_type& e1, const e2_type& e2, const e3_type& e3,
           const e4_type& e4, const e5_type& e5) {
    e1_ = e1;
    e2_ = e2;
    e3_ = e3;
    e4_ = e4;
    e5_ = e5;
  }

private:
  e1_type e1_;
  e2_type e2_;
  e3_type e3_;
  e4_type e4_;
  e5_type e5_;
};

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool operator==(const quintuple<T1, T2, T3, T4, T5>& lhs,
                       const quintuple<T1, T2, T3, T4, T5>& rhs) {
  return lhs.e1() == rhs.e1() && lhs.e2() == rhs.e2() && lhs.e3() == rhs.e3() &&
         lhs.e4() == rhs.e4() && lhs.e5() == rhs.e5();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool operator!=(const quintuple<T1, T2, T3, T4, T5>& lhs,
                       const quintuple<T1, T2, T3, T4, T5>& rhs) {
  return !(lhs == rhs);
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool operator<(const quintuple<T1, T2, T3, T4, T5>& lhs,
                      const quintuple<T1, T2, T3, T4, T5>& rhs) {
  return lhs.e1() < rhs.e1();
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool operator>(const quintuple<T1, T2, T3, T4, T5>& lhs,
                      const quintuple<T1, T2, T3, T4, T5>& rhs) {
  return lhs.e1() > rhs.e1();
}

} // namespace value
} // namespace netspeak

#endif // NETSPEAK_VALUE_QUINTUPLE_HPP
