// RawPostlist.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_ITERATOR_HPP
#define NETSPEAK_INVERTEDINDEX_ITERATOR_HPP

#include <numeric>
#include <ostream>
#include <vector>

#include <boost/utility.hpp>

#include "netspeak/invertedindex/ByteBuffer.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace invertedindex {

struct page_type {
  page_type(size_t size = 0)
      : buffer_(size), index_begin_(0), index_end_(0), index_cur_(0) {}

  page_type(const page_type& rhs)
      : buffer_(rhs.buffer_),
        index_begin_(rhs.index_begin_),
        index_end_(rhs.index_end_),
        index_cur_(rhs.index_cur_) {}

  void print(std::ostream& os) const {
    os << "{ buffer_size : " << buffer_.size()
       << ", index_begin : " << index_begin_ << ", index_end : " << index_end_
       << ", index_cur : " << index_cur_ << " }";
  }

  ByteBuffer buffer_;
  size_t index_begin_;
  size_t index_end_;
  size_t index_cur_;
};

struct swap_type {
  static const size_t default_pagesize = 5 * 1024 * 1024; // 5 MiB

  swap_type() : pagesize_(default_pagesize), offset_(0), stream_(NULL) {}

  swap_type(FILE* fs, size_t pagesize = default_pagesize)
      : pagesize_(pagesize), offset_(util::ftell(fs)), stream_(fs) {
    assert(pagesize_ != 0);
  }

  swap_type(const swap_type& rhs)
      : pagesize_(rhs.pagesize_), offset_(rhs.offset_), stream_(rhs.stream_) {}

  void print(std::ostream& os) const {
    os << "{ pagesize : " << pagesize_ << ", offset : " << offset_
       << ", stream : " << stream_ << " }";
  }

  size_t pagesize_;
  size_t offset_;
  FILE* stream_;
};

struct iterator_type : public boost::noncopyable {
  iterator_type() {}

  iterator_type(const page_type& page) : page_(page) {}

  iterator_type(const swap_type& swap) : swap_(swap) {}

  virtual ~iterator_type() {
    util::fclose(swap_.stream_);
  }

  void print(std::ostream& os) const {
    os << "{\n\t\tpage : ";
    page_.print(os);
    os << ",\n\t\tswap : ";
    swap_.print(os);
    os << "\n\t}";
  }

  virtual size_t byte_size() const = 0;

  virtual const char* next() = 0;

  virtual void rewind() = 0;

  virtual size_t size() const = 0;

  virtual void write(FILE* fs) = 0;

protected:
  page_type page_;
  swap_type swap_;
};

struct constant_size_iter : public iterator_type {
  constant_size_iter() : iterator_type(), count_(0), size_(0) {}

  constant_size_iter(size_t count, size_t size, const page_type& page)
      : iterator_type(page), count_(count), size_(size) {
    assert(size_ != 0);
    assert(page_.buffer_.size() >= byte_size());
    page_.index_begin_ = 0;
    page_.index_cur_ = 0;
    page_.index_end_ = count_;
  }

  constant_size_iter(size_t count, size_t size, const swap_type& swap)
      : iterator_type(swap), count_(count), size_(size) {
    assert(size_ != 0);
    page_.index_begin_ = 0;
    page_.index_cur_ = 0;
    page_.index_end_ = 0;
  }

  virtual ~constant_size_iter() {}

  inline size_t byte_size() const {
    return count_ * size_;
  }

  inline const char* next() {
    if (page_.index_cur_ == size())
      return NULL;
    if (page_.index_cur_ == page_.index_end_)
      swap();
    page_.buffer_.seek((page_.index_cur_++ - page_.index_begin_) * size_);
    return page_.buffer_.position();
  }

  inline void rewind() {
    page_.index_cur_ = 0;
    if (swap_.stream_ != NULL) {
      util::fseek(swap_.stream_, swap_.offset_, SEEK_SET);
      page_.index_begin_ = 0;
      page_.index_end_ = 0;
      page_.buffer_.clear();
    }
  }

