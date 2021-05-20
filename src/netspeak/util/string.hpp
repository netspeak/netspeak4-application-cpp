#ifndef NETSPEAK_UTIL_STRING_HPP
#define NETSPEAK_UTIL_STRING_HPP

#include <string>

namespace netspeak {
namespace util {

bool is_valid_utf8(const std::string& str);

void check_valid_utf8(const std::string& str);
void check_valid_utf8(const std::string& str, const std::string& message);

} // namespace util
} // namespace netspeak


#endif
