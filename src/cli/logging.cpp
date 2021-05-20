#include "cli/logging.hpp"

#include "boost/filesystem.hpp"

#include "netspeak/service/RequestLogger.hpp"


namespace cli {

using namespace netspeak::service;
namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

#define LOG_DIR_KEY "log-dir"

void add_logging_options(bpo::options_description_easy_init& easy_init) {
  easy_init(
      LOG_DIR_KEY ",l", bpo::value<std::string>(),
      "The directory in which log files will be created.\n"
      "\n"
      "Created log files will be named after the type of data they log and the "
      "current timestamp. This means that log files will be neither "
      "overwritten nor appended on subsequent starts.\n"
      "\n"
      "The log format is JSONLines. A unique request id, a tracking id, and "
      "the full request are logged everytime. For more information about the "
      "logging format see "
      "https://git.webis.de/code-generic/code-webis-faq/-/blob/master/"
      "README.md#how-to-work-on-logging-for-a-service (Webis internal).\n"
      "\n"
      "If the given directory doesn't exist already, it will be created.");
}

std::unique_ptr<NetspeakService::Service> add_logging(
    bpo::variables_map& variables,
    std::unique_ptr<NetspeakService::Service> service) {
  if (variables.count(LOG_DIR_KEY) > 0) {
    // add a logger
    bfs::path log_dir(variables[LOG_DIR_KEY].as<std::string>());
    return std::make_unique<RequestLogger>(std::move(service), log_dir);
  } else {
    return service;
  }
}

} // namespace cli
