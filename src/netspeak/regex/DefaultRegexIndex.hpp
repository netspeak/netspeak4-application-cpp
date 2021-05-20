#ifndef NETSPEAK_REGEX_DEFAULT_REGEX_INDEX_HPP
#define NETSPEAK_REGEX_DEFAULT_REGEX_INDEX_HPP

#include <chrono>
#include <codecvt>
#include <locale>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/regex.hpp>

#include "netspeak/regex/RegexIndex.hpp"
#include "netspeak/regex/RegexQuery.hpp"


namespace netspeak {
namespace regex {

class DefaultRegexIndex : public RegexIndex {
public: // types
  struct WordEntry {
    uint32_t offset;
    uint16_t length;
  };

private: // state
  /**
   * @brief The vocabulary string of this instance.
   */
  std::string vocabulary_;
  /**
   * @brief A list of entries of every word in \c vocabulary .
   */
  std::vector<struct WordEntry> words_;
  /**
   * @brief A set of all characters in the vocabulary.
   */
  std::unordered_set<char32_t> all_chars_;
  /**
   * @brief A hash table containing all words.
   */
  std::vector<uint32_t> word_hash_table_;

private: // initialization functions
  void initialize_words();
  void initialize_all_chars();
  void initialize_word_hash_table();

private: // functions
  /**
   * @brief Returns the word string from the given entry.
   *
   * @param entry
   * @return std::string
   */
  std::string word_from_entry(const WordEntry& entry) const;
  /**
   * @brief Returns the word at the given index in the word list.
   *
   * 0 will returns the first word, 1 the second, and so on.
   *
   * @param index
   * @return std::string
   */
  std::string word_at_index(uint32_t index) const;

  /**
   * @brief Returns the index of the given word or \c UINT32_MAX if the given
   * word is not a word of the vocabulary.
   *
   * @param word
   * @return uint32_t
   */
  uint32_t find_word(const std::string& word) const;

  /**
   * @brief Optimizes the given query considering the properties of the
   * vocabulary of the index.
   *
   * @param query
   * @return RegexQuery
   */
  RegexQuery optimize_query(const RegexQuery& query) const;

  void match_query_hash_lookup(const RegexQuery& query,
                               std::vector<std::string>& matches,
                               uint32_t max_matches) const;

  void match_query_regex(const RegexQuery& query,
                         std::vector<std::string>& matches,
                         uint32_t max_matches,
                         std::chrono::nanoseconds timeout) const;

public:
  /**
   * @brief Construct a new Regex Index object.
   *
   * After the index is constructed, it is immutable. The index does not
   * implement any form of query result caching.
   *
   * @param vocabulary The by \c \n separated words to search in.
   */
  DefaultRegexIndex(std::string vocabulary);
  DefaultRegexIndex(const DefaultRegexIndex&) = delete;
  ~DefaultRegexIndex() override{};

  const std::string& vocabulary() const {
    return vocabulary_;
  }

  /**
   * @brief Adds all words matching the given query to the given vector.
   *
   * @param query The query which will be used to match words.
   * @param matches The vector to which the matches will be added.
   * @param max_matches The maximum number of words to add to \c matches
   * @param timeout The amount of time after which the search for more words
   * will be aborted.
   */
  void match_query(const RegexQuery& query, std::vector<std::string>& matches,
                   uint32_t max_matches,
                   std::chrono::nanoseconds timeout) const override;
};

} // namespace regex
} // namespace netspeak

#endif // NETSPEAK_REGEX_DEFAULT_REGEX_INDEX_HPP
