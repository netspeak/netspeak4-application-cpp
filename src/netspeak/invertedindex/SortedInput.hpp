// SortedInput.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_SORTED_INPUT_HPP
#define NETSPEAK_INVERTEDINDEX_SORTED_INPUT_HPP

#include <memory>

#include "netspeak/invertedindex/IndexStrategy.hpp"
#include "netspeak/invertedindex/PostlistBuilder.hpp"
#include "netspeak/invertedindex/PostlistSorter.hpp"
#include "netspeak/invertedindex/StorageWriter.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * This strategy is for building an index from record sequences
 * whose values are sorted, e.g. those extracted from real inverted files.
 */
template <typename T>
class SortedInput : public IndexStrategy<T> {
public:
  typedef typename IndexStrategy<T>::record_type record_type;
  typedef typename record_type::value_type value_type;

  SortedInput(const Configuration& config)
      : IndexStrategy<value_type>(config), storage_(config.index_directory()) {}

  virtual ~SortedInput() {}

  virtual void index() {
    write_current_postlist_();
    storage_.close();
  }

private:
  void insert_(const record_type& record) {
    if (key_ != record.key()) {
      if (!key_.empty()) { // important for the very first call
        write_current_postlist_();
      }
      key_ = record.key();
    }
    builder_.push_back(record.value());
  }

  void write_current_postlist_() {
    auto postlist = builder_.build();
    switch (this->config().value_sorting()) {
      case value_sorting_type::ascending:
        postlist =
            PostlistSorter<value_type, std::less<value_type> >::sort(*postlist);
        break;
      case value_sorting_type::descending:
        postlist = PostlistSorter<value_type, std::greater<value_type> >::sort(
            *postlist);
        break;
      default:;
    }
    if (storage_.write(key_, *postlist)) {
      this->stats_.key_count++;
    }
  }

  PostlistBuilder<value_type> builder_;
  StorageWriter<value_type> storage_;
  std::string key_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_SORTED_INPUT_HPP
