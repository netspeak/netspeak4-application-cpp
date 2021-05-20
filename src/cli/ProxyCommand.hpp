#ifndef CLI_PROXY_COMMAND_HPP
#define CLI_PROXY_COMMAND_HPP

#include <string>

#include "cli/Command.hpp"


namespace cli {

class ProxyCommand : public Command {
public:
  ~ProxyCommand() override {}
  std::string name() override {
    return "proxy";
  };
  std::string desc() override;
  void add_options(boost::program_options::options_description_easy_init&
                       easy_init) override;
  int run(boost::program_options::variables_map variables) override;
};

} // namespace cli

#endif