  inline size_t size() const {
    return count_;
  }

  inline void write(FILE* fs) {
    rewind();
    if (swap_.stream_ == NULL) {
      page_.buffer_.write(fs);
    } else {
      while (swap() != 0) {
        page_.buffer_.write(fs);
      }
    }
  }

private:
  inline size_t swap() {
    const size_t remaining_value_count(size() - page_.index_end_);
    const size_t max_values_per_page(swap_.pagesize_ / size_);
    const size_t new_value_count(
        std::min(remaining_value_count, max_values_per_page));
    if (new_value_count != 0) {
      const size_t new_buffer_size(new_value_count * size_);
      page_.buffer_.resize(new_buffer_size);
      page_.buffer_.read(swap_.stream_);
      page_.index_begin_ = page_.index_end_;
      page_.index_cur_ = page_.index_begin_;
      page_.index_end_ += new_value_count;
    }
    return new_value_count;
  }

  const size_t count_;
  const size_t size_;
};

struct variable_size_iter : public iterator_type {
  typedef std::vector<uint32_t> size_vector;

  variable_size_iter(const size_vector& sizes, const page_type& page)
      : iterator_type(page), sizes_(sizes), offset_() {
    page_.index_begin_ = 0;
    page_.index_cur_ = 0;
    page_.index_end_ = sizes_.size();
  }

  variable_size_iter(const size_vector& sizes, const swap_type& swap)
      : iterator_type(swap), sizes_(sizes), offset_() {
    page_.index_begin_ = 0;
    page_.index_cur_ = 0;
    page_.index_end_ = 0;
  }

  virtual ~variable_size_iter() {}

  inline size_t byte_size() const {
    return size() * sizeof(size_vector::value_type) +
           std::accumulate(sizes_.begin(), sizes_.end(), 0);
  }

  inline const char* next() {
    if (page_.index_cur_ == size())
      return NULL;
    if (page_.index_cur_ == page_.index_end_)
      swap();
    page_.buffer_.seek(offset_);
    offset_ += sizes_.at(page_.index_cur_++);
    return page_.buffer_.position();
  }

  inline void rewind() {
    offset_ = 0;
    page_.index_cur_ = 0;
    if (swap_.stream_ != NULL) {
      util::fseek(swap_.stream_, swap_.offset_, SEEK_SET);
      page_.index_begin_ = 0;
      page_.index_end_ = 0;
      page_.buffer_.clear();
    }
  }

  inline size_t size() const {
    return sizes_.size();
  }

  inline void write(FILE* fs) {
    rewind();
    util::fwrite(sizes_.data(), sizeof(size_vector::value_type), sizes_.size(),
                 fs);
    if (swap_.stream_ == NULL) {
      page_.buffer_.write(fs);
    } else {
      while (swap() != 0) {
        page_.buffer_.write(fs);
      }
    }
  }

private:
  inline size_t swap() {
    size_t new_buffer_size(0);
    size_t new_value_count(0);
    for (unsigned i(page_.index_end_); i != sizes_.size(); ++i) {
      ++new_value_count;
      new_buffer_size += sizes_.at(i);
      if (new_buffer_size > swap_.pagesize_)
        break;
    }
    if (new_value_count != 0) {
      offset_ = 0;
      page_.buffer_.resize(new_buffer_size);
      page_.buffer_.read(swap_.stream_);
      page_.index_begin_ = page_.index_end_;
      page_.index_cur_ = page_.index_begin_;
      page_.index_end_ += new_value_count;
    }
    return new_value_count;
  }

  size_vector sizes_;
  size_t offset_;
};

inline std::ostream& operator<<(std::ostream& os, const page_type& page) {
  if (os)
    page.print(os);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const swap_type& swap) {
  if (os)
    swap.print(os);
  return os;
}

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_ITERATOR_HPP
