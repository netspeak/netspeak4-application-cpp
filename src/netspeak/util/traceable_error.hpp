#ifndef NETSPEAK_UTIL_TRACEABLE_ERROR_HPP
#define NETSPEAK_UTIL_TRACEABLE_ERROR_HPP

#include <sstream>
#include <stdexcept>
#include <string>

#ifndef NDEBUG
#include <boost/stacktrace.hpp>
#endif


namespace netspeak {
namespace util {

inline std::string add_trace(const std::string& what) {
#ifndef NDEBUG
  // add stack trace in debugging mode
  std::stringstream out;
  out << what << "\nAt:\n" << boost::stacktrace::stacktrace();
  return out.str();
#else
  // just keep the error message as is
  return what;
#endif
}

struct tracable_logic_error : public std::logic_error {
  inline tracable_logic_error(const std::string& what)
      : std::logic_error(add_trace(what)) {}
  virtual ~tracable_logic_error() throw() override {}
};
struct tracable_runtime_error : public std::runtime_error {
  inline tracable_runtime_error(const std::string& what)
      : std::runtime_error(add_trace(what)) {}
  virtual ~tracable_runtime_error() throw() override {}
};

} // namespace util
} // namespace netspeak


#endif
