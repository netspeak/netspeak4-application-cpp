// ByteBuffer.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_BYTE_BUFFER_HPP
#define NETSPEAK_INVERTEDINDEX_BYTE_BUFFER_HPP

#include <cstdlib>
#include <cstring>
#include <memory>

#include "netspeak/util/exception.hpp"
#include "netspeak/util/systemio.hpp"
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
 * A class to represent a raw byte buffer to store primitives and strings.
 * Note that the class' assignment operator returns a shallow copy of its
 * backed array which is managed by a smart pointer. Use the clone() method
 * to get a new deep copy of the data.
 */
class ByteBuffer {
private:
  struct buffer_type {
    buffer_type(size_t initial_size);
    void resize(size_t new_size);
    std::unique_ptr<char[]> data;
    size_t size;
  };

public:
  /**
   * The explicit constructor.
   * The byte buffer has a initial size of <code>size</code>.
   */
  ByteBuffer(size_t size = 0);

  /**
   * The copy constructor for shallow copying.
   * @param buffer
   */
  ByteBuffer(const ByteBuffer& buffer);

  /**
   * The destructor.
   */
  ~ByteBuffer() {}

private:
  inline size_t bytes_available() const {
    return buffer_->size - (bufpos_ - buffer_->data.get());
  }

public: // methods
  char* begin() {
    return buffer_->data.get();
  }
  const char* begin() const {
    return buffer_->data.get();
  }

  char* end() {
    return buffer_->data.get() + buffer_->size;
  }
  const char* end() const {
    return buffer_->data.get() + buffer_->size;
  }

  void clear();

  /**
   * Creates a deep copy of that buffer.
   */
  ByteBuffer clone() const;

  /**
   * The assignment operator for shallow copying.
   * The internal position will be set to the beginning.
   * @param buffer
   */
  ByteBuffer& operator=(const ByteBuffer& buffer);

  template <typename T>
  bool get(T& value) {
    typedef value::value_traits<T> traits_type;
    if (!bufpos_ ||
        bufpos_ + sizeof(value) > buffer_->data.get() + buffer_->size)
      return false;
    const char* new_bufpos(traits_type::copy_from(value, bufpos_));
    bufpos_ = const_cast<char*>(new_bufpos);
    return true;
  }

  template <typename T>
  bool put(const T& value) {
    typedef value::value_traits<T> traits_type;
    if (traits_type::size_of(value) > bytes_available())
      return false;
    bufpos_ = traits_type::copy_to(value, bufpos_);
    return true;
  }

  void seek(size_t offset);
  size_t tell() const;
  void rewind();
  void read(FILE* fs);
  void write(FILE* fs) const;

  /**
   * Allocates <tt>size</tt> bytes of memory.
   * @param size the new size of the byte buffer.
   */
  void resize(size_t size);

  /**
   * Get the size of the byte array.
   * @return the number of bytes stored.
   */
  size_t size() const {
    return buffer_->size;
  }

  const char* position() const {
    return bufpos_;
  }

private:
  std::shared_ptr<buffer_type> buffer_;
  char* bufpos_;
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_BYTE_BUFFER_HPP
