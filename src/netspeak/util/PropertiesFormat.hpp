#ifndef NETSPEAK_UTIL_PROPERTIES_FROMAT_HPP
#define NETSPEAK_UTIL_PROPERTIES_FROMAT_HPP

#include <istream>
#include <ostream>

#include "netspeak/util/traceable_error.hpp"


namespace netspeak {
namespace util {

// C++ is a very mature language, so of course it's not possible to move the
// implementation of a template function into a .cpp file.
// https://stackoverflow.com/questions/115703/storing-c-template-function-definitions-in-a-cpp-file

namespace __properties_format_impl {

bool has_next_key(std::istream& in);
std::string read_key(std::istream& in);
std::string read_value(std::istream& in);
void consume_delimiter(std::istream& in);

void write_key(std::ostream& out, const std::string& key);
void write_value(std::ostream& out, const std::string& value);

} // namespace __properties_format_impl

struct format_properties_error : public tracable_runtime_error {
  format_properties_error(const std::string& what)
      : tracable_runtime_error(what) {}
  virtual ~format_properties_error() throw() override {}
};

template <typename T>
void read_properties(std::istream& in, T inserter) {
  while (__properties_format_impl::has_next_key(in)) {
    std::string key = __properties_format_impl::read_key(in);
    __properties_format_impl::consume_delimiter(in);
    std::string value = __properties_format_impl::read_value(in);
    *inserter = std::make_pair(key, value);
    inserter++;
  }
}

template <typename T>
void write_properties(std::ostream& out, T begin, T end) {
  for (; begin != end; begin++) {
    const std::string& key = begin->first;
    const std::string& value = begin->second;
    __properties_format_impl::write_key(out, key);
    out << '=';
    __properties_format_impl::write_value(out, value);
    out << '\n';
  }
}


} // namespace util
} // namespace netspeak


#endif
