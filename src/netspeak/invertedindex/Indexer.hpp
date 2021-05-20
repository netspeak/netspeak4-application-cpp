// Indexer.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_INDEXER_HPP
#define NETSPEAK_INVERTEDINDEX_INDEXER_HPP

#include <cstring>
#include <memory>
#include <ostream>

#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/IndexStrategy.hpp"
#include "netspeak/invertedindex/RawIndexer.hpp"
#include "netspeak/invertedindex/SortedInput.hpp"
#include "netspeak/invertedindex/UnsortedInput.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class to create an inverted index from a number of records.
 */
template <typename T>
class Indexer : public RawIndexer {
public:
  typedef T value_type;
  typedef typename IndexStrategy<value_type>::record_type record_type;

  Indexer(const Configuration& config) {
    assert_config(config);
    util::log("Configuration", config);
    strategy_ =
        config.key_sorting() == key_sorting_type::sorted
            ? std::unique_ptr<IndexStrategy<typename record_type::value_type> >(
                  new SortedInput<typename record_type::value_type>(config))
            : std::unique_ptr<IndexStrategy<typename record_type::value_type> >(
                  new UnsortedInput<typename record_type::value_type>(config));
  }

  virtual ~Indexer() {}

  const Properties index() {
    Properties props;
    strategy_->index();
    props.key_count = strategy_->stats().key_count;
    props.value_count = strategy_->stats().value_count;
    props.total_size = strategy_->stats().total_size;
    props.value_sorting = strategy_->config().value_sorting();
    const std::string tn(
        value::value_traits<typename record_type::value_type>::type_name());
    std::strncpy(props.value_type, tn.c_str(), sizeof(props.value_type));
    props.write(bfs::path(strategy_->config().index_directory()) /
                Properties::default_filename());
    util::log("Indexing succeeded");
    util::log("Properties", props);
    return props;
  }

  void insert(const record_type& record) {
    assert_record(record);
    strategy_->insert(record);
    if (!(strategy_->stats().value_count % 10000000)) {
      util::log("#records inserted", strategy_->stats().value_count);
    }
  }

  void insert(const std::string& key, const value_type& value) {
    insert(record_type(key, value));
  }

  void insert(const std::string& key, const char* value) {
    record_type record(key);
    value::value_traits<typename record_type::value_type>::copy_from(
        record.value(), value);
    insert(record);
  }

  void set_expected_record_count(uint64_t record_count) {
    strategy_->set_expected_record_count(record_count);
  }

private: // class member
  static void assert_config(const Configuration& config) {
    if (!bfs::exists(config.index_directory())) {
      util::throw_invalid_argument("Does not exist", config.index_directory());
    }
    if (!bfs::is_directory(config.index_directory())) {
      util::throw_invalid_argument("Not a directory", config.index_directory());
    }
    if (!bfs::is_empty(config.index_directory())) {
      util::throw_invalid_argument("Is not empty", config.index_directory());
    }
  }

  static void assert_record(const record_type& record) {
    typedef value::value_traits<typename record_type::key_type> ktraits_t;
    typedef value::value_traits<typename record_type::value_type> vtraits_t;

    if (record.key().size() >
        std::numeric_limits<typename ktraits_t::io_size_type>::max()) {
      util::throw_invalid_argument("Too long key", record);
    }
    if (vtraits_t::size_of(record.value()) >
        std::numeric_limits<typename vtraits_t::io_size_type>::max()) {
      util::throw_invalid_argument("Too long value", record);
    }
  }

private:
  std::unique_ptr<IndexStrategy<typename record_type::value_type> > strategy_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_INDEXER_HPP
