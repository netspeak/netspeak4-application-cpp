// PostlistReader.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_POSTLIST_READER_HPP
#define NETSPEAK_INVERTEDINDEX_POSTLIST_READER_HPP

#include <climits>
#include <cmath>
#include <memory>
#include <numeric>

#include <boost/filesystem.hpp>

#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/util/logging.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class to read postlists from a binary file.
 */
template <typename T>
class PostlistReader {
private:
  typedef variable_size_iter::size_vector size_vector;

  PostlistReader();

public:
  static std::unique_ptr<Postlist<T>> read(
      const boost::filesystem::path& path, FILE* file, uint32_t index_begin = 0,
      uint32_t value_count = std::numeric_limits<uint32_t>::max(),
      uint32_t page_size = swap_type::default_pagesize) {
    std::unique_ptr<Postlist<T>> postlist;

    // load postlist head
    Head head;
    util::fread(&head, sizeof(head), 1, file);
    index_begin = std::min(index_begin, head.value_count);
    value_count = std::min(value_count, head.value_count - index_begin);
    std::size_t begin_of_payload = util::ftell(file);
    if (head.value_size == 0) // variable value size
    {
      begin_of_payload += head.value_count * sizeof(size_vector::value_type);

      // read value sizes to skip [0, index_begin)
      size_vector value_sizes(index_begin);
      util::fread(value_sizes.data(), sizeof(size_vector::value_type),
                  value_sizes.size(), file);
      const std::size_t offset_of_index_begin = std::accumulate(
          value_sizes.begin(), value_sizes.end(), begin_of_payload);

      // read value sizes to use [index_begin, index_begin + value_count)
      value_sizes.resize(value_count);
      util::fread(value_sizes.data(), sizeof(size_vector::value_type),
                  value_sizes.size(), file);
      const std::size_t total_payload_size =
          std::accumulate(value_sizes.begin(), value_sizes.end(), 0);

      // build new head
      Head new_head;
      new_head.value_count = value_count;
      new_head.value_size = head.value_size;
      new_head.total_size = total_payload_size;
      if (total_payload_size > page_size) {
        // leave payload in swap file
        FILE* swap_file = util::fopen(path, "rb");
        util::fseek(swap_file, offset_of_index_begin, SEEK_SET);
        util::log(__FILE__, __LINE__, "util::ftell", util::ftell(swap_file));
        const swap_type swap(swap_file);
        postlist.reset(new Postlist<T>(new_head, swap, value_sizes));
      } else {
        // read payload into buffer
        page_type page(total_payload_size);
        util::fseek(file, offset_of_index_begin, SEEK_SET);
        util::log((__FILE__), (__LINE__), "util::ftell", util::ftell(file));
        page.buffer_.read(file);
        postlist.reset(new Postlist<T>(new_head, page, value_sizes));
      }
    } else {
      const std::size_t offset_of_index_begin =
          begin_of_payload + index_begin * head.value_size;
      const std::size_t total_payload_size = value_count * head.value_size;
      // build new head
      Head new_head;
      new_head.value_count = value_count;
      new_head.value_size = head.value_size;
      new_head.total_size = total_payload_size;
      if (total_payload_size > page_size) {
        // read payload to use from swap file
        FILE* swap_file = util::fopen(path, "rb");
        util::fseek(swap_file, offset_of_index_begin, SEEK_SET);
        const swap_type swap(swap_file);
        postlist.reset(new Postlist<T>(new_head, swap));
      } else {
        // read payload to use into buffer
        page_type page(total_payload_size);
        util::fseek(file, offset_of_index_begin, SEEK_SET);
        page.buffer_.read(file);
        postlist.reset(new Postlist<T>(new_head, page));
      }
    }
    const std::size_t end_of_postlist = begin_of_payload + head.total_size;
    util::fseek(file, end_of_postlist, SEEK_SET);
    return postlist;
  }
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_POSTLIST_READER_HPP
