// BigHashMap.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_BIGHASHMAP_BIG_HASH_MAP_HPP
#define NETSPEAK_BIGHASHMAP_BIG_HASH_MAP_HPP

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "netspeak/bighashmap/Builder.hpp"
#include "netspeak/bighashmap/CmphMap.hpp"
#include "netspeak/bighashmap/ExternalMap.hpp"
#include "netspeak/bighashmap/InternalMap.hpp"
#include "netspeak/util/memory.hpp"
#include "netspeak/value/pair_traits.hpp"
#include "netspeak/value/quadruple_traits.hpp"
#include "netspeak/value/quintuple_traits.hpp"
#include "netspeak/value/sextuple_traits.hpp"
#include "netspeak/value/triple_traits.hpp"

namespace netspeak {
namespace bighashmap {

/**
 * For comments please take a look at {@link ExternalMap}.
 * By default read access to the hash table is not thread-safe.
 *
 * @author  martin.trenkmann@uni-weimar.de
 * @version $Id$
 */
template <typename ValueT, bool IsThreadSafe = false,
          typename TraitsT = value::value_traits<ValueT> >
class BigHashMap {
public:
  typedef ValueT Value;
  typedef TraitsT Traits;

  BigHashMap() = delete;

  virtual ~BigHashMap() {
    for (const auto& map : maps_) {
      delete map;
    }
  }

private:
  typedef CmphMap<ValueT, Traits> Map;

  static const std::string index_file_name;

  explicit BigHashMap(const std::vector<Map*>& sub_maps)
      : maps_(sub_maps), size_(0) {
    for (const auto& map : maps_) {
      size_ += map->size();
    }
  }

  static uint64_t SerializedSizeInBytes(const bfs::path& dir) {
    uint64_t size = 0;
    if (bfs::exists(dir)) {
      const bfs::directory_iterator end;
      for (bfs::directory_iterator it(dir); it != end; ++it) {
        if (!bfs::is_regular_file(it->path()))
          continue;
        if (boost::ends_with(it->path().string(), "dat") ||
            boost::ends_with(it->path().string(), "mph")) {
          size += bfs::file_size(it->path());
        }
      }
    }
    return size;
  }

public:
  static void Build(const bfs::path& input_dir, const bfs::path& output_dir,
                    Algorithm algorithm = Algorithm::BDZ) {
    // Set current locale to C to enable byte-wise UTF-8 string comparison.
    // http://stackoverflow.com/questions/20226851/how-do-locales-work-in-linux-posix-and-what-transformations-are-applied/20231523
    std::locale::global(std::locale("C"));
    bighashmap::Builder<Value>::Build(input_dir, output_dir, algorithm);
  }

  static BigHashMap* Open(
      const bfs::path& dir,
      util::memory_type memory = util::memory_type::min_required) {
    const bfs::path idx_file = dir / index_file_name;
    bfs::ifstream ifs(idx_file);
    if (!ifs) {
      util::throw_runtime_error("Cannot open", idx_file);
    }
    std::string part_idx_file;
    std::vector<Map*> part_maps;
    const uint64_t num_bytes = SerializedSizeInBytes(idx_file.parent_path());
    if (num_bytes > util::to_bytes(memory)) {
      while (std::getline(ifs, part_idx_file)) {
        part_maps.push_back(ExternalMap<ValueT, IsThreadSafe, Traits>::Open(
            idx_file.parent_path() / part_idx_file));
      }
    } else {
      while (std::getline(ifs, part_idx_file)) {
        part_maps.push_back(InternalMap<ValueT, Traits>::Open(
            idx_file.parent_path() / part_idx_file));
      }
    }
    ifs.close();
    return new BigHashMap(part_maps);
  }

  bool Get(const std::string& key, Value& value) {
    return maps_.empty()
               ? false
               : maps_[util::hash32(key) % maps_.size()]->Get(key, value);
  }

  bool Get(const std::string& key, char* buffer) {
    Value value;
    if (Get(key, value)) {
      Traits::copy_to(value, buffer);
      return true;
    }
    return false;
  }

  uint64_t size() const {
    return size_;
  }

  Algorithm algorithm() const {
    return maps_.empty() ? Algorithm::None : maps_.front()->algorithm();
  }

private:
  std::vector<Map*> maps_;
  uint64_t size_;
};

template <typename Value, bool IsThreadSafe, typename Traits>
const std::string BigHashMap<Value, IsThreadSafe, Traits>::index_file_name(
    bighashmap::Builder<Value>::index_file_name);

} // namespace bighashmap
} // namespace netspeak

#endif // NETSPEAK_BIGHASHMAP_BIG_HASH_MAP_HPP
