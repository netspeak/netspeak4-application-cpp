#ifndef NETSPEAK_SERVICE_REQUEST_LOGGER_HPP
#define NETSPEAK_SERVICE_REQUEST_LOGGER_HPP


#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "boost/filesystem.hpp"
#include "boost/optional.hpp"

#include "netspeak/Netspeak.hpp"
#include "netspeak/service/NetspeakService.grpc.pb.h"
#include "netspeak/service/NetspeakService.pb.h"
#include "netspeak/util/Mut.hpp"


namespace netspeak {
namespace service {

class RequestLogger final : public NetspeakService::Service {
private:
  std::unique_ptr<NetspeakService::Service> service_;
  std::atomic<uint64_t> req_counter_;

  util::Mut<std::ofstream> f_search_req_;
  util::Mut<std::ofstream> f_search_error;

  util::Mut<std::ofstream> f_get_corpora_req_;
  util::Mut<std::ofstream> f_get_corpora_error;

public:
  RequestLogger() = delete;
  RequestLogger(const RequestLogger&) = delete;
  RequestLogger(std::unique_ptr<NetspeakService::Service> service,
                boost::filesystem::path log_dir);
  ~RequestLogger() override {}

  grpc::Status Search(grpc::ServerContext* context,
                      const SearchRequest* request,
                      SearchResponse* response) override;
  grpc::Status GetCorpora(grpc::ServerContext* context,
                          const CorporaRequest* request,
                          CorporaResponse* response) override;
};

} // namespace service
} // namespace netspeak


#endif
