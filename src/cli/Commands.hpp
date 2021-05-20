#ifndef CLI_COMMANDS_HPP
#define CLI_COMMANDS_HPP

#include <memory>
#include <vector>

#include "cli/Command.hpp"


namespace cli {

class Commands {
private:
  std::vector<std::unique_ptr<Command>> commands_;

public:
  void add_command(std::unique_ptr<Command> command) {
    commands_.push_back(std::move(command));
  }

  int run(int argc, char* argv[]);
};

} // namespace cli

#endif
