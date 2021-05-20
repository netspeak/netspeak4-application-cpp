#ifndef CLI_STRESS_COMMAND_HPP
#define CLI_STRESS_COMMAND_HPP

#include <string>

#include "cli/Command.hpp"


namespace cli {

class StressCommand : public Command {
public:
  ~StressCommand() override {}
  std::string name() override {
    return "stress";
  };
  std::string desc() override;
  void add_options(boost::program_options::options_description_easy_init&
                       easy_init) override;
  int run(boost::program_options::variables_map variables) override;
};

} // namespace cli

#endif
