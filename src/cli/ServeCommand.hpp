#ifndef CLI_SERVE_COMMAND_HPP
#define CLI_SERVE_COMMAND_HPP

#include <string>

#include "cli/Command.hpp"


namespace cli {

class ServeCommand : public Command {
public:
  ~ServeCommand() override {}
  std::string name() override {
    return "serve";
  };
  std::string desc() override {
    return "Create a new gRPC server for a Netspeak index.";
  };
  void add_options(boost::program_options::options_description_easy_init&
                       easy_init) override;
  int run(boost::program_options::variables_map variables) override;
};

} // namespace cli

#endif