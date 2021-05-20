#include "netspeak/Dictionaries.hpp"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>

#include "netspeak/error.hpp"
#include "netspeak/util/check.hpp"

namespace netspeak {

const Dictionaries::Map Dictionaries::read_from_file(
    const boost::filesystem::path& csv) {
  boost::filesystem::ifstream ifs(csv);
  util::check(ifs.is_open(), error_message::cannot_open, csv);
  Map dict;
  std::string line;
  std::vector<std::string> tokens;
  const auto predicate = std::bind2nd(std::equal_to<char>(), '\t');
  while (std::getline(ifs, line)) {
    boost::split(tokens, line, predicate);
    if (tokens.size() < 2)
      continue;
    for (unsigned i = 1; i != tokens.size(); ++i) {
      dict.insert(std::make_pair(tokens.front(), tokens[i]));
    }
  }
  return dict;
}

} // namespace netspeak
