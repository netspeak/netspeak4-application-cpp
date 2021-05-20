// logging.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_LOGGING_HPP
#define NETSPEAK_UTIL_LOGGING_HPP

#include <cassert>
#include <cstdarg>
#include <ctime>
#include <iostream>
#include <iterator>
#include <string>

/**
 * Functions for logging output.
 */
namespace netspeak {
namespace util {

inline std::ostream& print_timestamp_to(std::ostream& os) {
  if (os) {
    const time_t rawtime(std::time(NULL));
    struct tm* now(localtime(&rawtime));
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", now);
    os << timestr;
  }
  return os;
}

inline void log(const std::string& msg) {
  print_timestamp_to(std::cout);
  std::printf(" %s\n", msg.c_str());
}

inline void log(const char* file, int line, const char* msg) {
  print_timestamp_to(std::cout);
  std::printf(" %s:%d %s\n", file, line, msg);
}

template <typename T>
void log(const std::string& msg, const T& value) {
  print_timestamp_to(std::cout);
  std::printf(" %s ", msg.c_str());
  std::cout << value << std::endl;
}

template <typename T>
void log(const char* file, int line, const char* msg, const T& value) {
  print_timestamp_to(std::cout);
  std::printf(" %s:%d %s ", file, line, msg);
  std::cout << value << std::endl;
}

template <typename Iter>
void log(const char* file, int line, const char* msg, Iter first, Iter last) {
  print_timestamp_to(std::cout);
  std::printf(" %s:%d %s\n", file, line, msg);
  std::copy(first, last,
            std::ostream_iterator<typename Iter::value_type>(std::cout, "\n"));
}

} // namespace util
} // namespace netspeak

#ifdef DEBUG_OUTPUT_ENABLED

#define DEBUG_LOG(MESSAGE) util::log(__FILE__, __LINE__, MESSAGE)
#define DEBUG_LOG2(MESSAGE, VAL) util::log(__FILE__, __LINE__, MESSAGE, VAL)
#define DEBUG_LOG_RANGE(MESSAGE, FIRST, LAST) \
  util::log(__FILE__, __LINE__, MESSAGE, FIRST, LAST)

#else

#define DEBUG_LOG(MESSAGE) ((void)0)
#define DEBUG_LOG2(MESSAGE, VAL) ((void)0)
#define DEBUG_LOG_RANGE(MESSAGE, FIRST, LAST) ((void)0)

#endif // DEBUG_OUTPUT_ENABLED

#endif // NETSPEAK_UTIL_LOGGING_HPP
