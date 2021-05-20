#ifndef NETSPEAK_REGEX_REGEX_QUERY_HPP
#define NETSPEAK_REGEX_REGEX_QUERY_HPP

#include <string>
#include <vector>


namespace netspeak {
namespace regex {

/**
 * @brief A basic unit of a regex query.
 *
 * A regex unit has a type which defines the function of its UTF32 string value.
 */
struct RegexUnit {
  /**
   * @brief The type of a unit.
   */
  enum class Type { QMARK, STAR, WORD, CHAR_SET, OPTIONAL_WORD };

  Type type;
  std::u32string value;

private:
  RegexUnit(Type type, const std::u32string& value)
      : type(type), value(value) {}

public:
  static RegexUnit qmark() {
    return RegexUnit(Type::QMARK, U"?");
  }
  static RegexUnit star() {
    return RegexUnit(Type::STAR, U"*");
  }
  static RegexUnit word(const std::u32string& word) {
    return RegexUnit(Type::WORD, word);
  }
  static RegexUnit char_set(const std::u32string& characters) {
    return RegexUnit(Type::CHAR_SET, characters);
  }
  static RegexUnit optional_word(const std::u32string& word) {
    return RegexUnit(Type::OPTIONAL_WORD, word);
  }
};


/**
 * @brief An immutable query for a regex index.
 *
 * Top create a query, either use a parser or the \c RegexQueryBuilder .
 */
class RegexQuery {
private:
  std::vector<RegexUnit> m_units;

public:
  RegexQuery(const std::vector<RegexUnit>& units) : m_units(units) {}

  /**
   * @brief Create a new \c RegexQuery that rejects all words.
   *
   * @return RegexQuery
   */
  static RegexQuery create_reject_all();

  /**
   * @brief Returns whether this query will reject all words.
   *
   * Note: All words in the context of a formal language.
   */
  bool reject_all() const {
    return combinations_upper_bound() == 0;
  }

  /**
   * @brief Returns whether this query will accept all words.
   *
   * Note: All words in the context of a formal language.
   */
  bool accept_all() const {
    return m_units.size() == 1 && m_units.front().type == RegexUnit::Type::STAR;
  }

  /**
   * @brief Returns whether this query will accept all non-empty words.
   *
   * Note: If the query accepts all words, it also accepts all non-empty words.
   *
   * Note: All words in the context of a formal language.
   */
  bool accept_all_non_empty() const {
    if (accept_all()) {
      return true;
    }
    if (m_units.size() == 2) {
      switch (m_units[0].type) {
        case RegexUnit::Type::STAR:
          return m_units[1].type == RegexUnit::Type::QMARK;
        case RegexUnit::Type::QMARK:
          return m_units[1].type == RegexUnit::Type::STAR;
        default:
          break;
      }
    }
    return false;
  }

  /**
   * @brief Returns an upper bound for the number of possible words, the query
   * can match. If \c UINT32_MAX is returned, the upper bound is essentially
   * infinite. If \c 0 is returned, the query cannot match any word.
   *
   * Queries which contain \c QMARK or \c star units will automatically return
   * \c UINT32_MAX .
   *
   * @return uint32_t
   */
  uint32_t combinations_upper_bound() const;

  /**
   * @brief Returns the minimum number of UTF8 bytes a word has to have to match
   * this query.
   *
   * If the query cannot match any words, \c SIZE_MAX is returned.
   *
   * @return size_t
   */
  size_t min_utf8_input_length() const;
  /**
   * @brief Returns the maximum number of UTF8 bytes a word can to have to match
   * this query.
   *
   * If the query can potentially match infinitely many words, \c SIZE_MAX is
   * returned.
   *
   * @return size_t
   */
  size_t max_utf8_input_length() const;

  /**
   * @brief Get the underlying units.
   *
   * @return std::vector<RegexUnit> const
   */
  const std::vector<RegexUnit>& get_units() const {
    return m_units;
  }
};

/**
 * @brief Appends the general string representation of a regex query.
 *
 * @param outputStream
 * @param query
 * @return std::ostream&
 */
std::ostream& operator<<(std::ostream& outputStream, const RegexQuery& query);


class RegexQueryBuilder {
private:
  std::vector<RegexUnit> units;
  bool reject_all;

public:
  RegexQueryBuilder() : units(), reject_all(false) {}

  /**
   * @brief Adds the given unit to the internal query.
   *
   * The builder might or might not actually add the given unit to the query.
   * The actual query is considered an implementation detail, so any number of
   * units might be added, changed, or removed as a result of this operation.
   *
   * The builder only guarantees is that the resulting query will behave as if
   * the given unit had been added.
   *
   * @param unit The unit to add.
   */
  void add(RegexUnit unit);

  /**
   * @brief Creates a query from the current internal state.
   *
   * @return RegexQuery
   */
  RegexQuery to_query() const {
    if (reject_all) {
      return RegexQuery::create_reject_all();
    } else {
      return RegexQuery(std::vector<RegexUnit>(units));
    }
  }
};


} // namespace regex
} // namespace netspeak

#endif // NETSPEAK_REGEX_REGEX_QUERY_HPP
