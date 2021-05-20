#ifndef NETSPEAK_UTIL_FILE_DESCRIPTOR_HPP
#define NETSPEAK_UTIL_FILE_DESCRIPTOR_HPP

#include <memory>


namespace netspeak {
namespace util {

/**
 * @brief A managed file descriptor. This is implemented using a shared pointer.
 */
struct FileDescriptor final {
private:
  struct _fd {
    int fd;
    _fd() = delete;
    _fd(const _fd&) = delete;
    _fd(int fd);
    ~_fd();
  };
  std::shared_ptr<_fd> fd_;

public:
  FileDescriptor() = delete;
  FileDescriptor(int fd);

  operator int() const;
};


} // namespace util
} // namespace netspeak


#endif
