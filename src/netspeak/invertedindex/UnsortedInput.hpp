// UnsortedInput.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_UNSORTED_INPUT_HPP
#define NETSPEAK_INVERTEDINDEX_UNSORTED_INPUT_HPP

#include <cstdio>
#include <memory>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include "netspeak/invertedindex/IndexStrategy.hpp"
#include "netspeak/invertedindex/PostlistBuilder.hpp"
#include "netspeak/invertedindex/PostlistSorter.hpp"
#include "netspeak/invertedindex/StorageWriter.hpp"
#include "netspeak/util/checksum.hpp"

namespace netspeak {
namespace invertedindex {

namespace bfs = boost::filesystem;

/**
 * This strategy is for building an index from
 * unsorted records, e.g. from a pseudo inverted file.
 */
template <typename T>
class UnsortedInput : public IndexStrategy<T> {
public:
  typedef typename IndexStrategy<T>::record_type record_type;
  typedef typename record_type::value_type value_type;

private:
  typedef std::shared_ptr<Postlist<value_type> > shared_postlist;
  typedef std::unordered_map<std::string, shared_postlist> postlist_map;
  typedef std::shared_ptr<PostlistBuilder<value_type> > shared_pbuilder;
  typedef std::unordered_map<std::string, shared_pbuilder> pbuilder_map;

public:
  UnsortedInput(const Configuration& config)
      : IndexStrategy<value_type>(config),
        max_bucket_size_(to_bytes(config.max_memory_usage()) / 12) // best avg.
  {
    if (max_bucket_size_ == 0)
      max_bucket_size_ = 10 * 1024 * 1024;
    bucket_dir_ = util::tmpdir(this->config().index_directory());
    bucket_fs_ = rehash(bfs::path(), bucket_dir_, 2);
    this->set_expected_record_count(config.expected_record_count());
  }

  virtual ~UnsortedInput() {
    bfs::remove_all(bucket_dir_);
  }

  virtual void index() {
    // group / sort / store records
    close(bucket_fs_);
    postlist_map postlists;
    StorageWriter<value_type> storage(this->config().index_directory());
    const bfs::directory_iterator end;
    for (bfs::directory_iterator it(bucket_dir_); it != end; ++it) {
      if (!bfs::is_regular_file(it->path())) {
        continue;
      }
      util::log("Processing", it->path());
      group(it->path(), postlists);
      switch (this->config().value_sorting()) {
        case value_sorting_type::ascending:
          sort<std::less<value_type> >(postlists);
          break;
        case value_sorting_type::descending:
          sort<std::greater<value_type> >(postlists);
          break;
        default:;
      }
      this->stats_.key_count += postlists.size();
      store(postlists, storage);
      bfs::remove(it->path());
      postlists.clear();
    }
    storage.close();
  }

private:
  static void close(std::vector<FILE*>& fs) {
    for (auto it(fs.begin()); it != fs.end(); ++it) {
      util::fclose(*it);
    }
  }

  static void group(const bfs::path& bucket_path, postlist_map& postlists) {
    assert(bfs::exists(bucket_path));

    record_type record;
    pbuilder_map builders;
    typename pbuilder_map::const_iterator it;
    FILE* rfs(util::fopen(bucket_path, "rb"));
    while (record.read(rfs)) {
      it = builders.find(record.key());
      if (it == builders.end()) {
        shared_pbuilder pb(new PostlistBuilder<value_type>);
        it = builders.insert(std::make_pair(record.key(), pb)).first;
      }
      it->second->push_back(record.value());
    }
    util::fclose(rfs);
    for (auto it = builders.begin(); it != builders.end(); ++it) {
      postlists.insert(
          std::make_pair(it->first, shared_postlist(it->second->build())));
    }
  }

  static const std::vector<FILE*> rehash(const bfs::path& src_dir,
                                         const bfs::path& des_dir,
                                         uint32_t num) {
    assert(bfs::is_empty(des_dir));
    // create destination bucket files
    std::vector<FILE*> des_fs(util::next_prime(num));
    for (unsigned i(0); i != des_fs.size(); ++i) {
      des_fs[i] = util::fopen(des_dir / ("bucket." + util::to_string(i)), "wb");
    }
    if (bfs::exists(src_dir)) {
      record_type record;
      const bfs::directory_iterator end;
      for (bfs::directory_iterator it(src_dir); it != end; ++it) {
        if (!bfs::is_regular_file(it->path()))
          continue;

        FILE* src_fs(util::fopen(it->path(), "rb"));
        while (record.read(src_fs)) {
          record.write(des_fs[util::hash32(record.key()) % des_fs.size()]);
        }
        util::fclose(src_fs);
        bfs::remove(it->path()); // early deletion to safe disk space
      }
    }
    return des_fs;
  }

  template <typename Comp>
  static void sort(postlist_map& postlists) {
    for (auto it(postlists.begin()); it != postlists.end(); ++it) {
      it->second = PostlistSorter<value_type, Comp>::sort(*it->second);
    }
  }

  static void store(postlist_map& postlists,
                    StorageWriter<value_type>& storage) {
    for (auto it(postlists.begin()); it != postlists.end(); ++it) {
      storage.write(it->first, *it->second);
    }
  }

  void insert_(const record_type& record) {
    if (!record.write(
            bucket_fs_[util::hash32(record.key()) % bucket_fs_.size()])) {
      util::throw_runtime_error("Cannot write record", record);
    }
    // do rehashing, if the maximal bucket size exceeds
    if (this->stats().total_size / bucket_fs_.size() > max_bucket_size_) {
      util::log("Bucket size exceeded", this->stats().value_count);
      if (this->config().expected_record_count() == 0) {
        rehash_(2 * bucket_fs_.size());
      } else {
        util::log("Expected total number of records",
                  this->config().expected_record_count());
        // estimate the final number of buckets
        // note: division by zero is not possible
        const double avg_value_size =
            this->stats().total_size /
            static_cast<double>(this->stats().value_count);
        const uint64_t expected_total_size =
            avg_value_size * this->config().expected_record_count();
        const size_t new_bucket_count =
            1 + expected_total_size / max_bucket_size_;
        rehash_(std::max(new_bucket_count, 2 * bucket_fs_.size()));
      }
    }
  }

  void rehash_(size_t new_bucket_count) {
    close(bucket_fs_);
    bfs::path new_dir(util::tmpdir(this->config().index_directory()));
    util::log("Rehashing current inserted records", this->stats().value_count);
    bucket_fs_ = rehash(bucket_dir_, new_dir, new_bucket_count);
    util::log("New number of bucket files", bucket_fs_.size());
    bfs::remove_all(bucket_dir_);
    bucket_dir_ = new_dir;
  }

  std::vector<FILE*> bucket_fs_;
  bfs::path bucket_dir_;
  uint64_t max_bucket_size_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_UNSORTED_INPUT_HPP
