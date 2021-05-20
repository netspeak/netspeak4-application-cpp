#ifndef CLI_LOGGING_HPP
#define CLI_LOGGING_HPP

#include <memory>
#include <string>

#include "boost/program_options.hpp"

#include "netspeak/service/NetspeakService.grpc.pb.h"

namespace cli {

void add_logging_options(
    boost::program_options::options_description_easy_init& easy_init);

std::unique_ptr<netspeak::service::NetspeakService::Service> add_logging(
    boost::program_options::variables_map& variables,
    std::unique_ptr<netspeak::service::NetspeakService::Service> service);

} // namespace cli

#endif
