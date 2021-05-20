#ifndef NETSPEAK_DICTIONARIES_HPP
#define NETSPEAK_DICTIONARIES_HPP

#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>

namespace netspeak {

/**
 * A class to represent a dictionary. A dictionary is a mapping from keys to
 * a list of values. Both keys and values have the type std::string. The text
 * file format of a dictionary is defined as follows:
 *
 * line = key TAB value0 TAB value1 TAB value2 ...
 *
 * Whereas whitespaces within keys and/or values are allowed.
 */
class Dictionaries {
public:
  typedef std::unordered_multimap<std::string, std::string> Map;

  static const Map read_from_file(const boost::filesystem::path& csv);
};

} // namespace netspeak

#endif // NETSPEAK_DICTIONARIES_HPP
