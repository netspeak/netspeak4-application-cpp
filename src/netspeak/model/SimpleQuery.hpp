#ifndef NETSPEAK_MODEL_SIMPLE_QUERY_HPP
#define NETSPEAK_MODEL_SIMPLE_QUERY_HPP

#include <string>
#include <vector>

#include "netspeak/model/LengthRange.hpp"
#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/Query.hpp"

namespace netspeak {
namespace model {

class SimpleQueryUnit__ {
public:
  enum class Tag {
    WORD,
    QMARK,
    STAR,
    REGEX,

    CONCAT,
    ALTERNATION,
  };
  typedef NormQuery::Unit::Source Source;
  typedef std::string Text;

private:
  typedef SimpleQueryUnit__ Unit;

  Tag tag_;
  Text text_;
  Source source_;
  std::vector<Unit> children_;

  static Unit new_unit(Tag tag, const Text& text, const Source& source);

public:
  SimpleQueryUnit__() = delete;
  SimpleQueryUnit__(const SimpleQueryUnit__&) = delete;
  SimpleQueryUnit__(SimpleQueryUnit__&&) = default;
  SimpleQueryUnit__& operator=(SimpleQueryUnit__&& other) = default;

  SimpleQueryUnit__(Tag tag, const Text& text, const Source& source)
      : tag_(tag), text_(text), source_(source){};

  static Unit new_word(const Text& text, const Source& source);
  static Unit new_regex(const Text& text, const Source& source);
  static Unit new_qmark(const Source& source);
  static Unit new_star(const Source& source);
  static Unit new_concat(const Source& source);
  static Unit new_alternation(const Source& source);

  Unit clone() const;

  Tag tag() const {
    return tag_;
  }
  const Text& text() const {
    return text_;
  }
  const Source& source() const {
    return source_;
  }
  std::vector<Unit>& children() {
    return children_;
  }
  const std::vector<Unit>& children() const {
    return children_;
  }

  bool is_terminal() const;

  /**
   * @brief Returns the range of lengths of phrases that can (theoretically) be
   * matched (= accepted) by this unit.
   *
   * If the returned range is empty, then this unit doesn't accept any phrases
   * at all.
   */
  LengthRange length_range() const;

  /**
   * @brief A possibly optimized version of \c length_range().min .
   */
  uint32_t min_length() const;

  void add_child(Unit);
  Unit pop_back();
  Unit remove_child(size_t index);
  void clear_children();
  std::vector<Unit> drain_children();
};

bool operator==(const SimpleQueryUnit__& lhs, const SimpleQueryUnit__& rhs);
inline bool operator!=(const SimpleQueryUnit__& lhs,
                       const SimpleQueryUnit__& rhs) {
  return !operator==(lhs, rhs);
}


class SimpleQuery {
public:
  typedef SimpleQueryUnit__ Unit;

private:
  Unit root_;

  SimpleQuery() = delete;
  SimpleQuery(const SimpleQuery&) = delete;

public:
  SimpleQuery(SimpleQuery&&) = default;
  SimpleQuery& operator=(SimpleQuery&& other) = default;

  SimpleQuery(Unit root) : root_(std::move(root)) {}

  Unit& root() {
    return root_;
  }
  const Unit& root() const {
    return root_;
  }

  bool has_wildcards() const;
};


std::ostream& operator<<(std::ostream& out, const SimpleQuery::Unit::Tag& tag);
std::ostream& operator<<(std::ostream& out, const SimpleQuery::Unit& unit);
std::ostream& operator<<(std::ostream& out, const SimpleQuery& query);


} // namespace model
} // namespace netspeak


#endif
