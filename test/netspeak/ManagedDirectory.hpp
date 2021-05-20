#ifndef TEST_NETSPEAK_MANAGED_DIRECTORY
#define TEST_NETSPEAK_MANAGED_DIRECTORY

#include <string>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace test {

/**
 * @brief Each instance of this class owns a direcotry in the file system.
 *
 * The directory and all its contents will be removed when a instance goes
 * out of scope.
 */
class ManagedDirectory {
private:
  const boost::filesystem::path path_;

public:
  ManagedDirectory() = delete;
  ManagedDirectory(const ManagedDirectory&) = delete;
  ManagedDirectory(const std::string& path_str) : path_(path_str) {
    BOOST_REQUIRE(boost::filesystem::create_directory(path_));
  }
  ~ManagedDirectory() {
    boost::filesystem::remove_all(path_);
  }

  const boost::filesystem::path& dir() const {
    return path_;
  }
};

} // namespace test

#endif
