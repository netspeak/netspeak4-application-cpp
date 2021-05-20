// CmphMap.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_BIGHASHMAP_CMPH_MAP_HPP
#define NETSPEAK_BIGHASHMAP_CMPH_MAP_HPP

#include <cmph.h>

#include <array>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

#include "netspeak/util/exception.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace bighashmap {

enum class Algorithm {
  BMZ = CMPH_BMZ,
  CHM = CMPH_CHM,
  BDZ = CMPH_BDZ,
  CHD = CMPH_CHD,
  None = CMPH_COUNT
};

/**
 * @author  martin.trenkmann@uni-weimar.de
 * @version $Id$
 */
template <typename ValueT, typename Traits>
class CmphMap {
public:
  typedef ValueT Value;
  typedef uint32_t Checksum;

  CmphMap(const CmphMap&) = delete;
  CmphMap& operator=(const CmphMap&) = delete;

  explicit CmphMap(const boost::filesystem::path& mph_file)
      : algo_(Algorithm::None), mphf_(NULL) {
    FILE* fd = util::fopen(mph_file, "rb");
    algo_ = ExtractAlgorithm(fd);
    util::rewind(fd);
    mphf_ = cmph_load(fd);
    if (mphf_ == NULL) {
      util::throw_runtime_error("Cannot load MPHF from", mph_file);
    }
    util::fclose(fd);
  }

  virtual ~CmphMap() {
    cmph_destroy(mphf_);
  }

  virtual bool Get(const std::string& key, Value& value) = 0;

  uint32_t size() const {
    return cmph_size(mphf_);
  }

  Algorithm algorithm() const {
    return algo_;
  }

protected:
  uint32_t Hash(const std::string& key) {
    // Important: For unknown keys the returned hash value might be out of
    // the perfect range [0, size). Maybe a bug? Modulo operation applied.
    // After inspecting the CMPH code cmph_search is assumed to be thread-safe.
    return cmph_search(mphf_, key.c_str(), key.size()) % size();
  }

private:
  static Algorithm ExtractAlgorithm(FILE* fd) {
    std::string algorithm(3, '\0');
    util::fread(const_cast<char*>(algorithm.data()), sizeof(char),
                algorithm.size(), fd);
    if (algorithm == "bdz") {
      return Algorithm::BDZ;
    }
    if (algorithm == "bmz") {
      return Algorithm::BMZ;
    }
    if (algorithm == "chd") {
      return Algorithm::CHD;
    }
    if (algorithm == "chm") {
      return Algorithm::CHM;
    }

    return Algorithm::None;
  }

  Algorithm algo_;
  cmph_t* mphf_;
};

} // namespace bighashmap
} // namespace netspeak

#endif // NETSPEAK_BIGHASHMAP_CMPH_MAP_HPP
