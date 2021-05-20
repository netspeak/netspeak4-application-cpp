// check.hpp -*- C++ -*-
// Copyright (C) 2008-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_CHECK_HPP
#define NETSPEAK_UTIL_CHECK_HPP

#include <string>

#include "netspeak/util/exception.hpp"

namespace netspeak {
namespace util {

inline void check(bool expr, const std::string& msg) {
  if (!expr) {
    throw_runtime_error(msg);
  }
}

template <typename T>
void check(bool expr, const std::string& msg, const T& obj) {
  if (!expr) {
    throw_runtime_error(msg, obj);
  }
}

template <typename T>
void check(bool expr, const std::string& func, const std::string& msg,
           const T& obj) {
  if (!expr) {
    throw_runtime_error(func + " : " + msg, obj);
  }
}

template <typename T>
void check_equal(const T& expected, const T& actual,
                 const std::string& message = "") {
  if (!(expected == actual)) {
    std::ostringstream oss;
    oss << message << "\nexpected: " << expected << "\nactual: " << actual;
    throw_runtime_error(oss.str());
  }
}

} // namespace util
} // namespace netspeak

#endif // NETSPEAK_UTIL_CHECK_HPP
