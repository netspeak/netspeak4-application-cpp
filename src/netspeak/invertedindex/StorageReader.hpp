// StorageReader.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_STORAGE_READER_HPP
#define NETSPEAK_INVERTEDINDEX_STORAGE_READER_HPP

#include <memory>
#include <mutex>
#include <type_traits>

#include <boost/filesystem.hpp>

#include "netspeak/bighashmap/BigHashMap.hpp"
#include "netspeak/invertedindex/PostlistReader.hpp"
#include "netspeak/invertedindex/StorageWriter.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace invertedindex {

namespace bfs = boost::filesystem;

template <typename T, bool ThreadSafe>
class StorageReader {
private:
  typedef std::vector<FILE*> FileVector;
  typedef std::vector<bfs::path> PathVector;
  typedef typename StorageWriter<T>::Address Address;
  typedef bighashmap::BigHashMap<Address, ThreadSafe> Map;
  typedef std::integral_constant<bool, ThreadSafe> IsThreadSafe;

  inline bool ReadHead(FILE* file, uint32_t offset, Head& head,
                       std::false_type) const {
    util::fseek(file, offset, SEEK_SET);
    util::fread(&head, sizeof(head), 1, file);
    return true;
  }

  // Thread-safe version.
  inline bool ReadHead(FILE* file, uint32_t offset, Head& head,
                       std::true_type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ReadHead(file, offset, head, std::false_type());
  }

  inline std::unique_ptr<Postlist<T> > ReadPostlist(
      const bfs::path& path, FILE* file, uint32_t offset, uint32_t begin,
      uint32_t length, uint32_t page_size, std::false_type) const {
    util::fseek(file, offset, SEEK_SET);
    return PostlistReader<T>::read(path, file, begin, length, page_size);
  }

  // Thread-safe version.
  inline std::unique_ptr<Postlist<T> > ReadPostlist(
      const bfs::path& path, FILE* file, uint32_t offset, uint32_t begin,
      uint32_t length, uint32_t page_size, std::true_type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ReadPostlist(path, file, offset, begin, length, page_size,
                        std::false_type());
  }

public:
  StorageReader() {}

  StorageReader(const bfs::path& directory, util::memory_type memory) {
    Open(directory, memory);
  }

  ~StorageReader() {
    Close();
  }

  void Close() {
    table_.reset(); // calls delete
    for (auto it(files_.begin()); it != files_.end(); ++it) {
      util::fclose(*it);
    }
    files_.clear();
    paths_.clear();
  }

  bool IsOpen() const {
    return !files_.empty();
  }

  void Open(const bfs::path& directory, util::memory_type memory) {
    if (IsOpen())
      return;

    const bfs::path data_dir(directory / StorageWriter<T>::k_data_dir);
    const bfs::path table_dir(directory / StorageWriter<T>::k_table_dir);

    // open header table
    table_.reset(Map::Open(table_dir, memory));

    // open data files
    const size_t data_file_count(std::distance(
        bfs::directory_iterator(data_dir), bfs::directory_iterator()));
    files_.reserve(data_file_count);
    paths_.reserve(data_file_count);
    for (unsigned i(0); i != data_file_count; ++i) {
      const std::string num(util::to_string(i));
      paths_.push_back(data_dir / (StorageWriter<T>::k_data_file + num));
      files_.push_back(util::fopen(paths_.back(), "rb"));
    }
  }

  bool ReadHead(const std::string& key, Head& head) {
    Address address;
    if (table_ && table_->Get(key, address) && address.e1() < files_.size()) {
      FILE* file = files_[address.e1()];
      const uint32_t offset = address.e2();
      return ReadHead(file, offset, head, IsThreadSafe());
    }
    return false;
  }

  /**
   * Returns the postlist for a given \c key, which may be truncated at the
   * the beginning by the value of \c begin and at the end by the value
   * of \c length. If a mapping for \c key exists but the other parameters
   * truncate the postlist completely, then an empty postlist will be
   * returned. Otherwise, if no mapping exists at all NULL will be returned
   * to indicate that case.
   *
   * @param key the key of the wanted postlist.
   * @param begin the index of the first element.
   * @param length the number of the element to read.
   * @param page_size the size of the postlists swap buffer in bytes.
   * @return a postlist, which can be empty or null.
   */
  std::unique_ptr<Postlist<T> > ReadPostlist(
      const std::string& key, uint32_t begin = 0,
      uint32_t length = std::numeric_limits<uint32_t>::max(),
      uint32_t page_size = swap_type::default_pagesize) {
    Address address;
    std::unique_ptr<Postlist<T> > postlist;
    if (table_ && table_->Get(key, address) && address.e1() < files_.size()) {
      FILE* file = files_[address.e1()];
      const uint32_t offset = address.e2();
      const bfs::path& path = paths_[address.e1()];
      postlist = ReadPostlist(path, file, offset, begin, length, page_size,
                              IsThreadSafe());
    }
    return postlist;
  }

private:
  std::unique_ptr<Map> table_;
  FileVector files_;
  PathVector paths_;
  mutable std::mutex mutex_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_STORAGE_READER_HPP
