// systemio.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_UTIL_SYSTEMIO_HPP
#define NETSPEAK_UTIL_SYSTEMIO_HPP

#include <climits>
#include <cstring>

#include <boost/filesystem.hpp>

#include "netspeak/util/exception.hpp"

/**
 * Functions to replace FILE* I/O to throw exceptions on failure.
 */
namespace netspeak {
namespace util {

namespace bfs = boost::filesystem;

inline bool is_hidden_file(const bfs::path& path) {
  return path.filename().string().front() == '.';
}

/**
 * Converts the given 'path' to an absolute path using 'base' as base path.
 * If 'path' is already an absolute path it is returned without modification.
 */
inline bfs::path make_absolute(const bfs::path& path, const bfs::path& base) {
  return path.is_absolute() ? path : (base / path);
}

inline void signal_error(const std::string& msg) {
  std::perror(msg.c_str());
  throw_runtime_error(msg);
}

template <typename T>
void signal_error(const std::string& msg, const T& obj) {
  std::perror(msg.c_str());
  throw_runtime_error(msg, obj);
}

inline uint64_t directory_size(const bfs::path& dir) {
  uint64_t num_bytes = 0;
  const bfs::directory_iterator end;
  for (bfs::directory_iterator it(dir); it != end; ++it) {
    if (bfs::is_regular_file(it->path())) {
      if (!is_hidden_file(it->path())) {
        num_bytes += bfs::file_size(it->path());
      }
    } else if (bfs::is_directory(it->path())) {
      num_bytes += directory_size(it->path());
    }
  }
  return num_bytes;
}

inline void fclose(FILE* fs) {
  if (fs == NULL)
    return;
  if (std::fclose(fs) == -1) {
    signal_error("std::fclose failed");
  }
}

inline FILE* fopen(const bfs::path& path, const std::string& mode) {
  FILE* fs(std::fopen(path.string().c_str(), mode.c_str()));
  if (fs == NULL) {
    signal_error("std::fopen failed", path);
  }
  return fs;
}

inline void fread(void* data, size_t size, size_t count, FILE* fs) {
  // As it turns out, simply asserting NULL is not enough. As the `std::fread`
  // spec clearly states:
  //
  // > f size or count is zero, fread returns zero and performs no other
  // > action.
  //
  // Since `data` may be the `data()` value of a vector, we have to take this
  // into account because `data()` may return NULL for empty vectors depending
  // on the compiler.
  assert(data != NULL || size == 0 || count == 0);
  assert(fs != NULL);

  if (std::fread(data, size, count, fs) != count) {
    signal_error("std::fread failed");
  }
}

inline void fseek(FILE* fs, long offset, int origin) {
  assert(fs != NULL);
  if (std::fseek(fs, offset, origin) != 0) {
    signal_error("std::fseek failed");
  }
}

inline void fwrite(const void* data, size_t size, size_t count, FILE* fs) {
  // Same as for `util::fread`
  assert(data != NULL || size == 0 || count == 0);
  assert(fs != NULL);

  if (std::fwrite(data, size, count, fs) != count) {
    signal_error("std::fwrite failed");
  }
}

inline size_t ftell(FILE* fs) {
  assert(fs != NULL);
  const long offset(std::ftell(fs));
  if (offset == -1) {
    signal_error("std::ftell failed");
  }
  return offset;
}

inline void rewind(FILE* fs) {
  if (fs == NULL)
    return;
  std::rewind(fs);
}

inline FILE* tmpfile() {
  FILE* fs(std::tmpfile());
  if (fs == NULL) {
    signal_error("std::tmpfile failed");
  }
  return fs;
}

inline const bfs::path tmpdir(const bfs::path& parent_dir) {
  char tmpdir[PATH_MAX];
  const std::string pattern("/tmp_XXXXXX");
  const std::string parent_str(parent_dir.string());
  std::strncpy(tmpdir, parent_str.c_str(), parent_str.size() + 1);
  std::strncat(tmpdir, pattern.c_str(), pattern.size() + 1);
  if (mkdtemp(tmpdir) == NULL) {
    signal_error("mkdtemp failed");
  }
  return tmpdir;
}

inline void CreateOrCheckIfEmpty(const bfs::path& directory) {
  if (bfs::is_directory(directory)) {
    if (!bfs::is_empty(directory)) {
      signal_error("is not empty", directory);
    }
  } else {
    bfs::create_directories(directory);
  }
}

} // namespace util
} // namespace netspeak

#endif // NETSPEAK_UTIL_SYSTEMIO_HPP
