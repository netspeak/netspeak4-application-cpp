// InvertedFileWriter.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_INVERTED_FILE_WRITER_HPP
#define NETSPEAK_INVERTEDINDEX_INVERTED_FILE_WRITER_HPP

#include <ostream>

#include "netspeak/invertedindex/Record.hpp"
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

/**
 * A class to write records in the inverted file format to an output stream.
 */
template <typename T>
class InvertedFileWriter {
public:
  typedef Record<T> RecordType;

private:
  typedef value::value_traits<typename RecordType::key_type> key_traits;
  typedef value::value_traits<typename RecordType::value_type> value_traits;

public:
  static const char value_separator = '\t';

  InvertedFileWriter(std::ostream& os) : os_(os) {}

  ~InvertedFileWriter() = default;

  bool write(const RecordType& record) {
    if (key_ != record.key()) {
      if (os_.tellp() != 0)
        os_ << '\n';
      key_traits::print_to(record.key(), os_);
      key_ = record.key();
    }
    os_ << value_separator;
    value_traits::print_to(record.value(), os_);
    return static_cast<bool>(os_);
  }

  std::size_t tell() {
    return os_.tellp();
  }

private:
  std::ostream& os_;
  std::string key_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_INVERTED_FILE_WRITER_HPP
