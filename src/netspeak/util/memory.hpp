// memory.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_MEMORY_HPP
#define NETSPEAK_UTIL_MEMORY_HPP

#include <cstdint>
#include <string>

namespace netspeak {
namespace util {

enum class memory_type {
  min_required,
  mb512,
  mb1024,
  mb2048,
  mb4096,
  mb8192,
  mb16384,
  mb32768,
  mb65536
};

inline uint64_t to_bytes(memory_type memory) {
  uint64_t bytes(1024 * 1024); // 1 MiB
  switch (memory) {
    case memory_type::mb512:
      return bytes * 512;
    case memory_type::mb1024:
      return bytes * 1024;
    case memory_type::mb2048:
      return bytes * 2048;
    case memory_type::mb4096:
      return bytes * 4096;
    case memory_type::mb8192:
      return bytes * 8192;
    case memory_type::mb16384:
      return bytes * 16384;
    case memory_type::mb32768:
      return bytes * 32768;
    case memory_type::mb65536:
      return bytes * 65536;
    default:
      return 0;
  }
}

inline std::string to_string(memory_type memory) {
  switch (memory) {
    case memory_type::mb512:
      return "mb512";
    case memory_type::mb1024:
      return "mb1024";
    case memory_type::mb2048:
      return "mb2048";
    case memory_type::mb4096:
      return "mb4096";
    case memory_type::mb8192:
      return "mb8192";
    case memory_type::mb16384:
      return "mb16384";
    case memory_type::mb32768:
      return "mb32768";
    case memory_type::mb65536:
      return "mb65536";
    default:
      return "min_required";
  }
}

} // namespace util
} // namespace netspeak

#endif // NETSPEAK_UTIL_MEMORY_HPP
