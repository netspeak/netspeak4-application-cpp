#include "netspeak/model/Query.hpp"

#include <cassert>
#include <ostream>

#include "netspeak/error.hpp"


namespace netspeak {
namespace model {

typedef Query::Unit Unit;


void Unit::add_child(const std::shared_ptr<Unit>& unit) {
  assert(!!unit);

  if (is_terminal()) {
    throw tracable_logic_error("A non-terminal cannot have children.");
  }
  if (unit->parent()) {
    throw tracable_logic_error("The unit is already has a parent.");
  }

  unit->parent_ = self_;
  children_.push_back(unit);
}

bool Unit::is_terminal() const {
  switch (tag()) {
    case Tag::WORD:
    case Tag::QMARK:
    case Tag::STAR:
    case Tag::PLUS:
    case Tag::REGEX:
    case Tag::DICTSET:
      return true;

    default:
      return false;
  }
}

LengthRange Unit::length_range() const {
  switch (tag()) {
    case Tag::WORD:
    case Tag::QMARK:
    case Tag::REGEX:
      return LengthRange(1, 1);
    case Tag::STAR:
      return LengthRange(0);
    case Tag::PLUS:
      return LengthRange(1);
    case Tag::DICTSET:
      // A dict set is guaranteed to match itself and all replacement phrases
      // may not be empty.
      return LengthRange(1);

    case Tag::OPTIONSET: {
      LengthRange range(0, 0);
      for (const auto& child : children()) {
        range |= child->length_range();
      }
      return range;
    }
    case Tag::ALTERNATION: {
      LengthRange range;
      for (const auto& child : children()) {
        range |= child->length_range();
      }
      return range;
    }

    case Tag::ORDERSET: // for order sets, the same rules as for concats apply
    case Tag::CONCAT: {
      LengthRange range(0, 0);
      for (const auto& child : children()) {
        range &= child->length_range();
        if (range.empty()) {
          return range;
        }
      }
      return range;
    }

    default:
      throw tracable_logic_error("Unknown tag");
  }
}


std::ostream& operator<<(std::ostream& out, const Unit::Tag& tag) {
  switch (tag) {
    case Unit::Tag::WORD:
      return out << "Word";
    case Unit::Tag::QMARK:
      return out << "QMark";
    case Unit::Tag::STAR:
      return out << "Star";
    case Unit::Tag::PLUS:
      return out << "Plus";
    case Unit::Tag::REGEX:
      return out << "Regex";
    case Unit::Tag::DICTSET:
      return out << "DictSet";

    case Unit::Tag::ORDERSET:
      return out << "OrderSet";
    case Unit::Tag::OPTIONSET:
      return out << "OptionSet";
    case Unit::Tag::ALTERNATION:
      return out << "Alternation";
    case Unit::Tag::CONCAT:
      return out << "Concat";

    default:
      throw tracable_logic_error("Unknown tag");
  }
}
std::ostream& operator<<(std::ostream& out, const Unit& unit) {
  out << unit.tag();

  switch (unit.tag()) {
    case Unit::Tag::QMARK:
    case Unit::Tag::STAR:
    case Unit::Tag::PLUS:
      // since, the text is trivial, we don't print anything
      break;

    case Unit::Tag::WORD:
    case Unit::Tag::REGEX:
    case Unit::Tag::DICTSET:
      out << "(\"" << unit.text() << "\")";
      break;

    case Unit::Tag::ORDERSET:
    case Unit::Tag::OPTIONSET:
    case Unit::Tag::ALTERNATION:
    case Unit::Tag::CONCAT: {
      out << "{";
      auto it = unit.children().begin();
      auto end = unit.children().end();
      if (it != end) {
        out << " " << **it;
        it++;
        for (; it != end; it++) {
          out << ", " << **it;
        }
      }
      out << " }";
      break;
    }

    default:
      throw tracable_logic_error("Unknown tag");
  }

  return out;
}
std::ostream& operator<<(std::ostream& out, const Query& query) {
  out << "Query(" << *query.alternatives() << ")";
  return out;
}


} // namespace model
} // namespace netspeak
