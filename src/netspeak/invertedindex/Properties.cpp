#include "netspeak/invertedindex/Properties.hpp"

#include "netspeak/util/systemio.hpp"

namespace netspeak {
namespace invertedindex {

Properties::Properties()
    : value_sorting(value_sorting_type::disabled),
      version_number(k_version_number),
      value_count(),
      total_size(),
      key_count() {
  std::memset(value_type, 0, sizeof(value_type));
}

Properties::Properties(const Properties& rhs)
    : value_sorting(rhs.value_sorting),
      version_number(rhs.version_number),
      value_count(rhs.value_count),
      total_size(rhs.total_size),
      key_count(rhs.key_count) {
  std::strncpy(value_type, rhs.value_type, sizeof(value_type));
}

const std::string Properties::default_filename() {
  return "properties"; // alternative: static class constant
}

void Properties::print(std::ostream& os) const {
  os << "{\n  value_type : " << value_type
     << ",\n  value_sorting : " << util::to_string(value_sorting)
     << ",\n  version_number : " << version_number
     << ",\n  total_size : " << total_size
     << ",\n  value_count : " << value_count << ",\n  key_count : " << key_count
     << "\n}";
}

void Properties::read(const boost::filesystem::path& path) {
  FILE* rfs(util::fopen(path, "rb"));
  util::fread(this, sizeof(Properties), 1, rfs);
  util::fclose(rfs);
}

void Properties::write(const boost::filesystem::path& path) {
  FILE* wfs(util::fopen(path, "wb"));
  util::fwrite(this, sizeof(Properties), 1, wfs);
  util::fclose(wfs);
}

Properties& Properties::operator=(const Properties& rhs) {
  std::strncpy(value_type, rhs.value_type, sizeof(value_type));
  value_sorting = rhs.value_sorting;
  version_number = rhs.version_number;
  value_count = rhs.value_count;
  total_size = rhs.total_size;
  key_count = rhs.key_count;
  return *this;
}

std::ostream& operator<<(std::ostream& os, const Properties& properties) {
  if (os)
    properties.print(os);
  return os;
}

} // namespace invertedindex
} // namespace netspeak
