#ifndef NETSPEAK_SERVICE_LOAD_BALANCE_PROXY_HPP
#define NETSPEAK_SERVICE_LOAD_BALANCE_PROXY_HPP


#include <memory>
#include <unordered_map>

#include "netspeak/service/NetspeakService.grpc.pb.h"
#include "netspeak/service/NetspeakService.pb.h"


namespace netspeak {
namespace service {

/**
 * @brief A proxy to unify and load-balance any number of compatible Netspeak
 * indexes.
 *
 * Two indexes are compatible if their corpus key is different or if all their
 * coprus information (key, language, name) is equal.
 *
 * All operations of this class are thread safe.
 */
class LoadBalanceProxy final : public NetspeakService::Service {
public:
  typedef std::shared_ptr<NetspeakService::Stub> StubPtr;
  typedef std::vector<std::pair<Corpus, StubPtr>> StubVector;

private:
  std::unordered_map<std::string, std::vector<StubPtr>> services_;
  std::vector<Corpus> corpora_;

public:
  LoadBalanceProxy() = delete;
  LoadBalanceProxy(const LoadBalanceProxy&) = delete;
  LoadBalanceProxy(const StubVector& stubs);
  ~LoadBalanceProxy() override {}
  grpc::Status Search(grpc::ServerContext* context,
                      const SearchRequest* request,
                      SearchResponse* response) override {
    return Search_(context, request, response);
  }
  grpc::Status GetCorpora(grpc::ServerContext* context,
                          const CorporaRequest* request,
                          CorporaResponse* response) override {
    return GetCorpora_(context, request, response);
  }

private:
  // These functions exists because we cannot declare the override as `const`.
  // The `const` means that the compiler will not allow us to modify the state
  // of the object and as an immutable object, it's inherently thread-safe.

  grpc::Status Search_(grpc::ServerContext* context,
                       const SearchRequest* request,
                       SearchResponse* response) const;
  grpc::Status GetCorpora_(grpc::ServerContext* context,
                           const CorporaRequest* request,
                           CorporaResponse* response) const;

public:
  /**
   * @brief Returns the whether the two corpora are compatible by the definition
   * of this proxy.
   *
   * @param a
   * @param b
   */
  static bool areCompatible(const Corpus& a, const Corpus& b);
};

} // namespace service
} // namespace netspeak


#endif
