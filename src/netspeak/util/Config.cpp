#include "netspeak/util/Config.hpp"

#include <fstream>

#include "netspeak/util/PropertiesFormat.hpp"
#include "netspeak/util/traceable_error.hpp"

namespace netspeak {
namespace util {


Config::Config(boost::filesystem::path file_name)
    : data_(), file_name_(file_name) {
  std::ifstream ifs(file_name.string());
  if (!ifs.good()) {
    throw tracable_runtime_error("Could not open " + file_name.string());
  }
  merge_properties(ifs);
}


const std::string& Config::get(const std::string& key,
                               const std::string& default_value) const {
  if (contains(key)) {
    return data_.at(key);
  } else {
    return default_value;
  }
}
const std::string& Config::get(const std::string& key) const {
  if (contains(key)) {
    return data_.at(key);
  } else {
    if (file_name_.empty()) {
      throw tracable_runtime_error("Incomplete config. \"" + key +
                                   "\" is missing.");
    } else {
      throw tracable_runtime_error("Incomplete config. \"" + key +
                                   "\" is missing in \"" + file_name_.string() +
                                   "\".");
    }
  }
}


void Config::merge_properties(std::istream& in) {
  util::read_properties(in, std::inserter(data_, data_.end()));
}

std::ostream& Config::operator<<(std::ostream& out) const {
  util::write_properties(out, begin(), end());
  return out;
}

} // namespace util
} // namespace netspeak
