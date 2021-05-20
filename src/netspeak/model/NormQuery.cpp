#include "netspeak/model/NormQuery.hpp"

#include <ostream>

#include "netspeak/error.hpp"
#include "netspeak/util/exception.hpp"


namespace netspeak {
namespace model {


typedef NormQuery::Unit Unit;


const Unit::Text QMARK_STR = std::make_shared<std::string>("?");

Unit Unit::word(const Text& text, const Source& source) {
  return Unit(Tag::WORD, text, source);
}
Unit Unit::qmark(const Source& source) {
  return Unit(Tag::QMARK, QMARK_STR, source);
}

bool Unit::operator==(const Unit& rhs) const {
  if (tag() != rhs.tag()) {
    return false;
  }
  if (tag() == Unit::Tag::WORD) {
    return *text() == *rhs.text();
  }
  return true;
}


const NormQuery NormQuery::EMPTY = NormQuery();

bool NormQuery::has_qmarks() const {
  for (const auto& unit : units()) {
    if (unit.tag() == Unit::Tag::QMARK) {
      return true;
    }
  }
  return false;
}
bool NormQuery::has_words() const {
  for (const auto& unit : units()) {
    if (unit.tag() == Unit::Tag::WORD) {
      return true;
    }
  }
  return false;
}

size_t NormQuery::count_qmarks() const {
  size_t count = 0;
  for (const auto& unit : units()) {
    if (unit.tag() == Unit::Tag::QMARK) {
      count++;
    }
  }
  return count;
}
size_t NormQuery::count_words() const {
  size_t count = 0;
  for (const auto& unit : units()) {
    if (unit.tag() == Unit::Tag::WORD) {
      count++;
    }
  }
  return count;
}

bool NormQuery::operator==(const NormQuery& rhs) const {
  if (size() != rhs.size()) {
    return false;
  }
  for (size_t i = 0; i != size(); i++) {
    if (units()[i] != rhs.units()[i]) {
      return false;
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& out, const NormQuery::Unit::Tag& tag) {
  switch (tag) {
    case Unit::Tag::WORD:
      return out << "Word";
    case Unit::Tag::QMARK:
      return out << "QMark";

    default:
      throw tracable_logic_error("Unknown tag");
  }
}
std::ostream& operator<<(std::ostream& out, const NormQuery::Unit& unit) {
  switch (unit.tag()) {
    case Unit::Tag::WORD:
      return out << "\"" << *unit.text() << "\"";
    case Unit::Tag::QMARK:
      return out << "?";

    default:
      throw tracable_logic_error("Unknown tag");
  }

  return out;
}
std::ostream& operator<<(std::ostream& out, const NormQuery& query) {
  // This will out the norm query "foo ?" as:
  //
  //   NormQuery( "foo" ? )
  //
  // Each unit will be printed as its text (quoted for WORDs).

  out << "NormQuery(";
  for (const auto& unit : query.units()) {
    out << " " << unit;
  }
  out << " )";

  return out;
}

} // namespace model
} // namespace netspeak
