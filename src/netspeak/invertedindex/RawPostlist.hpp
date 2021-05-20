// RawPostlist.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_RAW_POSTLIST_HPP
#define NETSPEAK_INVERTEDINDEX_RAW_POSTLIST_HPP

#include <memory>

#include "netspeak/invertedindex/Iterator.hpp"

namespace netspeak {
namespace invertedindex {

struct Head {
  Head() : value_count(), value_size(), total_size() {}

  void PrintTo(std::ostream& os) const {
    os << "{ value_count : " << value_count << ", value_size : " << value_size
       << ", total_size : " << total_size << " }";
  }

  uint32_t value_count;
  uint32_t value_size;
  uint32_t total_size;
} __attribute__((packed));

class RawPostlist {
public:
  RawPostlist() : iter_(new constant_size_iter) {}

  RawPostlist(const Head& head, const page_type& page)
      : iter_(new constant_size_iter(head.value_count, head.value_size, page)),
        head_(head) {}

  RawPostlist(const Head& head, const swap_type& swap)
      : iter_(new constant_size_iter(head.value_count, head.value_size, swap)),
        head_(head) {
    assert(swap.stream_ != NULL);
  }

  RawPostlist(const Head& head, const page_type& page,
              const variable_size_iter::size_vector& sizes)
      : iter_(new variable_size_iter(sizes, page)), head_(head) {
    assert(head.value_count == sizes.size());
  }

  RawPostlist(const Head& head, const swap_type& swap,
              const variable_size_iter::size_vector& sizes)
      : iter_(new variable_size_iter(sizes, swap)), head_(head) {
    assert(head.value_count == sizes.size());
    assert(swap.stream_ != NULL);
  }

  virtual ~RawPostlist(){};

  inline size_t byte_size() const {
    return sizeof(Head) + iter_->byte_size();
  }

  inline bool empty() const {
    return size() == 0;
  }

  inline const Head& head() const {
    return head_;
  }

  inline const char* next() const {
    return iter_->next();
  }

  inline void print(std::ostream& os) const {
    os << "{\n\thead : ";
    head_.PrintTo(os);
    os << ",\n\titer : ";
    iter_->print(os);
    os << "\n}";
  }

  inline void rewind() const {
    iter_->rewind();
  }

  inline size_t size() const {
    return iter_->size();
  }

  inline void write(FILE* fs) {
    util::fwrite(&head_, sizeof(head_), 1, fs);
    iter_->write(fs);
  }

private:
  std::unique_ptr<iterator_type> iter_;
  Head head_;
};

inline std::ostream& operator<<(std::ostream& os, const Head& head) {
  if (os)
    head.PrintTo(os);
  return os;
}

inline std::ostream& operator<<(std::ostream& os, const RawPostlist& postlist) {
  if (os)
    postlist.print(os);
  return os;
}

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_RAW_POSTLIST_HPP
