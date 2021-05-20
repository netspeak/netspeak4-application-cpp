#ifndef CLI_REGEX_COMMAND_HPP
#define CLI_REGEX_COMMAND_HPP

#include <string>

#include "cli/Command.hpp"


namespace cli {

class RegexCommand : public Command {
public:
  ~RegexCommand() override {}
  std::string name() override {
    return "regex";
  };
  std::string desc() override;
  void add_options(boost::program_options::options_description_easy_init&
                       easy_init) override;
  int run(boost::program_options::variables_map variables) override;
};

} // namespace cli

#endif
