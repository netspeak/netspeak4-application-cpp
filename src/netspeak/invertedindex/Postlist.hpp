// Postlist.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_POSTLIST_HPP
#define NETSPEAK_INVERTEDINDEX_POSTLIST_HPP

#include "netspeak/invertedindex/RawPostlist.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * Specialized class to interpret raw values of a RawPostlist.
 */
template <typename T>
class Postlist : public RawPostlist {
public:
  typedef T value_type;

  Postlist() : RawPostlist() {}

  Postlist(const Head& head, const page_type& page) : RawPostlist(head, page) {}

  Postlist(const Head& head, const swap_type& swap) : RawPostlist(head, swap) {}

  Postlist(const Head& head, const page_type& page,
           const variable_size_iter::size_vector& sizes)
      : RawPostlist(head, page, sizes) {}

  Postlist(const Head& head, const swap_type& swap,
           const variable_size_iter::size_vector& sizes)
      : RawPostlist(head, swap, sizes) {}

  virtual ~Postlist() {}

  bool next(T& value) const {
    const char* buffer(RawPostlist::next());
    if (buffer == NULL)
      return false;
    value::value_traits<value_type>::copy_from(value, buffer);
    return true;
  }
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_POSTLIST_HPP
