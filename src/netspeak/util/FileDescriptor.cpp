#include "netspeak/util/FileDescriptor.hpp"

#include <unistd.h>


namespace netspeak {
namespace util {

FileDescriptor::_fd::_fd(int fd) : fd(fd) {}
FileDescriptor::_fd::~_fd() {
  ::close(fd);
}

FileDescriptor::FileDescriptor(int fd) : fd_() {
  if (fd == -1) {
    throw std::runtime_error("Invalid file descriptor.");
  }
  fd_ = std::make_shared<_fd>(fd);
}

FileDescriptor::operator int() const {
  return fd_->fd;
}


} // namespace util
} // namespace netspeak
