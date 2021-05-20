#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include "antlr4/parse.hpp"

#include "netspeak/error.hpp"
#include "netspeak/model/Query.hpp"


namespace netspeak {
namespace regex {

std::string remove_spaces(const std::string& str) {
  std::string new_str;
  for (const auto c : str) {
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      // skip the space character
    } else {
      new_str.push_back(c);
    }
  }
  return new_str;
}

std::string parse(const std::string& query_str) {
  const auto query = antlr4::parse_query(query_str);

  std::stringstream out;
  out << *query;
  return out.str();
}

// The syntax for this test suite is brought to you by the C preprocessor.
// https://gcc.gnu.org/onlinedocs/gcc-4.8.5/cpp/Stringification.html
#define ASSERT_QUERY(query, expected)                                         \
  do {                                                                        \
    BOOST_CHECK_EQUAL(remove_spaces(parse(query)), remove_spaces(#expected)); \
  } while (0)
#define ASSERT_INCORRECT_QUERY(query)                               \
  do {                                                              \
    BOOST_CHECK_THROW(parse(query), netspeak::invalid_query_error); \
  } while (0)


BOOST_AUTO_TEST_SUITE(test_parse)

BOOST_AUTO_TEST_CASE(test_parse_query) {
  // simple
  ASSERT_QUERY("", Query(Alternation{ Concat{} }));
  ASSERT_QUERY("   ", Query(Alternation{ Concat{} }));
  ASSERT_QUERY("abc", Query(Alternation{ Concat{ Word("abc") } }));
  ASSERT_QUERY("abc def",
               Query(Alternation{ Concat{ Word("abc"), Word("def") } }));
  ASSERT_QUERY("  abc   Def  ",
               Query(Alternation{ Concat{ Word("abc"), Word("Def") } }));

  // wild cards
  ASSERT_QUERY("abc ? def",
               Query(Alternation{ Concat{ Word("abc"), QMark, Word("def") } }));
  ASSERT_QUERY(
      "abc ? def *",
      Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Star } }));
  ASSERT_QUERY("abc ? def * * *",
               Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Star,
                                          Star, Star } }));
  ASSERT_QUERY(
      "abc ? def +",
      Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Plus } }));
  ASSERT_QUERY(
      "abc ? def ..",
      Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Star } }));
  ASSERT_QUERY(
      "abc ? def ...",
      Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Star } }));
  ASSERT_QUERY(
      "abc ? def ....",
      Query(Alternation{ Concat{ Word("abc"), QMark, Word("def"), Star } }));

  // option set
  ASSERT_QUERY("abc [ ]",
               Query(Alternation{ Concat{ Word("abc"), OptionSet{} } }));
  ASSERT_QUERY(
      "abc [ foo ]",
      Query(Alternation{ Concat{ Word("abc"), OptionSet{ Word("foo") } } }));
  ASSERT_QUERY("abc [ foo bar ]",
               Query(Alternation{ Concat{
                   Word("abc"), OptionSet{ Word("foo"), Word("bar") } } }));
  ASSERT_QUERY(
      "abc [ foo \"bar baz\" ]",
      Query(Alternation{ Concat{
          Word("abc"),
          OptionSet{ Word("foo"), Concat{ Word("bar"), Word("baz") } } } }));
  ASSERT_INCORRECT_QUERY("abc { foo ? }");

  // order set
  ASSERT_QUERY("abc { }",
               Query(Alternation{ Concat{ Word("abc"), OrderSet{} } }));
  ASSERT_QUERY(
      "abc { foo }",
      Query(Alternation{ Concat{ Word("abc"), OrderSet{ Word("foo") } } }));
  ASSERT_QUERY("abc { foo bar }",
               Query(Alternation{ Concat{
                   Word("abc"), OrderSet{ Word("foo"), Word("bar") } } }));
  ASSERT_QUERY(
      "abc { foo \"bar baz\" }",
      Query(Alternation{ Concat{
          Word("abc"),
          OrderSet{ Word("foo"), Concat{ Word("bar"), Word("baz") } } } }));
  ASSERT_INCORRECT_QUERY("abc { foo ? }");

  // alternation set
  ASSERT_QUERY("abc | def", Query(Alternation{ Concat{ Word("abc") },
                                               Concat{ Word("def") } }));
  ASSERT_QUERY("abc * | * def *",
               Query(Alternation{ Concat{ Word("abc"), Star },
                                  Concat{ Star, Word("def"), Star } }));

  // dictionary
  ASSERT_QUERY("abc #def",
               Query(Alternation{ Concat{ Word("abc"), DictSet("def") } }));
  ASSERT_INCORRECT_QUERY("abc #");
  ASSERT_INCORRECT_QUERY("abc #?");
  ASSERT_INCORRECT_QUERY("abc #+");
  ASSERT_INCORRECT_QUERY("abc #r#");
  ASSERT_INCORRECT_QUERY("abc #*");
  ASSERT_INCORRECT_QUERY("abc #r*");

  // regex
  ASSERT_QUERY("**", Query(Alternation{ Concat{ Regex("**") } }));
  ASSERT_QUERY("*?+...", Query(Alternation{ Concat{ Regex("*?+...") } }));
  ASSERT_QUERY("abc? ?def..",
               Query(Alternation{ Concat{ Regex("abc?"), Regex("?def..") } }));
  ASSERT_QUERY("[ abc? ?def.. ]", Query(Alternation{ Concat{ OptionSet{
                                      Regex("abc?"), Regex("?def..") } } }));
  ASSERT_QUERY("{ abc? ?def.. }", Query(Alternation{ Concat{ OrderSet{
                                      Regex("abc?"), Regex("?def..") } } }));
}

BOOST_AUTO_TEST_CASE(test_too_long_query) {
  ASSERT_INCORRECT_QUERY(std::string(4000, 'a'));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace regex
} // namespace netspeak
