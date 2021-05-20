#include "netspeak/invertedindex/ByteBuffer.hpp"

namespace netspeak {
namespace invertedindex {

ByteBuffer::buffer_type::buffer_type(size_t initial_size)
    : data(new char[initial_size]), size(initial_size) {
  if (!data)
    throw std::bad_alloc();
}
void ByteBuffer::buffer_type::resize(size_t new_size) {
  if (new_size != size && new_size != 0) {
    data.reset(new char[new_size]);
    if (!data)
      throw std::bad_alloc();
  }
  size = new_size;
}

ByteBuffer::ByteBuffer(size_t size)
    : buffer_(new buffer_type(size)), bufpos_(buffer_->data.get()) {}

/**
 * The copy constructor for shallow copying.
 * @param buffer
 */
ByteBuffer::ByteBuffer(const ByteBuffer& buffer)
    : buffer_(buffer.buffer_), bufpos_(buffer_->data.get()) {}


void ByteBuffer::clear() {
  // does not free memory
  buffer_->size = 0;
  rewind();
}

ByteBuffer ByteBuffer::clone() const {
  ByteBuffer deep_copy(size());
  std::memcpy(deep_copy.buffer_->data.get(), begin(), size());
  return deep_copy;
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer& buffer) {
  buffer_ = buffer.buffer_;
  bufpos_ = buffer_->data.get();
  return *this;
}

void ByteBuffer::seek(size_t offset) {
  if (offset > size()) {
    util::throw_out_of_range("ByteBuffer::seek failed", offset);
  }
  bufpos_ = buffer_->data.get() + offset;
}

size_t ByteBuffer::tell() const {
  return bufpos_ - begin();
}

void ByteBuffer::rewind() {
  bufpos_ = buffer_->data.get();
}

void ByteBuffer::read(FILE* fs) {
  util::fread(buffer_->data.get(), 1, size(), fs);
  rewind();
}

void ByteBuffer::write(FILE* fs) const {
  util::fwrite(begin(), 1, size(), fs);
}

void ByteBuffer::resize(size_t size) {
  buffer_->resize(size);
  rewind();
}


} // namespace invertedindex
} // namespace netspeak
