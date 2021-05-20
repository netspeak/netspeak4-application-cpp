// Searcher.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_SEARCHER_HPP
#define NETSPEAK_INVERTEDINDEX_SEARCHER_HPP

#include <boost/filesystem.hpp>

#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/invertedindex/RawSearcher.hpp"
#include "netspeak/invertedindex/StorageReader.hpp"

namespace netspeak {
namespace invertedindex {

/**
 * A class to search an inverted index.
 * By default instances of this class are thread-safe.
 */
template <typename Value, bool ThreadSafe = true>
class Searcher : public RawSearcher {
public:
  Searcher() {}

  Searcher(const Configuration& config) {
    open(config);
  }

  virtual ~Searcher() {
    close();
  }

  virtual void close() {
    storage_.Close();
  }

  bool is_open() const {
    return storage_.IsOpen();
  }

  void open(const Configuration& config) {
    if (is_open())
      return;
    config_ = config;

    // load properties
    const boost::filesystem::path filename(
        boost::filesystem::path(config.index_directory()) /
        Properties::default_filename());
    FILE* rfs(util::fopen(filename, "rb"));
    util::fread(&props_, sizeof(props_), 1, rfs);
    util::fclose(rfs);

    // check properties
    assert_properties(props_);

    // open storage
    if (props_.key_count == 0) {
      util::log("Searcher", "Index seems to be empty");
    } else {
      storage_.Open(config.index_directory(), config.max_memory_usage());
    }
  }

  virtual bool search_head(const std::string key, Head& head) {
    return storage_.ReadHead(key, head);
  }

  /**
   * Returns the postlist for a given \c key, which may be truncated at the
   * the beginning by the value of \c begin and at the end by the value
   * of \c length. If a mapping for \c key exists but the other parameters
   * truncate the postlist completely, then an empty postlist will be
   * returned. Otherwise, if no mapping exists at all NULL will be returned
   * to indicate that case.
   */
  std::unique_ptr<Postlist<Value> > search_postlist(
      const std::string& key, uint32_t begin = 0,
      uint32_t length = std::numeric_limits<uint32_t>::max()) {
    std::unique_ptr<Postlist<Value> > plist;
    try {
      plist = storage_.ReadPostlist(key, begin, length);
    } catch (const std::exception& error) {
      util::log("Exception occurs", error.what());
    } catch (...) {
      util::log("WTF what's happened");
    }
    return plist;
  }

  virtual std::unique_ptr<RawPostlist> search_raw_postlist(
      const std::string& key, uint32_t begin, uint32_t len) {
    auto plist = search_postlist(key, begin, len);
    return std::unique_ptr<RawPostlist>(plist.release());
  }

  const Configuration& configuration() const {
    return config_;
  }

  virtual const Properties& properties() const {
    return props_;
  }

private:
  static void assert_properties(const Properties& properties) {
    if (properties.version_number != Properties::k_version_number) {
      std::ostringstream oss;
      oss << "The version number of the index you want load is "
          << properties.version_number
          << ", but your installed library is of different version "
          << Properties::k_version_number
          << ". Please install the correct library.";
      util::throw_domain_error("Version conflict", oss.str());
    }

    // check value type
    const std::string tn(value::value_traits<Value>::type_name());
    if (tn != std::string(properties.value_type)) {
      std::ostringstream oss;
      oss << "The value type of the index you want load is "
          << properties.value_type
          << ", but your value type parameterization is " << tn
          << ". Please reload with the correct value type.";
      util::throw_domain_error("Wrong value type", oss.str());
    }
  }

  Configuration config_;
  Properties props_;
  StorageReader<Value, ThreadSafe> storage_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_SEARCHER_HPP
