// RawSearcher.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_RAW_SEARCHER_HPP
#define NETSPEAK_INVERTEDINDEX_RAW_SEARCHER_HPP

#include <limits>
#include <memory>
#include <string>

#include "netspeak/invertedindex/Properties.hpp"
#include "netspeak/invertedindex/RawPostlist.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class to search an inverted index.
 */
class RawSearcher {
public:
  virtual ~RawSearcher() {}

  virtual void close() = 0;

  virtual bool search_head(const std::string key, Head& head) = 0;

  virtual std::unique_ptr<RawPostlist> search_raw_postlist(
      const std::string& key, uint32_t begin, uint32_t len) = 0;

  virtual const Properties& properties() const = 0;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_RAW_SEARCHER_HPP
