#include <iostream>

#include <boost/test/unit_test.hpp>

#include "paths.hpp"

#include "netspeak/regex/DefaultRegexIndex.hpp"
#include "netspeak/regex/RegexQuery.hpp"
#include "netspeak/regex/parsers.hpp"


namespace netspeak {
namespace regex {

std::string query_to_string(const RegexQuery& query) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  std::string res;

  for (const auto& unit : query.get_units()) {
    switch (unit.type) {
      case RegexUnit::Type::QMARK:
        res.append("{qmark}");
        break;
      case RegexUnit::Type::STAR:
        res.append("{star}");
        break;

      case RegexUnit::Type::CHAR_SET:
        res.append("{char_set:'");
        res.append(conv.to_bytes(unit.value));
        res.append("'}");
        break;

      case RegexUnit::Type::OPTIONAL_WORD:
        res.append("{optional_word:'");
        res.append(conv.to_bytes(unit.value));
        res.append("'}");
        break;

      case RegexUnit::Type::WORD:
        res.append("{word:'");
        res.append(conv.to_bytes(unit.value));
        res.append("'}");
        break;

      default:
        res.append("{UNKNOWN}");
        break;
    }
  }

  return res;
}

std::string netspeak_query(std::string netspeak_query) {
  return query_to_string(parse_netspeak_regex_query(netspeak_query));
}

BOOST_AUTO_TEST_SUITE(test_regex)

BOOST_AUTO_TEST_CASE(test_regex_query_parser) {
  // simple words
  BOOST_REQUIRE_EQUAL(netspeak_query(""), "");
  BOOST_REQUIRE_EQUAL(netspeak_query("abc"), "{word:'abc'}");

  // qmark
  BOOST_REQUIRE_EQUAL(netspeak_query("?"), "{qmark}");
  BOOST_REQUIRE_EQUAL(netspeak_query("???"), "{qmark}{qmark}{qmark}");

  // qmark, star, and plus
  BOOST_REQUIRE_EQUAL(netspeak_query("?*"), "{qmark}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("*?"), "{qmark}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("*?*"), "{qmark}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("***?****"), "{qmark}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("***+***"), "{qmark}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("+"), "{qmark}{star}");

  // alternative syntax for star
  BOOST_REQUIRE_EQUAL(netspeak_query("."), "{word:'.'}");
  BOOST_REQUIRE_EQUAL(netspeak_query(".."), "{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("..b"), "{star}{word:'b'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a.."), "{word:'a'}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a..b"), "{word:'a'}{star}{word:'b'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("..."), "{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("...b"), "{star}{word:'b'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a..."), "{word:'a'}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a...b"), "{word:'a'}{star}{word:'b'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("......."), "{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query(".......b"), "{star}{word:'b'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a......."), "{word:'a'}{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a.......b"),
                      "{word:'a'}{star}{word:'b'}");

  // char set
  BOOST_REQUIRE_EQUAL(netspeak_query("[ab]"), "{char_set:'ab'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("[aaabbbccc]"), "{char_set:'abc'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a[bbb]c"), "{word:'abc'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("a[]c"), "{char_set:''}");

  // order set
  BOOST_REQUIRE_EQUAL(netspeak_query("{abc}"),
                      "{char_set:'abc'}{char_set:'abc'}{char_set:'abc'}");
  BOOST_REQUIRE_EQUAL(
      netspeak_query("{abba}"),
      "{char_set:'ab'}{char_set:'ab'}{char_set:'ab'}{char_set:'ab'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("{aaaa}"), "{word:'aaaa'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("{a}"), "{word:'a'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("{}"), "");

  // optional
  BOOST_REQUIRE_EQUAL(netspeak_query("colo[u]r"),
                      "{word:'colo'}{optional_word:'u'}{word:'r'}");

  // star absorbs optionals
  BOOST_REQUIRE_EQUAL(netspeak_query("*[a]"), "{star}");
  BOOST_REQUIRE_EQUAL(netspeak_query("[a]?[b]?[c]?*"),
                      "{qmark}{qmark}{qmark}{star}");
}

BOOST_AUTO_TEST_CASE(test_regex_query_parser_with_invalid_queries) {
  BOOST_REQUIRE_EQUAL(netspeak_query("["), "{word:'['}");
  BOOST_REQUIRE_EQUAL(netspeak_query("[{"), "{word:'[{'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("{"), "{word:'{'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("]}"), "{word:']}'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("]"), "{word:']'}");
  BOOST_REQUIRE_EQUAL(netspeak_query("}"), "{word:'}'}");
}

BOOST_AUTO_TEST_CASE(test_regex_query_accept_all) {
  BOOST_REQUIRE(parse_netspeak_regex_query("*").accept_all());
  BOOST_REQUIRE(parse_netspeak_regex_query("***").accept_all());
  BOOST_REQUIRE(parse_netspeak_regex_query("[a]***[b]").accept_all());
}

BOOST_AUTO_TEST_CASE(test_regex_query_reject_all) {
  BOOST_REQUIRE(parse_netspeak_regex_query("[]").reject_all());
  BOOST_REQUIRE(parse_netspeak_regex_query("a[]").reject_all());
  BOOST_REQUIRE(parse_netspeak_regex_query("*[]*").reject_all());
}


std::vector<std::string> matches(const DefaultRegexIndex& index,
                                 const std::string& netspeak_query,
                                 uint32_t max_matches) {
  std::vector<std::string> matches;
  auto query = parse_netspeak_regex_query(netspeak_query);
  index.match_query(query, matches, max_matches, std::chrono::seconds(5));
  return matches;
}

BOOST_AUTO_TEST_CASE(test_default_regex_index) {
  // 1k lowercase words
  auto vocabulary = test::load_file(test::REGEX_1K_LOWER);
  const DefaultRegexIndex index(vocabulary);

  auto test = [&](std::string query, const std::vector<std::string>& expected) {
    std::vector<std::string> actual = matches(index, query, 10);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                    expected.begin(), expected.end());
  };

  {
    std::vector<std::string> e{};
    test("", e);
  }
  {
    std::vector<std::string> e{ ",", "the", "of",  "and", "to",
                                "a", "in",  "for", "is",  "on" };
    test("*", e);
  }
  {
    std::vector<std::string> e{ "for", "far" };
    test("f?r", e);
  }
  {
    std::vector<std::string> e{ "love" };
    test("love", e);
  }
  {
    std::vector<std::string> e{ "and", "as",    "at",  "are",  "all",
                                "an",  "about", "any", "also", "am" };
    test("a+", e);
  }
  {
    std::vector<std::string> e{ "add", "address", "additional", "added",
                                "addition" };
    test("*add*", e);
  }
  {
    std::vector<std::string> e{ "get", "set", "yet", "let" };
    test("[bglsy]et", e); // "bet" is not in the vocabulary
  }
  {
    std::vector<std::string> e{ "from", "form" };
    test("f{or}m", e);
  }
  {
    std::vector<std::string> e{ "i", "ii" };
    // "i" can be constructed in two different ways (enabling either the first
    // or the seconde [i] while disabling the other) but is only present once in
    // the result. This has to be tested because the implementation uses a hash
    // table approach.
    test("[i][i]", e); // "bet" is not in the vocabulary
  }
  {
    std::vector<std::string> e{};
    test("The", e); // the vocabulary is lowercase
  }
}

BOOST_AUTO_TEST_CASE(test_default_regex_index_unicode) {
  DefaultRegexIndex index("for\nfür\nfar\nform");

  auto test = [&](std::string query, const std::vector<std::string>& expected) {
    std::vector<std::string> actual = matches(index, query, 10);
    BOOST_REQUIRE_EQUAL_COLLECTIONS(actual.begin(), actual.end(),
                                    expected.begin(), expected.end());
  };

  {
    std::vector<std::string> e{ "for", "für", "far" };
    test("f?r", e);
  }
}

uint32_t get_combinations(std::string netspeak_regex_query) {
  return parse_netspeak_regex_query(netspeak_regex_query)
      .combinations_upper_bound();
}
BOOST_AUTO_TEST_CASE(test_regex_query_combinations) {
  BOOST_REQUIRE_EQUAL(get_combinations("*[]*"), 0);
  BOOST_REQUIRE_EQUAL(get_combinations("foo"), 1);
  BOOST_REQUIRE_EQUAL(get_combinations("fo[o]"), 2);
  BOOST_REQUIRE_EQUAL(get_combinations("[bgps]et"), 4);
  BOOST_REQUIRE_EQUAL(get_combinations("f{orm}"), 27);
  BOOST_REQUIRE_EQUAL(get_combinations("{abcd}"), 256);
  BOOST_REQUIRE_EQUAL(get_combinations("{abcdef}"), 46656);
  BOOST_REQUIRE_EQUAL(get_combinations("{abcdefgh}"), 16777216);
  // overflow will round down to UINT32_MAX
  BOOST_REQUIRE_EQUAL(get_combinations("{abcdefghij}"), UINT32_MAX);

  // any qmark or star will result in UINT32_MAX
  BOOST_REQUIRE_EQUAL(get_combinations("foo?"), UINT32_MAX);
  BOOST_REQUIRE_EQUAL(get_combinations("foo*"), UINT32_MAX);
  BOOST_REQUIRE_EQUAL(get_combinations("foo+"), UINT32_MAX);
}

size_t get_max_input_length(std::string netspeak_regex_query) {
  return parse_netspeak_regex_query(netspeak_regex_query)
      .max_utf8_input_length();
}
BOOST_AUTO_TEST_CASE(test_regex_query_max_input_length) {
  BOOST_REQUIRE_EQUAL(get_max_input_length(""), 0);
  BOOST_REQUIRE_EQUAL(get_max_input_length("foo"), 3);
  BOOST_REQUIRE_EQUAL(get_max_input_length("ä"), 2);
  BOOST_REQUIRE_EQUAL(get_max_input_length("fo[o]"), 3);
  BOOST_REQUIRE_EQUAL(get_max_input_length("[bgps]et"), 3);
  BOOST_REQUIRE_EQUAL(get_max_input_length("[aä]"), 2);
  BOOST_REQUIRE_EQUAL(get_max_input_length("f{orm}"), 4);
  BOOST_REQUIRE_EQUAL(get_max_input_length("?"), 4);
  BOOST_REQUIRE_EQUAL(get_max_input_length("???d"), 13);
  BOOST_REQUIRE_EQUAL(get_max_input_length("*d*"), SIZE_MAX);
  BOOST_REQUIRE_EQUAL(get_max_input_length("++b[a]"), SIZE_MAX);
}

size_t get_min_input_length(std::string netspeak_regex_query) {
  return parse_netspeak_regex_query(netspeak_regex_query)
      .min_utf8_input_length();
}
BOOST_AUTO_TEST_CASE(test_regex_query_min_input_length) {
  BOOST_REQUIRE_EQUAL(get_min_input_length(""), 0);
  BOOST_REQUIRE_EQUAL(get_min_input_length("foo"), 3);
  BOOST_REQUIRE_EQUAL(get_min_input_length("ä"), 2);
  BOOST_REQUIRE_EQUAL(get_min_input_length("fo[o]"), 2);
  BOOST_REQUIRE_EQUAL(get_min_input_length("[bgps]et"), 3);
  BOOST_REQUIRE_EQUAL(get_min_input_length("[aä]"), 1);
  BOOST_REQUIRE_EQUAL(get_min_input_length("f{orm}"), 4);
  BOOST_REQUIRE_EQUAL(get_min_input_length("?"), 1);
  BOOST_REQUIRE_EQUAL(get_min_input_length("???d"), 4);
  BOOST_REQUIRE_EQUAL(get_min_input_length("*d*"), 1);
  BOOST_REQUIRE_EQUAL(get_min_input_length("++b[a]"), 3);
  BOOST_REQUIRE_EQUAL(get_min_input_length("*[]*"), SIZE_MAX);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace regex
} // namespace netspeak
