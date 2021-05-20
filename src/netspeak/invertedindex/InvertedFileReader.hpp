// InvertedFileReader.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_INVERTED_FILE_READER_HPP
#define NETSPEAK_INVERTEDINDEX_INVERTED_FILE_READER_HPP

#include <istream>
#include <string>

#include "netspeak/invertedindex/RecordReader.hpp"
#include "netspeak/util/exception.hpp"
#include "netspeak/value/big_string_traits.hpp"
#include "netspeak/value/pair_traits.hpp"
#include "netspeak/value/quadruple_traits.hpp"
#include "netspeak/value/quintuple_traits.hpp"
#include "netspeak/value/sextuple_traits.hpp"
#include "netspeak/value/string_traits.hpp"
#include "netspeak/value/triple_traits.hpp"

namespace netspeak {
namespace invertedindex {

template <typename T>
class InvertedFileReader : public RecordReader<T> {
private:
  typedef RecordReader<T> base_type;
  typedef typename base_type::record_type record_type;
  typedef value::value_traits<typename record_type::key_type> key_traits;
  typedef value::value_traits<typename record_type::value_type> value_traits;

public:
  InvertedFileReader(std::istream& is) : base_type(), is_(is), num_lines_(1) {
    is_.peek();                        // Sets eofbit if no data was available.
    key_traits::parse_from(key_, is_); // Read first key or invalidate stream.
  }

  virtual ~InvertedFileReader() {}

  virtual bool read(record_type& record) {
    try {
      while (is_.good()) {
        switch (is_.peek()) {
          case '\r':
          case std::istream::traits_type::eof():
            is_.get();
            break;
          case '\n':
            is_.get();
            ++num_lines_;
            key_traits::parse_from(key_, is_);
            if (is_.eof())
              break;
          // Otherwise fall through here.
          default:
            record.key().assign(key_);
            value_traits::parse_from(record.value(), is_);
            return true;
        }
      }
    } catch (std::exception& e) {
      util::throw_runtime_error(std::string(e.what()) +
                                " Line: " + std::to_string(num_lines_));
    }
    return false;
  }

  virtual std::size_t tell() {
    return is_.tellg();
  }

private:
  std::istream& is_;
  std::string key_;
  std::size_t num_lines_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_INVERTED_FILE_READER_HPP
