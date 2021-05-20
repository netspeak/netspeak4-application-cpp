#ifndef CLI_COMMAND_HPP
#define CLI_COMMAND_HPP

#include <string>

#include "boost/program_options.hpp"


namespace cli {

class Command {
public:
  virtual ~Command() {}
  virtual std::string name() = 0;
  virtual std::string desc() = 0;
  virtual void add_options(
      boost::program_options::options_description_easy_init& easy_init) = 0;
  virtual int run(boost::program_options::variables_map variables) = 0;
};

} // namespace cli

#endif
