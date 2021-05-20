#include "netspeak/service/UniqueMap.hpp"

#include "netspeak/error.hpp"


namespace netspeak {
namespace service {

UniqueMap::UniqueMap(std::unique_ptr<std::vector<entry>> entries_ptr)
    : instances_(), corpora_() {
  if (!entries_ptr) {
    throw std::logic_error("The pointer of UniqueMap entries is null.");
  }

  auto& entries = *entries_ptr;
  for (size_t i = 0; i != entries.size(); i++) {
    UniqueMap::entry e = std::move(entries[i]);

    std::string key = e.corpus.key();
    if (instances_.find(key) != instances_.end()) {
      throw tracable_logic_error("Duplicate corpus key " + key);
    }

    instances_.emplace(key, std::move(e.instance));
    corpora_.push_back(std::move(e.corpus));
  }
}

grpc::Status UniqueMap::Search_(grpc::ServerContext*,
                                const SearchRequest* request,
                                SearchResponse* response) const {
  auto it = instances_.find(request->corpus());
  // check the corpus
  if (it == instances_.end()) {
    auto error = response->mutable_error();
    error->set_kind(SearchResponse::Error::INVALID_CORPUS);
    error->set_message("Unknown corpus");
    return grpc::Status::OK;
  }

  // forward the request to the Netspeak instance
  // (search is guaranteed not to throw, so we don't need to do anything)
  const auto& instance = it->second;
  instance->search(*request, *response);
  return grpc::Status::OK;
}

grpc::Status UniqueMap::GetCorpora_(grpc::ServerContext*, const CorporaRequest*,
                                    CorporaResponse* response) const {
  // just add a copy of all corpora
  for (const auto& corpus : corpora_) {
    response->add_corpora()->CopyFrom(corpus);
  }
  return grpc::Status::OK;
}


} // namespace service
} // namespace netspeak
