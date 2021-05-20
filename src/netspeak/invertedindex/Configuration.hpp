// Configuration.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_CONFIGURATION_HPP
#define NETSPEAK_INVERTEDINDEX_CONFIGURATION_HPP

#include <cstdint>

#include <boost/filesystem.hpp>

#include "netspeak/util/conversion.hpp"
#include "netspeak/util/memory.hpp"

namespace netspeak {
namespace invertedindex {

enum class key_sorting_type { unsorted, sorted };

enum class value_sorting_type { disabled, ascending, descending };

inline std::string to_string(key_sorting_type sorting) {
  switch (sorting) {
    case key_sorting_type::unsorted:
      return "unsorted";
    case key_sorting_type::sorted:
      return "sorted";
    default:
      return "unknown";
  }
}

inline std::string to_string(value_sorting_type sorting) {
  switch (sorting) {
    case value_sorting_type::disabled:
      return "disabled";
    case value_sorting_type::ascending:
      return "ascending";
    case value_sorting_type::descending:
      return "descending";
    default:
      return "unknown";
  }
}

inline std::ostream& operator<<(std::ostream& os, value_sorting_type sorting) {
  return os << to_string(sorting);
}

class Configuration {
public:
  Configuration()
      : key_sorting_(key_sorting_type::unsorted),
        value_sorting_(value_sorting_type::disabled),
        max_memory_usage_(util::memory_type::mb1024),
        expected_record_count_(0) {}

  ~Configuration() {}

public:
  uint64_t expected_record_count() const {
    return expected_record_count_;
  }

  const std::string& input_file() const {
    return input_file_;
  }

  const std::string& input_directory() const {
    return input_directory_;
  }

  const std::string& index_directory() const {
    return index_directory_;
  }

  key_sorting_type key_sorting() const {
    return key_sorting_;
  }

  util::memory_type max_memory_usage() const {
    return max_memory_usage_;
  }

  value_sorting_type value_sorting() const {
    return value_sorting_;
  }

  void print(std::ostream& os) const {
    const std::string exp_rec_cnt(
        expected_record_count_ == 0 ? "undefined"
                                    : util::to_string(expected_record_count_));
    os << "{\n  expected_record_count : " << exp_rec_cnt << ",\n  input_file : "
       << (input_file_.empty() ? "undefined" : input_file_)
       << ",\n  input_directory : "
       << (input_directory_.empty() ? "undefined" : input_directory_)
       << ",\n  index_directory : "
       << (index_directory_.empty() ? "undefined" : index_directory_)
       << ",\n  key_sorting : " << to_string(key_sorting_)
       << ",\n  value_sorting : " << to_string(value_sorting_)
       << ",\n  max_memory_usage : " << util::to_string(max_memory_usage_)
       << "\n}";
  }

  void set_expected_record_count(uint64_t record_count) {
    expected_record_count_ = record_count;
  }

  void set_input_file(const std::string& file) {
    input_file_ = file;
  }

  void set_input_directory(const std::string& dir) {
    input_directory_ = dir;
  }

  void set_index_directory(const std::string& dir) {
    index_directory_ = dir;
  }

  void set_key_sorting(key_sorting_type sorting) {
    key_sorting_ = sorting;
  }

  void set_max_memory_usage(util::memory_type memory) {
    max_memory_usage_ = memory;
  }

  void set_value_sorting(value_sorting_type sorting) {
    value_sorting_ = sorting;
  }

private:
  std::string input_file_;
  std::string input_directory_;
  std::string index_directory_;
  key_sorting_type key_sorting_;
  value_sorting_type value_sorting_;
  util::memory_type max_memory_usage_;
  uint64_t expected_record_count_;
};

inline std::ostream& operator<<(std::ostream& os, const Configuration& config) {
  if (os)
    config.print(os);
  return os;
}

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_CONFIGURATION_HPP
