#include "cli/RegexCommand.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

#include "boost/filesystem.hpp"

#include "netspeak/error.hpp"
#include "netspeak/regex/DefaultRegexIndex.hpp"
#include "netspeak/regex/parsers.hpp"
#include "netspeak/util/check.hpp"

namespace cli {

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

std::string RegexCommand::desc() {
  return "Load an interactive shell for a regex word list.\n"
         "\n"
         "This is intended as an easy way to test the regex index. The regex "
         "index is a small in-memory index used to process Netspeak regex "
         "words.\n"
         "\n"
         "Example:\n"
         "    netspeak4 regex -i \"/path/to/index/regex-list/regex-vocab\"";
};

void RegexCommand::add_options(
    boost::program_options::options_description_easy_init& easy_init) {
  easy_init("in,i", bpo::value<std::string>()->required(),
            "The path of the regex list.\n"
            "\n"
            "This has to be a file where each line contains a word and all "
            "line are separated using one new line character (\\n).");
}

std::string load_file(std::string path) {
  std::ifstream ifs(path);
  netspeak::util::check(ifs.is_open(), netspeak::error_message::cannot_open,
                        path);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  ifs.close();
  return content;
}

int RegexCommand::run(boost::program_options::variables_map variables) {
  const auto file = variables["in"].as<std::string>();
  const auto string = load_file(file);

  std::cout << "Constructing regex index...\n";
  const auto init_start = std::chrono::steady_clock::now();
  const netspeak::regex::DefaultRegexIndex index(string);
  const auto init_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - init_start);
  std::cout << "Took " << init_time.count()
            << "ms to construtct the regex index.\n";

  while (true) {
    std::string query;
    std::cout << "\nEnter query (empty query to exit): ";
    std::getline(std::cin, query);
    if (query.empty()) {
      std::cout << "Exit." << std::endl;
      break;
    }

    const auto regex_query = netspeak::regex::parse_netspeak_regex_query(query);
    const auto timeout = std::chrono::milliseconds(1000);
    std::vector<std::string> matches;


    const auto match_start = std::chrono::steady_clock::now();
    index.match_query(regex_query, matches, 20, timeout);
    const auto match_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - match_start);

    std::cout << "Found " << matches.size() << " matche(s) in "
              << match_time.count() << "ms:\n";
    for (size_t i = 0; i < matches.size(); i++) {
      std::cout << "  " << std::setw(4) << std::right << (i + 1) << "  "
                << std::setw(60) << std::left << matches[i] << "\n";
    }
  }

  return EXIT_SUCCESS;
}

} // namespace cli
