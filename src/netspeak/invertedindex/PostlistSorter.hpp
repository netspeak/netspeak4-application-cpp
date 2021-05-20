// PostlistSorter.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_POSTLIST_SORTER_HPP
#define NETSPEAK_INVERTEDINDEX_POSTLIST_SORTER_HPP

#include <cassert>

#include "netspeak/invertedindex/PostlistBuilder.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class that provides a static method to sort a huge postlist with an
 * external n-merge-sort algorithm. If the size of the postlist is less than
 * max_memory_usage_mb sorting falls back to the std::sort algorithm.
 */
template <typename T, typename Comp>
class PostlistSorter {
public:
  typedef T value_type;
  typedef Comp compare_type;
  typedef value::value_traits<value_type> traits_type;

private: // disallow instantiation
  PostlistSorter();

public:
  static std::unique_ptr<Postlist<value_type> > sort(
      Postlist<value_type>& postlist, size_t max_memory_usage_mb = 100) {
    const size_t max_bucket_size(1 + max_memory_usage_mb * 1024 * 1024);
    const size_t bucket_count(postlist.head().total_size / max_bucket_size);
    if (bucket_count == 0) {
      return internal_sort(postlist);
    } else {
      std::vector<FILE*> buckets(scatter(postlist, bucket_count));
      return merge(buckets);
    }
  }

private:
  static std::unique_ptr<Postlist<value_type> > internal_sort(
      Postlist<value_type>& postlist) {
    value_type value;
    postlist.rewind();
    std::vector<value_type> values;
    values.reserve(postlist.size());
    while (postlist.next(value)) {
      values.push_back(value);
    }

    assert(values.size() == postlist.size());

    std::sort(values.begin(), values.end(), compare_type());
    PostlistBuilder<value_type> builder;
    for (auto it(values.begin()); it != values.end(); ++it) {
      builder.push_back(*it);
    }
    return builder.build();
  }

  static std::unique_ptr<Postlist<value_type> > merge(
      std::vector<FILE*>& buckets) {
    // initialize value streams
    std::vector<value_type> values(buckets.size());
    for (unsigned i(0); i != buckets.size(); ++i) {
      std::rewind(buckets[i]);
      if (!traits_type::read_from(values[i], buckets[i])) {
        util::throw_runtime_error("traits_type::read_from failed for",
                                  values[i]);
      }
    }
    // merge value streams
    size_t bucket_index;
    PostlistBuilder<value_type> builder;
    typename std::vector<value_type>::iterator next_value;
    while (!values.empty()) {
      next_value =
          std::min_element(values.begin(), values.end(), compare_type());
      builder.push_back(*next_value);
      bucket_index = std::distance(values.begin(), next_value);
      if (!traits_type::read_from(*next_value, buckets[bucket_index])) {
        values.erase(next_value);
        util::fclose(buckets[bucket_index]);
        buckets.erase(buckets.begin() + bucket_index);
      }
    }
    return builder.build();
  }

  static const std::vector<FILE*> scatter(Postlist<value_type>& postlist,
                                          size_t bucket_count) {
    value_type value;
    postlist.rewind();
    std::vector<FILE*> bucket_fs;
    std::vector<value_type> values;
    if (bucket_count == 0)
      ++bucket_count;
    const size_t max_values_per_bucket(1 + postlist.size() / bucket_count);
    values.reserve(max_values_per_bucket);
    while (postlist.next(value)) {
      values.push_back(value);
      if (values.size() == max_values_per_bucket) {
        std::sort(values.begin(), values.end(), compare_type());
        bucket_fs.push_back(write_to_tmpfile(values));
        values.clear();
      }
    }
    if (!values.empty()) {
      std::sort(values.begin(), values.end(), compare_type());
      bucket_fs.push_back(write_to_tmpfile(values));
    }
    return bucket_fs;
  }

  static FILE* write_to_tmpfile(const std::vector<value_type>& values) {
    FILE* tmp_fs(util::tmpfile());
    for (auto it(values.begin()); it != values.end(); ++it) {
      if (!traits_type::write_to(*it, tmp_fs)) {
        util::throw_runtime_error("traits_type::write_to failed for", *it);
      }
    }
    return tmp_fs;
  }
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_POSTLIST_SORTER_HPP
