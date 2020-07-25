#ifndef CLI_BUILD_COMMAND_HPP
#define CLI_BUILD_COMMAND_HPP

#include <string>

#include "cli/Command.hpp"


namespace cli {

class BuildCommand : public Command {
public:
  ~BuildCommand() override {}
  std::string name() override {
    return "build";
  };
  std::string desc() override {
    return "Builds a new `netspeak` from n-grams.\n"
           "If your n-gram collection is not unique, you have to merge. When "
           "merging the directory `netspeak-merged` will be added to `from` "
           "directories parent folder. Use `-merge-dir` to override this "
           "behaviour";
  };
  void add_options(boost::program_options::options_description_easy_init&
                       easy_init) override;
  int run(boost::program_options::variables_map variables) override;
};

} // namespace cli

#endif