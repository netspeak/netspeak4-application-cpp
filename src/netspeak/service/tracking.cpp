#include "netspeak/service/tracking.hpp"


namespace netspeak {
namespace service {

bool is_valid_tracking_id(const std::string& id) {
  if (id.length() != 32) {
    return false;
  }

  for (const auto c : id) {
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F'))) {
      return false;
    }
  }

  return true;
}

void set_tracking_id(grpc::ClientContext& context, const std::string& id) {
  if (is_valid_tracking_id(id)) {
    context.AddMetadata("netspeak-tracking-id", id);
  }
}

std::string get_tracking_id(const grpc::ServerContext& context) {
  std::string tracking_id;

  const auto& metadata = context.client_metadata();
  auto it = metadata.find("netspeak-tracking-id");
  if (it != metadata.end()) {
    const auto id_raw = it->second;

    // the tracking id is expected to by an 128bit hexadecimal number.
    // if that is not case, the given tracking id will be ignored.
    // valid tracking ids will be normalized to all lower-case letters.

    if (id_raw.length() == 32) {
      for (const auto c : id_raw) {
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
          tracking_id.push_back(c);
        } else if (c >= 'A' && c <= 'F') {
          tracking_id.push_back('a' + c - 'A');
        } else {
          // invalid
          tracking_id.clear();
          break;
        }
      }
    }
  }

  return tracking_id;
}

} // namespace service
} // namespace netspeak
