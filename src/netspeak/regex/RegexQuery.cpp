#include "netspeak/regex/RegexQuery.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <ostream>

namespace netspeak {
namespace regex {

/*
 * The implementation will do the following optimizations:
 *
 * 1.  `q°` == `°q` == `q`
 * 2.  `*` == `**`
 * 3.  `?*` == `*?`
 * 4.  `[aa]` == `[a]` == `a`
 * 5.  `()` == `°`
 * 6.  `(w)*` == `*(w)` == `*`
 *
 * Syntax:
 *  -  ° is the empty word
 *  -  q is a query
 *  -  [abc] is a char set
 *  -  (abc) is an optional word
 */

RegexQuery RegexQuery::create_reject_all() {
  std::vector<RegexUnit> units;
  units.push_back(RegexUnit::char_set(U""));
  return RegexQuery(units);
}

uint32_t RegexQuery::combinations_upper_bound() const {
  uint64_t count = 1;
  for (const auto& unit : m_units) {
    switch (unit.type) {
      case RegexUnit::Type::QMARK:
      case RegexUnit::Type::STAR:
        return UINT32_MAX;

      case RegexUnit::Type::OPTIONAL_WORD:
        count *= 2;
        break;

      case RegexUnit::Type::CHAR_SET:
        count *= unit.value.size();
        break;

      default:
        break;
    }

    if (count == 0)
      return 0;
    if (count >= UINT32_MAX)
      return UINT32_MAX;
  }
  return count;
}

const size_t MIN_UTF8_BYTES_PER_CHAR = 1;
const size_t MAX_UTF8_BYTES_PER_CHAR = 4;
// More information
// https://en.wikipedia.org/wiki/UTF-8#Description
size_t number_of_utf8_bytes(char32_t c) {
  if (c <= 0x7F)
    return 1;
  if (c <= 0x7FF)
    return 2;
  if (c <= 0xFFFF)
    return 3;
  return 4;
}

size_t RegexQuery::min_utf8_input_length() const {
  size_t min = 0;
  for (const auto& unit : m_units) {
    switch (unit.type) {
      case RegexUnit::Type::WORD:
        for (const auto c : unit.value) {
          min += number_of_utf8_bytes(c);
        }
        break;

      case RegexUnit::Type::QMARK:
        min += MIN_UTF8_BYTES_PER_CHAR;
        break;

      case RegexUnit::Type::CHAR_SET: {
        if (unit.value.empty()) {
          return SIZE_MAX;
        }
        size_t char_set_min = SIZE_MAX;
        for (const auto c : unit.value) {
          const auto c_min = number_of_utf8_bytes(c);
          if (c_min < char_set_min) {
            char_set_min = c_min;
            if (char_set_min == MIN_UTF8_BYTES_PER_CHAR) {
              break;
            }
          }
        }
        min += char_set_min;
        break;
      }

      default:
        // ignore all other tokens
        break;
    }
  }
  return min;
}
size_t RegexQuery::max_utf8_input_length() const {
  size_t max = 0;
  for (const auto& unit : m_units) {
    switch (unit.type) {
      case RegexUnit::Type::WORD:
      case RegexUnit::Type::OPTIONAL_WORD:
        for (const auto c : unit.value) {
          max += number_of_utf8_bytes(c);
        }
        break;

      case RegexUnit::Type::QMARK:
        max += MAX_UTF8_BYTES_PER_CHAR;
        break;

      case RegexUnit::Type::CHAR_SET: {
        // it's not defined what should happen if the char_set doesn't contain
        // characters, so we just do whatever
        size_t char_set_max = 0;
        for (const auto c : unit.value) {
          const auto c_max = number_of_utf8_bytes(c);
          if (c_max > char_set_max) {
            char_set_max = c_max;
            if (char_set_max == MAX_UTF8_BYTES_PER_CHAR) {
              break;
            }
          }
        }
        max += char_set_max;
        break;
      }

      case RegexUnit::Type::STAR:
        return SIZE_MAX;
    }
  }
  return max;
}

/**
 * @brief Removes all trailing optional words.
 *
 * There may be any number of qmarks and optional words between optional words
 * and the end.
 *
 * Example: The query `f(a)?(b)?` (`()` denotes optional words) will be
 * transformed to `f??`.
 *
 * This function is intended to be used before adding a star (`*`) to the query.
 * This function will then apply some of the absorption and commutative rules to
 * simplify the query.
 *
 * @param units
 */
void remove_trailing_optional_words(std::vector<RegexUnit>& units) {
  // remove qmarks and optional words

  size_t qmarks = 0;
  while (!units.empty()) {
    const auto type = units.back().type;
    if (type == RegexUnit::Type::QMARK) {
      qmarks++;
      units.pop_back();
    } else if (type == RegexUnit::Type::OPTIONAL_WORD) {
      units.pop_back();
    } else {
      // fun's over. We hit a unit which isn't a QMARK or optional word
      break;
    }
  }

  // add removed qmarks back in
  // (all qmarks are equivalent, so we'll just create new ones)

  while (qmarks-- > 0) {
    units.push_back(RegexUnit::qmark());
  }
}

/**
 * @brief Returns a new string in which all duplicate characters of the given
 * string are removed.
 *
 * The order of the characters might be different from the input string.
 *
 * @param characters
 * @return std::u32string
 */
std::u32string without_duplicates(const std::u32string& characters) {
  if (characters.size() < 2) {
    return std::u32string(characters);
  }

  std::vector<char32_t> chars(characters.begin(), characters.end());
  std::sort(chars.begin(), chars.end());

  std::u32string dupFree;
  char32_t last = chars[0];
  dupFree.push_back(last);
  for (size_t i = 1; i < chars.size(); i++) {
    char32_t ch = chars[i];
    if (ch != last) {
      last = ch;
      dupFree.push_back(last);
    }
  }

  return dupFree;
}

RegexUnit optimize_unit(RegexUnit& unit) {
  if (unit.type == RegexUnit::Type::CHAR_SET) {
    std::u32string set = without_duplicates(unit.value);
    if (set.size() == 1) {
      // rule 4
      return RegexUnit::word(set);
    } else {
      return RegexUnit::char_set(set);
    }
  }

  return unit;
}

void RegexQueryBuilder::add(RegexUnit unit) {
  if (reject_all) {
    // we can't recover from the reject_all state, so we don't need to done
    // anything anymore
    return;
  }

  unit = optimize_unit(unit);

  if (unit.value.empty()) {
    // there's no point in concatenating the empty string
    if (unit.type == RegexUnit::Type::WORD ||
        unit.type == RegexUnit::Type::OPTIONAL_WORD) {
      return; // rule 1, 5
    }

    // make reject all
    if (unit.type == RegexUnit::Type::CHAR_SET) {
      reject_all = true;
      return;
    }
  }

  // if the vector is empty, just add the unit
  if (units.empty()) {
    units.push_back(unit);
    return;
  }

  // using the above rules, we will try to eliminate and simplify as many units
  // as possible.

  // The order of `?`s and `*`s generally doesn't matter but we will strictly
  // enforce the order `?` < `*`. This will actually make optimizing easier.

  auto prev = units.back();
  switch (unit.type) {
    case RegexUnit::Type::WORD:
      if (prev.type == RegexUnit::Type::WORD) {
        // concatenate adjacent words.
        std::u32string concat(prev.value);
        concat.append(unit.value);
        units.back() = RegexUnit::word(concat);
        return;
      }
      break;

    case RegexUnit::Type::QMARK:
      if (prev.type == RegexUnit::Type::STAR) {
        // switch their order (rule 3)
        units.back() = unit;
        units.push_back(prev);
        return;
      }
      break;

    case RegexUnit::Type::STAR:
      if (prev.type == RegexUnit::Type::STAR) {
        return; // rule 2
      }
      if (prev.type == RegexUnit::Type::OPTIONAL_WORD ||
          prev.type == RegexUnit::Type::QMARK) {
        // rule 6, 3
        remove_trailing_optional_words(units);
        units.push_back(unit);
        return;
      }
      break;

    case RegexUnit::Type::CHAR_SET:
      // char_sets are already optimized as is, so we don't have to do anything
      break;

    case RegexUnit::Type::OPTIONAL_WORD:
      if (prev.type == RegexUnit::Type::STAR) {
        return; // rule 6
      }
      break;
  }

  // just add the unit
  units.push_back(unit);
}


std::ostream& operator<<(std::ostream& outputStream, const RegexQuery& query) {
  if (query.reject_all()) {
    return outputStream << "[]REJECT_ALL[]";
  } else {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

    for (const auto& unit : query.get_units()) {
      switch (unit.type) {
        case RegexUnit::Type::QMARK:
          outputStream << "?";
          break;
        case RegexUnit::Type::STAR:
          outputStream << "*";
          break;

        case RegexUnit::Type::CHAR_SET:
          outputStream << "[";
          outputStream << conv.to_bytes(unit.value);
          outputStream << "]";
          break;

        case RegexUnit::Type::OPTIONAL_WORD:
          outputStream << "(";
          outputStream << conv.to_bytes(unit.value);
          outputStream << ")";
          break;

        default:
          outputStream << conv.to_bytes(unit.value);
          break;
      }
    }

    return outputStream;
  }
}


} // namespace regex
} // namespace netspeak
