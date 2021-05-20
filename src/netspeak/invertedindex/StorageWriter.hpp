// StorageWriter.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_STORAGE_WRITER_HPP
#define NETSPEAK_INVERTEDINDEX_STORAGE_WRITER_HPP

#include <boost/filesystem/fstream.hpp>
#include <boost/utility.hpp>

#include "netspeak/bighashmap/Builder.hpp"
#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/invertedindex/Record.hpp"
#include "netspeak/util/conversion.hpp"

namespace netspeak {
namespace invertedindex {

namespace bfs = boost::filesystem;

template <typename T>
class StorageWriter : public boost::noncopyable {
public:
  /**
   * The header table's value type.
   */
  typedef value::pair<uint16_t, uint32_t> Address;

  static const std::string k_data_dir;
  static const std::string k_data_file;
  static const std::string k_table_dir;
  static const std::string k_table_file;

  static const size_t data_file_size_max = 1024 * 1024 * 1024; // 1GB

  StorageWriter(const bfs::path& directory)
      : directory_(directory), data_file_cnt_(), data_wfs_(NULL) {
    if (!bfs::exists(directory)) {
      util::throw_invalid_argument("Does not exist", directory);
    }
    if (!bfs::is_directory(directory)) {
      util::throw_invalid_argument("Not a directory", directory);
    }
    //    if (!bfs::is_empty(directory)) {
    //      util::throw_invalid_argument("Is not empty", directory);
    //    }
  }

  ~StorageWriter() {
    close();
  }

  void close() {
    if (data_wfs_ == NULL)
      return;

    table_ofs_.close();
    util::fclose(data_wfs_);
    data_wfs_ = NULL;
    const bfs::path table_dir(directory_ / k_table_dir);
    if (!bfs::create_directory(table_dir)) {
      util::throw_invalid_argument("Cannot create", table_dir);
    }
    util::log("Building header table");
    bighashmap::Builder<Address>::Build(directory_, table_dir);
  }

  bool write(const std::string& key, Postlist<T>& postlist) {
    if (postlist.empty()) {
      return false;
    }
    if (key.empty()) {
      util::throw_invalid_argument("Cannot write empty key");
    }
    const size_t postlist_size(postlist.byte_size());
    if (data_wfs_ == NULL) {
      const bfs::path data_dir(directory_ / k_data_dir);
      if (!bfs::create_directory(data_dir)) {
        util::throw_invalid_argument("Cannot create", data_dir);
      }
      const std::string num(util::to_string(data_file_cnt_++));
      data_wfs_ = util::fopen(data_dir / (k_data_file + num), "wb");
      const bfs::path table_file(directory_ / k_table_file);
      table_ofs_.open(table_file);
      if (!table_ofs_) {
        util::throw_invalid_argument("Cannot create", table_file);
      }
    } else if (util::ftell(data_wfs_) + postlist_size > data_file_size_max) {
      util::fclose(data_wfs_);
      const bfs::path data_dir(directory_ / k_data_dir);
      const std::string num(util::to_string(data_file_cnt_++));
      data_wfs_ = util::fopen(data_dir / (k_data_file + num), "wb");
    }
    Record<Address> record(key,
                           Address(data_file_cnt_ - 1, util::ftell(data_wfs_)));
    postlist.write(data_wfs_);
    return (table_ofs_ << record << '\n').good();
  }

private:
  bfs::path directory_;
  bfs::ofstream table_ofs_;
  uint16_t data_file_cnt_;
  FILE* data_wfs_;
};

template <typename T>
const std::string StorageWriter<T>::k_data_dir("data");

template <typename T>
const std::string StorageWriter<T>::k_data_file("data.");

template <typename T>
const std::string StorageWriter<T>::k_table_dir("table");

template <typename T>
const std::string StorageWriter<T>::k_table_file("table.txt");

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_STORAGE_WRITER_HPP
