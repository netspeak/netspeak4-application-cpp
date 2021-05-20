#ifndef NETSPEAK_MODEL_QUERY_HPP
#define NETSPEAK_MODEL_QUERY_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "boost/optional.hpp"

#include "netspeak/model/LengthRange.hpp"


namespace netspeak {
namespace model {

class QueryUnit__ {
public:
  enum class Tag {
    // Terminals
    WORD,
    QMARK,
    STAR,
    PLUS,
    REGEX,
    DICTSET,

    // Non-terminals
    ORDERSET,
    OPTIONSET,
    ALTERNATION,
    CONCAT,
  };

private:
  typedef QueryUnit__ Unit;

  Tag tag_;
  std::string text_;
  std::vector<std::shared_ptr<const Unit>> children_;
  std::weak_ptr<const Unit> parent_;
  // This is a weak ref to this object. This is necessary to set the parent of
  // children. Not nice, but hey. It works.
  std::weak_ptr<Unit> self_;

public:
  QueryUnit__(Tag tag, const std::string& text) : tag_(tag), text_(text) {}
  QueryUnit__() = delete;
  QueryUnit__(const QueryUnit__&) = delete;

  inline static std::shared_ptr<Unit> terminal(Unit::Tag tag,
                                               const std::string& text) {
    const auto unit = std::make_shared<Unit>(tag, text);
    unit->self_ = unit;
    return unit;
  }
  inline static std::shared_ptr<Unit> non_terminal(Unit::Tag tag) {
    const auto unit = std::make_shared<Unit>(tag, "");
    unit->self_ = unit;
    return unit;
  }


  Tag tag() const {
    return tag_;
  }

  /**
   * The unescaped text value of the unit.
   *
   * "Unescaped" means that backslashes used to escape characters (e.g. in words
   * and regexes) and prefixes used to identify the token (e.g. the `#` of dict
   * sets) are already handled.
   *
   * This value is only relevant for terminals. The text of non-terminals is
   * empty.
   *
   * @return const std::string&
   */
  const std::string& text() const {
    return text_;
  }

  const std::vector<std::shared_ptr<const Unit>>& children() const {
    return children_;
  }

  std::shared_ptr<const Unit> parent() const {
    return parent_.lock();
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


  void add_child(const std::shared_ptr<Unit>& unit);
};


/**
 * The internal representation of a parsed Netspeak query.
 *
 * This is a literal representation. While completely parsed and interpreted, it
 * will literally represent all syntactical structures of the textual query.
 */
class Query {
public:
  typedef QueryUnit__ Unit;

private:
  std::shared_ptr<Unit> alternatives_;

  Query(const Query&) = delete;

public:
  Query() : alternatives_(Unit::non_terminal(Unit::Tag::ALTERNATION)){};

  const std::shared_ptr<Unit>& alternatives() const {
    return alternatives_;
  }


  LengthRange length_range() const {
    return alternatives()->length_range();
  }
};


std::ostream& operator<<(std::ostream& out, const Query::Unit::Tag& tag);
std::ostream& operator<<(std::ostream& out, const Query::Unit& unit);
std::ostream& operator<<(std::ostream& out, const Query& query);


} // namespace model
} // namespace netspeak


#endif
