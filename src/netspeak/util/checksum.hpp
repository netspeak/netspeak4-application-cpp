// checksum.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_CHECKSUM_HPP
#define NETSPEAK_UTIL_CHECKSUM_HPP

#include <cmath>
#include <string>

/**
 * Functions to compute string checksums and prime numbers.
 */
namespace netspeak {
namespace util {

inline bool is_prime(uint32_t number) {
  if (number == 2)
    return true;
  if (number < 2 || number % 2 == 0)
    return false;
  for (unsigned i(3); i <= std::sqrt(number); ++++i) {
    if (number % i == 0)
      return false;
  }
  return true;
}

inline uint32_t next_prime(uint32_t number) {
  while (!is_prime(number)) ++number;
  return number;
}

// primary template
template <typename T>
T hash(const char* key, size_t length);

template <>
inline uint16_t hash<uint16_t>(const char* key, size_t length) {
  // Included from /usr/include/c++/4.4/tr1_impl/functional_hash.h
  uint16_t result = 0;
  for (; length > 0; --length) {
    result = (result * 131) + *key++;
  }
  return result;
}

template <>
inline uint32_t hash<uint32_t>(const char* key, size_t length) {
  // Included from /usr/include/c++/4.4/tr1_impl/functional_hash.h
  uint32_t result = static_cast<uint32_t>(2166136261UL);
  for (; length > 0; --length) {
    result ^= static_cast<uint32_t>(*key++);
    result *= static_cast<uint32_t>(16777619UL);
  }
  return result;
}

template <>
inline uint64_t hash<uint64_t>(const char* key, size_t length) {
  // Included from /usr/include/c++/4.4/tr1_impl/functional_hash.h
  uint64_t result = static_cast<uint64_t>(14695981039346656037ULL);
  for (; length > 0; --length) {
    result ^= static_cast<uint64_t>(*key++);
    result *= static_cast<uint64_t>(1099511628211ULL);
  }
  return result;
}

template <typename T>
T hash(const std::string& key) {
  return hash<T>(key.c_str(), key.size());
}

// free functions

inline size_t hash(const std::string& key) {
  return hash<size_t>(key.c_str(), key.size());
}

inline uint16_t hash16(const std::string& key) {
  return hash<uint16_t>(key.c_str(), key.size());
}

inline uint32_t hash32(const std::string& key) {
  return hash<uint32_t>(key.c_str(), key.size());
}

inline uint64_t hash64(const std::string& key) {
  return hash<uint64_t>(key.c_str(), key.size());
}

} // namespace util
} // namespace netspeak

#endif // NETSPEAK_UTIL_CHECKSUM_HPP
