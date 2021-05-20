// exception.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_EXCEPTION_HPP
#define NETSPEAK_UTIL_EXCEPTION_HPP

#include <sstream>
#include <stdexcept>
#include <string>

/**
 * Functions that provide an interface to throw C++ standard exceptions
 * attached with exception-dependent messages and optional user information.
 */
namespace netspeak {
namespace util {

template <typename T>
inline std::string format(const std::string& msg, const T& value) {
  std::ostringstream oss;
  oss << msg << " : " << value;
  return oss.str();
}

template <typename T>
inline void throw_runtime_error(const std::string& msg, const T& value) {
  throw std::runtime_error(format(msg, value));
}

template <typename T>
inline void throw_invalid_argument(const std::string& msg, const T& value) {
  throw std::invalid_argument(format(msg, value));
}

template <typename T>
inline void throw_out_of_range(const std::string& msg, const T& value) {
  throw std::out_of_range(format(msg, value));
}

template <typename T>
inline void throw_length_error(const std::string& msg, const T& value) {
  throw std::length_error(format(msg, value));
}

template <typename T>
inline void throw_logic_error(const std::string& msg, const T& value) {
  throw std::logic_error(format(msg, value));
}

template <typename T>
inline void throw_domain_error(const std::string& msg, const T& value) {
  throw std::domain_error(format(msg, value));
}

inline void throw_logic_error(const std::string& msg) {
  throw std::logic_error(msg);
}

inline void throw_domain_error(const std::string& msg) {
  throw std::domain_error(msg);
}

inline void throw_runtime_error(const std::string& msg) {
  throw std::runtime_error(msg);
}

inline void throw_overflow_error(const std::string& msg) {
  throw std::overflow_error(msg);
}

inline void throw_underflow_error(const std::string& msg) {
  throw std::underflow_error(msg);
}

inline void throw_invalid_argument(const std::string& msg) {
  throw std::invalid_argument(msg);
}

} // namespace util
} // namespace netspeak

#endif // NETSPEAK_UTIL_EXCEPTION_HPP
