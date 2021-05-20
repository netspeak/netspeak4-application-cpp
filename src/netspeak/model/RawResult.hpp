#ifndef NETSPEAK_MODEL_RAW_RESULT_HPP
#define NETSPEAK_MODEL_RAW_RESULT_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/RawPhraseResult.hpp"
#include "netspeak/model/RawRefResult.hpp"


namespace netspeak {
namespace model {


/**
 * @brief A result set of phrase and phrase references of multiple norm queries.
 */
class RawResult {
public:
  struct PhraseItem {
  public:
    std::shared_ptr<const NormQuery> query;
    std::shared_ptr<const RawPhraseResult> result;
  };
  struct RefItem {
  public:
    std::shared_ptr<const NormQuery> query;
    std::shared_ptr<const RawRefResult> result;
  };

private:
  std::vector<PhraseItem> phrases_;
  std::vector<RefItem> refs_;
  std::set<std::string> unknown_words_;

public:
  RawResult() {}

  /**
   * @brief All added phrase result sets with at least one phrase.
   */
  const std::vector<PhraseItem>& phrases() const {
    return phrases_;
  }
  /**
   * @brief All added phrase reference result sets with at least one reference.
   */
  const std::vector<RefItem>& refs() const {
    return refs_;
  }

  bool empty() const {
    return refs_.empty() && phrases_.empty();
  }

  void add_item(const NormQuery& query,
                const std::shared_ptr<const RawPhraseResult>& result);
  void add_item(const NormQuery& query,
                const std::shared_ptr<const RawRefResult>& result);

  const std::set<std::string>& unknown_words() const {
    return unknown_words_;
  }
};


} // namespace model
} // namespace netspeak


#endif
