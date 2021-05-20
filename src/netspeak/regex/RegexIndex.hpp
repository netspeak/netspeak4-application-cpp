#ifndef NETSPEAK_REGEX_REGEX_INDEX_HPP
#define NETSPEAK_REGEX_REGEX_INDEX_HPP

#include <chrono>
#include <string>
#include <vector>

#include "netspeak/regex/RegexQuery.hpp"


namespace netspeak {
namespace regex {

class RegexIndex {
public:
  virtual ~RegexIndex() {}

  /**
   * @brief Adds all words matching the given query to the given vector.
   *
   * @param query The query which will be used to match words.
   * @param matches The vector to which the matches will be added.
   * @param max_matches The maximum number of words to add to \c matches
   * @param timeout The amount of time after which the search for more words
   * will be aborted.
   */
  virtual void match_query(const RegexQuery& query,
                           std::vector<std::string>& matches,
                           uint32_t max_matches,
                           std::chrono::nanoseconds timeout) const = 0;
};

} // namespace regex
} // namespace netspeak

#endif // NETSPEAK_REGEX_REGEX_INDEX_HPP
