// IndexStrategy.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_INDEX_STRATEGY_HPP
#define NETSPEAK_INVERTEDINDEX_INDEX_STRATEGY_HPP

#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/Record.hpp"

namespace netspeak {
namespace invertedindex {

template <typename T>
class IndexStrategy {
public:
  typedef Record<T> record_type;

  struct stats_type {
    stats_type() : key_count(0), value_count(0), total_size(0) {}
    uint64_t key_count;
    uint64_t value_count;
    uint64_t total_size;
  };

  IndexStrategy(const Configuration& config) : config_(config) {}

  virtual ~IndexStrategy() {}

  virtual void index() = 0;

  void insert(const record_type& record) {
    ++stats_.value_count;
    stats_.total_size += value::value_traits<T>::size_of(record.value());
    insert_(record);
  }

  void set_expected_record_count(uint64_t record_count) {
    if (config_.expected_record_count() < record_count) {
      config_.set_expected_record_count(record_count);
    }
  }

  const Configuration& config() const {
    return config_;
  }

  const stats_type& stats() const {
    return stats_;
  }

private:
  virtual void insert_(const record_type& record) = 0;

protected:
  Configuration config_;
  stats_type stats_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_INDEX_STRATEGY_HPP
