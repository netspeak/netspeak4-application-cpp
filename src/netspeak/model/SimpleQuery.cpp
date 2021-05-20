#include "netspeak/model/SimpleQuery.hpp"

#include <algorithm>
#include <cassert>
#include <ostream>

#include "netspeak/error.hpp"
#include "netspeak/util/Vec.hpp"


namespace netspeak {
namespace model {

typedef SimpleQuery::Unit Unit;


inline Unit Unit::new_unit(Tag tag, const Text& text, const Source& source) {
  return Unit(tag, text, source);
  ;
}
Unit Unit::new_word(const Text& text, const Source& source) {
  return Unit::new_unit(Tag::WORD, text, source);
}
Unit Unit::new_regex(const Text& text, const Source& source) {
  return Unit::new_unit(Tag::REGEX, text, source);
}
Unit Unit::new_qmark(const Source& source) {
  return Unit::new_unit(Tag::QMARK, "?", source);
}
Unit Unit::new_star(const Source& source) {
  return Unit::new_unit(Tag::STAR, "*", source);
}
Unit Unit::new_concat(const Source& source) {
  return Unit::new_unit(Tag::CONCAT, "", source);
}
Unit Unit::new_alternation(const Source& source) {
  return Unit::new_unit(Tag::ALTERNATION, "", source);
}

Unit Unit::clone() const {
  auto unit = Unit::new_unit(tag(), text(), source());
  for (const auto& child : children()) {
    unit.add_child(child.clone());
  }
  return unit;
}


bool Unit::is_terminal() const {
  switch (tag_) {
    case Unit::Tag::WORD:
    case Unit::Tag::QMARK:
    case Unit::Tag::STAR:
    case Unit::Tag::REGEX:
      return true;

    default:
      return false;
  }
}


LengthRange Unit::length_range() const {
  switch (tag_) {
    case Unit::Tag::STAR:
      return LengthRange(0);

    case Unit::Tag::WORD:
    case Unit::Tag::QMARK:
    case Unit::Tag::REGEX:
      return LengthRange(1, 1);

    case Unit::Tag::ALTERNATION: {
      LengthRange range;
      for (const auto& child : children()) {
        range |= child.length_range();
      }
      return range;
    }

    case Unit::Tag::CONCAT: {
      LengthRange range(0, 0);
      for (const auto& child : children()) {
        range &= child.length_range();
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

uint32_t Unit::min_length() const {
  switch (tag_) {
    case Unit::Tag::STAR:
      return 0;

    case Unit::Tag::WORD:
    case Unit::Tag::QMARK:
    case Unit::Tag::REGEX:
      return 1;

    case Unit::Tag::ALTERNATION: {
      uint32_t min = UINT32_MAX;
      for (const auto& child : children()) {
        const auto c_min = child.min_length();
        if (c_min < min) {
          if (c_min == 0) {
            return 0;
          }
          min = c_min;
        }
      }
      return min;
    }

    case Unit::Tag::CONCAT: {
      uint32_t min = 0;
      for (const auto& child : children()) {
        const auto c_min = child.min_length();
        if (c_min == UINT32_MAX) {
          return UINT32_MAX;
        }
        min += c_min;
      }
      return min;
    }

    default:
      throw tracable_logic_error("Unknown tag");
  }
}

void Unit::add_child(Unit unit) {
  if (is_terminal()) {
    throw tracable_logic_error("A non-terminal cannot have children.");
  }

  children_.push_back(std::move(unit));
}
Unit Unit::pop_back() {
  if (children_.empty()) {
    throw tracable_logic_error("Cannot pop back of a unit without children.");
  }
  return util::vec_pop(children_);
}
Unit Unit::remove_child(size_t index) {
  if (index >= children_.size()) {
    throw tracable_logic_error("Index out of range.");
  }
  Unit child = std::move(children_[index]);
  children_.erase(children_.begin() + index);
  return child;
}

void Unit::clear_children() {
  children_.clear();
}
std::vector<Unit> Unit::drain_children() {
  std::vector<Unit> ret;
  std::swap(children_, ret);
  return ret;
}


bool operator==(const Unit& lhs, const Unit& rhs) {
  if (lhs.tag() != rhs.tag()) {
    return false;
  }
  if (lhs.is_terminal()) {
    return lhs.text() == rhs.text();
  } else {
    const auto& l_children = lhs.children();
    const auto& r_children = rhs.children();
    if (l_children.size() != r_children.size()) {
      return false;
    }
    for (size_t i = 0; i < l_children.size(); i++) {
      const auto& l_child = l_children[i];
      const auto& r_child = r_children[i];
      if (l_child != r_child) {
        return false;
      }
    }
    return true;
  }
}


bool unit_contains_wildcards(const Unit& unit) {
  if (unit.is_terminal()) {
    for (const auto& child : unit.children()) {
      if (unit_contains_wildcards(child)) {
        return true;
      }
    }
    return false;
  } else {
    return unit.tag() == Unit::Tag::QMARK || unit.tag() == Unit::Tag::STAR;
  }
}
bool SimpleQuery::has_wildcards() const {
  return unit_contains_wildcards(root());
}


std::ostream& operator<<(std::ostream& out, const SimpleQuery::Unit::Tag& tag) {
  switch (tag) {
    case Unit::Tag::WORD:
      return out << "Word";
    case Unit::Tag::QMARK:
      return out << "QMark";
    case Unit::Tag::STAR:
      return out << "Star";
    case Unit::Tag::REGEX:
      return out << "Regex";

    case Unit::Tag::ALTERNATION:
      return out << "Alternation";
    case Unit::Tag::CONCAT:
      return out << "Concat";

    default:
      throw tracable_logic_error("Unknown tag");
  }
}
std::ostream& operator<<(std::ostream& out, const SimpleQuery::Unit& unit) {
  switch (unit.tag()) {
    case Unit::Tag::WORD:
      // TODO: Universal word escaping. This also affects Query and NormQuery
      return out << "\"" << unit.text() << "\"";
    case Unit::Tag::QMARK:
      return out << "?";
    case Unit::Tag::STAR:
      return out << "*";
    case Unit::Tag::REGEX:
      return out << "Regex(\"" << unit.text() << "\")";

    case Unit::Tag::ALTERNATION: {
      auto it = unit.children().begin();
      auto end = unit.children().end();

      if (it == end) {
        out << "Alt{}";
      } else {
        out << "Alt{ " << *it;
        it++;
        for (; it != end; it++) {
          out << " | " << *it;
        }
        out << " }";
      }

      return out;
    }

    case Unit::Tag::CONCAT: {
      auto it = unit.children().begin();
      auto end = unit.children().end();

      if (it == end) {
        out << "Concat{}";
      } else {
        out << "Concat{";
        for (; it != end; it++) {
          out << " " << *it;
        }
        out << " }";
      }

      return out;
    }

    default:
      throw tracable_logic_error("Unknown tag");
  }
}
std::ostream& operator<<(std::ostream& out, const SimpleQuery& query) {
  return out << "SimpleQuery( " << query.root() << " )";
}


} // namespace model
} // namespace netspeak
