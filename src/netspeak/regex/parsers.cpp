#include "netspeak/regex/parsers.hpp"

#include <codecvt>
#include <locale>
#include <unordered_set>

#include <boost/optional.hpp>

#include "netspeak/model/QuerySyntax.hpp"


namespace netspeak {
namespace regex {

typedef model::QuerySyntax Syntax;


/**
 * @brief Reads the given range until it finds a given end characters. If it
 * finds the end characters, it will returns all characters between the start of
 * the given range and the end characters (inclusive beginning, exclusive end).
 *
 * @param begin
 * @param end
 * @param end_char
 * @return boost::optional<std::u32string>
 */
boost::optional<std::u32string> read_until_char(
    std::u32string::const_iterator begin, std::u32string::const_iterator end,
    char32_t end_char) {
  std::u32string result;
  for (auto it = begin; it != end; it++) {
    char32_t c = *it;
    if (c == end_char) {
      return result;
    } else {
      result.push_back(c);
    }
  }

  // if we had hit the end char, we wouldn't be here
  return boost::none;
}


RegexQuery parse_netspeak_regex_query(const std::string& netspeak_query) {
  RegexQueryBuilder builder;

  // while input and output strings are UTF-8, the implementation will work
  // exclusively with UTF-32 strings, so each code point is one character

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  std::u32string query = conv.from_bytes(netspeak_query);

  for (std::u32string::const_iterator it = query.begin(); it != query.end();
       ++it) {
    char32_t c = *it;
    switch (c) {
      case Syntax::QMARK:
        builder.add(RegexUnit::qmark());
        break;
      case Syntax::STAR:
        builder.add(RegexUnit::star());
        break;
      case Syntax::PLUS:
        builder.add(RegexUnit::qmark());
        builder.add(RegexUnit::star());
        break;

      case '.':
        // >= 2 dots is equal to "*" but a single dot is just a character
        if (*(it + 1) == '.') {
          builder.add(RegexUnit::star());
          while ((*(it + 1)) == '.') {
            ++it;
          }
        } else {
          builder.add(RegexUnit::word(U"."));
        }
        break;

      case Syntax::BRACKET_LEFT: {
        // e.g. "[u]" or "[aeiou]"
        auto result =
            read_until_char(it + 1, query.end(), Syntax::BRACKET_RIGHT);
        if (result) {
          it += result->size() + 1; // result + the end characters
          // "[u]" will be translated to /u?/ (optional) while "[aeiou]" will
          // be translated to /[aeiou]/ (non-optional). The actual UTF-8 pattern
          // might be different but will behave identical to the above UTF-32
          // patterns.
          auto value = *result;
          if (value.size() == 1) {
            builder.add(RegexUnit::optional_word(value));
          } else {
            builder.add(RegexUnit::char_set(value));
          }
        } else {
          // not valid syntax, so just append the character
          builder.add(RegexUnit::word(std::u32string(1, c)));
        }
        break;
      }

      case Syntax::BRACE_LEFT: {
        // E.g. "{form}" will be translated to /[form][form][form][form]/ (or
        // equivalent).
        auto result = read_until_char(it + 1, query.end(), Syntax::BRACE_RIGHT);
        if (result) {
          const auto& result_chars = *result;
          it += result_chars.size() + 1; // result + the end characters
          auto char_set = RegexUnit::char_set(result_chars);
          for (auto i = 0; i < result->size(); i++) {
            builder.add(char_set);
          }
        } else {
          // not valid syntax, so just append the character
          builder.add(RegexUnit::word(std::u32string(1, c)));
        }
        break;
      }

      default:
        builder.add(RegexUnit::word(std::u32string(1, c)));
        break;
    }
  }

  return builder.to_query();
}


} // namespace regex
} // namespace netspeak
