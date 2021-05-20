#ifndef NETSPEAK_SERVICE_TRACKING_HPP
#define NETSPEAK_SERVICE_TRACKING_HPP


#include <string>

#include "netspeak/service/NetspeakService.grpc.pb.h"
#include "netspeak/service/NetspeakService.pb.h"


namespace netspeak {
namespace service {

bool is_valid_tracking_id(const std::string& id);

void set_tracking_id(grpc::ClientContext& context, const std::string& id);

std::string get_tracking_id(const grpc::ServerContext& context);

} // namespace service
} // namespace netspeak


#endif
