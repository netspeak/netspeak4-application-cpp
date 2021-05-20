#include "netspeak/model/RawResult.hpp"


namespace netspeak {
namespace model {


void RawResult::add_item(const NormQuery& query,
                         const std::shared_ptr<const RawPhraseResult>& result) {
  // add unknown words
  unknown_words_.insert(result->unknown_words().begin(),
                        result->unknown_words().end());

  if (!result->empty()) {
    RawResult::PhraseItem item{ std::make_shared<NormQuery>(query), result };
    phrases_.push_back(std::move(item));
  }
}
void RawResult::add_item(const NormQuery& query,
                         const std::shared_ptr<const RawRefResult>& result) {
  // add unknown words
  unknown_words_.insert(result->unknown_words().begin(),
                        result->unknown_words().end());

  if (!result->empty()) {
    RawResult::RefItem item{ std::make_shared<NormQuery>(query), result };
    refs_.push_back(std::move(item));
  }
}


} // namespace model
} // namespace netspeak
