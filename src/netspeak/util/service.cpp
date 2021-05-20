#include "netspeak/util/service.hpp"

namespace grpc {

std::ostream& operator<<(std::ostream& out, const grpc::Status& status) {
  return out << "Code " << status.error_code() << ": " << status.error_message()
             << "\n"
             << "Details: " << status.error_details();
}

} // namespace grpc

namespace netspeak {
namespace service {

std::ostream& operator<<(std::ostream& out, const Corpus& corpus) {
  return out << corpus.name() << " (key=\"" << corpus.key()
             << "\" lang=" << corpus.language() << ")";
}

} // namespace service
} // namespace netspeak
