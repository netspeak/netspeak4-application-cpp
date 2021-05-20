#include "cli/Commands.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "boost/program_options.hpp"

#include "cli/util.hpp"

namespace cli {

namespace bpo = boost::program_options;

void print_line(std::ostream& out, std::string line,
                bool skip_first_indentation, size_t indentation,
                size_t max_width) {
  std::string ident(indentation, ' ');
  size_t available_space = max_width - indentation;

  if (line.empty()) {
    out << '\n';
    return;
  }

  while (!line.empty()) {
    if (skip_first_indentation) {
      skip_first_indentation = false;
    } else {
      out << ident;
    }

    if (line.size() <= available_space) {
      // line fits
      out << line;
      line.clear();
    } else {
      // we have to split the line
      size_t space_pos = line.find(' ');
      if (space_pos == std::string::npos || space_pos > available_space) {
        // split the word.
        out << line.substr(0, available_space);
        line = line.substr(available_space);
      } else {
        // try to fit in as many words as possible
        while (true) {
          size_t next = line.find(' ', space_pos + 1);
          if (next == std::string::npos || next > available_space) {
            break;
          } else {
            space_pos = next;
          }
        }
        // split
        out << line.substr(0, space_pos);
        line = line.substr(space_pos + 1);
      }
    }

    out << '\n';
  }
}
void print_text(std::ostream& out, const std::string& text,
                size_t indentation = 0, size_t max_width = 100) {
  std::stringstream str(text);

  std::string line;
  bool first = true;
  while (std::getline(str, line, '\n')) {
    print_line(out, line, first, indentation, max_width);
    if (first) {
      first = false;
    }
  }
}

std::string get_first_line(const std::string& text) {
  size_t pos = text.find('\n');
  if (pos == std::string::npos) {
    return text;
  } else {
    return text.substr(0, pos);
  }
}

bpo::options_description get_option_desc(Command& command) {
  bpo::options_description desc(command.desc());

  auto easy_init = desc.add_options();
  command.add_options(easy_init);

  return desc;
}

std::string get_option_identifier(const bpo::option_description& option) {
  std::string id = option.format_name();
  auto arg = option.format_parameter();
  if (!arg.empty()) {
    id.push_back(' ');
    id.append(arg);
  }

  auto semantic = option.semantic();
  if (semantic->is_required()) {
    id.append(" (req)");
  }

  return id;
}
void print_command_help(Command& command, bool with_name) {
  if (with_name) {
    std::cout << FG_GREEN << "netspeak4 " << command.name() << " ..." << RESET
              << "\n"
              << "\n";
  }

  // print description
  print_text(std::cout, command.desc());

  // print options
  auto option_desc = get_option_desc(command);

  std::cout << "\n";

  if (option_desc.options().empty()) {
    std::cout << "No arguments.\n";
  } else {
    std::cout << "Arguments:\n";

    size_t max_name_len = 0;
    for (const auto& option_ptr : option_desc.options()) {
      if (option_ptr) {
        auto name = get_option_identifier(*option_ptr);
        max_name_len = std::max(max_name_len, name.size());
      }
    }
    for (const auto& option_ptr : option_desc.options()) {
      if (option_ptr) {
        std::cout << "\n";
        auto name = get_option_identifier(*option_ptr);
        auto desc = option_ptr->description();
        auto padding = max_name_len - name.size();
        std::cout << "  " << name << std::string(padding, ' ') << "  ";
        print_text(std::cout, desc, 2 + max_name_len + 2);
      }
    }
  }
}

/**
 * @brief Print a short help message that lists all available commands.
 *
 * @param commands
 */
void print_commands_help(const std::vector<std::unique_ptr<Command>>& commands,
                         bool full) {
  size_t max_name_len = 0;
  for (const auto& command : commands) {
    max_name_len = std::max(max_name_len, command->name().size());
  }

  std::cout << "usage: netspeak4 <command> [<args>]\n"
            << "where <command> is one of:\n"
            << "\n";

  for (const auto& command : commands) {
    const auto& name = command->name();
    const auto& desc = command->desc();
    auto padding = max_name_len - name.size();
    std::cout << "  " << name << std::string(padding, ' ') << "  ";
    // only print the first line
    print_text(std::cout, get_first_line(desc), 2 + max_name_len + 2);
  }

  std::cout << "\n"
            << "Run `netspeak4 <command> --help` to view command-specific "
               "usage information.\n";

  if (full) {
    for (auto& command : commands) {
      std::cout << "\n";
      std::cout << FG_BRIGHT_BLACK << std::string(40, '-') << RESET << "\n";
      print_command_help(*command, true);
    }
  }
}

void print_error(const std::vector<std::unique_ptr<Command>>& commands,
                 const std::string& message) {
  std::cerr << "Error: " << message << "\n"
            << "\n";
  print_commands_help(commands, false);
}
void print_error(Command& command, const std::string& message) {
  std::cerr << "Error: " << message << "\n"
            << "\n";
  print_command_help(command, true);
}

int run_command(Command& command, const std::vector<std::string>& opts) {
  try {
    bpo::variables_map variables;

    try {
      auto options_desc = get_option_desc(command);

      auto parsed_options =
          bpo::command_line_parser(opts).options(options_desc).run();
      bpo::store(parsed_options, variables);

      bpo::notify(variables);
    } catch (std::exception& e) {
      print_error(command, e.what());
      return EXIT_FAILURE;
    }

    return command.run(std::move(variables));
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int Commands::run(int argc, char* argv[]) {
  if (argc < 2) {
    print_error(commands_, "Not enough arguments.");
    return EXIT_FAILURE;
  }

  try {
    bpo::options_description global("");
    auto global_init = global.add_options();
    global_init("command", bpo::value<std::string>(), "command to execute");
    global_init("help,h", "Print this help message.");
    global_init("subargs", bpo::value<std::vector<std::string>>(),
                "Arguments for command");

    bpo::positional_options_description pos_opt_desc;
    pos_opt_desc.add("command", 1).add("subargs", -1);

    auto parsed_options = bpo::command_line_parser(argc, argv)
                              .options(global)
                              .positional(pos_opt_desc)
                              .allow_unregistered()
                              .run();

    bpo::variables_map variables;
    bpo::store(parsed_options, variables);

    if (variables.count("command") == 0) {
      if (variables.count("help") != 0) {
        print_commands_help(commands_, true);
        return EXIT_SUCCESS;
      } else {
        throw std::runtime_error("Missing command.");
      }
    }
    std::string command_name = variables["command"].as<std::string>();

    for (const auto& command_ptr : commands_) {
      auto& command = *command_ptr;
      if (command.name() == command_name) {
        // might print help
        if (variables.count("help") != 0) {
          print_command_help(command, false);
          return EXIT_SUCCESS;
        }

        // collect all arguments
        std::vector<std::string> opts = bpo::collect_unrecognized(
            parsed_options.options, bpo::include_positional);
        if (!opts.empty()) {
          opts.erase(opts.begin());
        }

        try {
          return run_command(command, opts);
        } catch (std::exception& e) {
          std::cerr << "Error: " << e.what() << "\n";
          return EXIT_FAILURE;
        }
      }
    }

    print_error(commands_, "Unknown command \"" + command_name + "\".");
    return EXIT_FAILURE;
  } catch (std::exception& e) {
    print_error(commands_, e.what());
    return EXIT_FAILURE;
  }
}

} // namespace cli
