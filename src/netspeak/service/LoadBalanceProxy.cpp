#include "netspeak/service/LoadBalanceProxy.hpp"

#include "netspeak/error.hpp"
#include "netspeak/util/service.hpp"

namespace netspeak {
namespace service {

bool LoadBalanceProxy::areCompatible(const Corpus& a, const Corpus& b) {
  if (a.key() != b.key()) {
    return true;
  } else {
    return a.language() == b.language() && a.name() == b.name();
  }
}

void check_compatible(const Corpus& a, const Corpus& b) {
  if (!LoadBalanceProxy::areCompatible(a, b)) {
    std::stringstream what;
    what << "The corpora " << a << " and " << b
         << " have the same key but are incompatible.";
    throw std::logic_error(what.str());
  }
}

void initialize_from_stubs(
    const LoadBalanceProxy::StubVector& stubs,
    std::unordered_map<std::string, std::vector<LoadBalanceProxy::StubPtr>>&
        services,
    std::vector<Corpus>& corpora) {
  std::unordered_map<std::string, Corpus> corpora_map;

  for (const auto& pair : stubs) {
    const auto& corpus = pair.first;
    const auto& stub = pair.second;
    const auto& key = corpus.key();

    // add or check the current corpus
    auto corpora_it = corpora_map.find(key);
    if (corpora_it == corpora_map.end()) {
      corpora.push_back(corpus);
      corpora_map.emplace(key, corpus);
    } else {
      check_compatible(corpora_it->second, corpus);
    }

    // add the stub
    auto it = services.find(key);
    if (it == services.end()) {
      std::vector<LoadBalanceProxy::StubPtr> list{ stub };
      services.emplace(key, list);
    } else {
      it->second.push_back(stub);
    }
  }
}

LoadBalanceProxy::LoadBalanceProxy(const StubVector& stubs)
    : services_(), corpora_() {
  initialize_from_stubs(stubs, services_, corpora_);
}


/**
 * @brief This determinestically returns a random number from the given number.
 *
 * @param x
 * @return uint64_t
 */
uint64_t bit_mix(uint64_t x) {
  // do 16 iterations of xorshift* using x as the seed
  // https://en.wikipedia.org/wiki/Xorshift#xorshift*
  for (int i = 16; i > 0; i--) {
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
  }
  return x * UINT64_C(0x2545F4914F6CDD1D);
}

grpc::Status LoadBalanceProxy::Search_(grpc::ServerContext*,
                                       const SearchRequest* request,
                                       SearchResponse* response) const {
  const auto& services_ref = services_;

  auto it = services_ref.find(request->corpus());
  // check the corpus
  if (it == services_ref.end()) {
    auto error = response->mutable_error();
    error->set_kind(SearchResponse::Error::INVALID_CORPUS);
    error->set_message("Unknown corpus");
    return grpc::Status::OK;
  }

  const auto& services = it->second;

  grpc::ClientContext context;
  if (services.size() == 1) {
    // There's only one service available, so just forward the request.
    return services[0]->Search(&context, *request, response);
  } else {
    // There are at least two services, so we have to choose which one to
    // forward to. To be more cache friendly, the service will be chosen based
    // on the hash of the query.
    // The hash will then be processed further to prevent bad hashes. This extra
    // processing is necessary as an attacker could exploit weaknesses in the
    // hash function provided by C++ to redirect a huge workload to one service.

    const auto hash = std::hash<std::string>{}(request->query());
    const auto index = bit_mix(hash) % services.size();
    return services[index]->Search(&context, *request, response);
  }
}

grpc::Status LoadBalanceProxy::GetCorpora_(grpc::ServerContext*,
                                           const CorporaRequest*,
                                           CorporaResponse* response) const {
  const auto& corpora_ref = corpora_; // copy the shared_ptr

  // just add a copy of all corpora
  for (const auto& corpus : corpora_ref) {
    response->add_corpora()->CopyFrom(corpus);
  }
  return grpc::Status::OK;
}


} // namespace service
} // namespace netspeak
