
#include <memory>

#include <boost/test/unit_test.hpp>

#include "paths.hpp"

#include "antlr4/parse.hpp"

#include "netspeak/Dictionaries.hpp"
#include "netspeak/QueryNormalizer.hpp"
#include "netspeak/error.hpp"
#include "netspeak/model/NormQuery.hpp"
#include "netspeak/model/Query.hpp"
#include "netspeak/regex/DefaultRegexIndex.hpp"


using namespace netspeak;
using namespace netspeak::model;

BOOST_AUTO_TEST_SUITE(netspeak_normalization)

std::shared_ptr<regex::DefaultRegexIndex> _cached_regex_index;
std::shared_ptr<regex::DefaultRegexIndex> get_regex_index() {
  if (!_cached_regex_index) {
    auto regex_words = test::load_file(test::TEST_REGEX_DATA);
    _cached_regex_index =
        std::make_shared<regex::DefaultRegexIndex>(regex_words);
  }
  return _cached_regex_index;
}

std::shared_ptr<Dictionaries::Map> _cached_dictionary;
std::shared_ptr<Dictionaries::Map> get_dictionary() {
  if (!_cached_dictionary) {
    const auto dict = Dictionaries::read_from_file(test::DICTIONARY_SET);
    _cached_dictionary = std::make_shared<Dictionaries::Map>();
    for (const auto& pair : dict) {
      _cached_dictionary->insert(pair);
    }
  }
  return _cached_dictionary;
}

const QueryNormalizer::InitConfig DEFAULT_INIT = {
  .regex_index = get_regex_index(),
  .dictionary = get_dictionary(),
};
const QueryNormalizer::Options DEFAULT_OPTIONS = {
  .max_norm_queries = 1000,
  .min_length = 1,
  .max_length = 5,
  .max_regex_matches = 10,
  .max_regex_time = std::chrono::milliseconds(30),
};

std::vector<std::string> stringify_norm_queries(
    const std::vector<NormQuery>& norm_queries) {
  std::vector<std::string> result;
  for (const auto& query : norm_queries) {
    std::string str;
    for (const auto& unit : query.units()) {
      if (!str.empty()) {
        str.push_back(' ');
      }
      if (unit.tag() == NormQuery::Unit::Tag::QMARK) {
        str.push_back('?');
      } else {
        str.append(*unit.text());
      }
    }
    result.push_back(std::move(str));
  }
  return result;
}

#define NORMALIZE_QUERY(query_str, expected_norm_queries, init_config, \
                        options)                                       \
  do {                                                                 \
    const auto _query = antlr4::parse_query(query_str);                \
    std::vector<NormQuery> _norm_queries;                              \
                                                                       \
    QueryNormalizer _qn(init_config);                                  \
    _qn.normalize(_query, options, _norm_queries);                     \
    auto _str_norm_queries = stringify_norm_queries(_norm_queries);    \
                                                                       \
    BOOST_CHECK_EQUAL_COLLECTIONS(                                     \
        _str_norm_queries.begin(), _str_norm_queries.end(),            \
        expected_norm_queries.begin(), expected_norm_queries.end());   \
  } while (0)


// In the following lists, clang-format will generally try to make them more
// compact which doesn't make sense here.

// clang-format off

BOOST_AUTO_TEST_CASE(test_normalize_qmark) {
  {
    const auto query = "test ?";
    std::vector<std::string> expected{
      "test ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "? test ? ?";
    std::vector<std::string> expected{
      "? test ? ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_asterisk) {
  {
    const auto query = "test *";
    std::vector<std::string> expected{
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
      "test ?",
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test * * ...";
    std::vector<std::string> expected{
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
      "test ?",
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "* test";
    std::vector<std::string> expected{
      "? ? ? ? test",
      "? ? ? test",
      "? ? test",
      "? test",
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "* test *";
    std::vector<std::string> expected{
      "? ? ? ? test",
      "? ? ? test ?",
      "? ? ? test",
      "? ? test ? ?",
      "? ? test ?",
      "? ? test",
      "? test ? ? ?",
      "? test ? ?",
      "? test ?",
      "? test",
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
      "test ?",
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "foo * bar";
    std::vector<std::string> expected{
      "foo ? ? ? bar",
      "foo ? ? bar",
      "foo ? bar",
      "foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "foo * bar *";
    std::vector<std::string> expected{
      "foo ? ? ? bar",
      "foo ? ? bar ?",
      "foo ? ? bar",
      "foo ? bar ? ?",
      "foo ? bar ?",
      "foo ? bar",
      "foo bar ? ? ?",
      "foo bar ? ?",
      "foo bar ?",
      "foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "* foo * bar";
    std::vector<std::string> expected{
      "? ? ? foo bar",
      "? ? foo ? bar",
      "? ? foo bar",
      "? foo ? ? bar",
      "? foo ? bar",
      "? foo bar",
      "foo ? ? ? bar",
      "foo ? ? bar",
      "foo ? bar",
      "foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "* foo * bar *";
    std::vector<std::string> expected{
      "? ? ? foo bar",
      "? ? foo ? bar",
      "? ? foo bar ?",
      "? ? foo bar",
      "? foo ? ? bar",
      "? foo ? bar ?",
      "? foo ? bar",
      "? foo bar ? ?",
      "? foo bar ?",
      "? foo bar",
      "foo ? ? ? bar",
      "foo ? ? bar ?",
      "foo ? ? bar",
      "foo ? bar ? ?",
      "foo ? bar ?",
      "foo ? bar",
      "foo bar ? ? ?",
      "foo bar ? ?",
      "foo bar ?",
      "foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "foo * bar * xyz";
    std::vector<std::string> expected{
      "foo ? ? bar xyz",
      "foo ? bar ? xyz",
      "foo ? bar xyz",
      "foo bar ? ? xyz",
      "foo bar ? xyz",
      "foo bar xyz",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "* a * b * c * d * e *";
    std::vector<std::string> expected{
      "a b c d e",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_plus) {
  {
    const auto query = "test +";
    std::vector<std::string> expected{
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
      "test ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test + + *";
    std::vector<std::string> expected{
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_optionset) {
  {
    const auto query = "test [ ]";
    std::vector<std::string> expected{
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test [ foo ]";
    std::vector<std::string> expected{
      "test",
      "test foo",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test [ \"foo bar\" ]";
    std::vector<std::string> expected{
      "test",
      "test foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test [ foo bar ]";
    std::vector<std::string> expected{
      "test foo",
      "test bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test [ \"foo bar\" baz ]";
    std::vector<std::string> expected{
      "test foo bar",
      "test baz",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test [ foo bar baz ]";
    std::vector<std::string> expected{
      "test foo",
      "test bar",
      "test baz",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    // the star should absorb the optional option set
    const auto query = "test [ foo ] ? *";
    std::vector<std::string> expected{
      "test ? ? ? ?",
      "test ? ? ?",
      "test ? ?",
      "test ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_orderset) {
  {
    const auto query = "test { }";
    std::vector<std::string> expected{
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test { foo }";
    std::vector<std::string> expected{
      "test foo",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test { \"foo bar\" }";
    std::vector<std::string> expected{
      "test foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test { foo bar }";
    std::vector<std::string> expected{
      "test foo bar",
      "test bar foo",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test { \"foo bar\" baz }";
    std::vector<std::string> expected{
      "test foo bar baz",
      "test baz foo bar",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "test { foo bar baz }";
    std::vector<std::string> expected{
      "test foo bar baz",
      "test foo baz bar",
      "test bar foo baz",
      "test bar baz foo",
      "test baz foo bar",
      "test baz bar foo",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_dictset) {
  {
    const auto query = "#test";
    std::vector<std::string> expected{
      "test",
      "screen",
      "examination",
      "prove",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "#unknown_word";
    std::vector<std::string> expected{
      "unknown_word",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_regex) {
  {
    const auto query = "t* ?";
    std::vector<std::string> expected{
      "test ?",
      "tester ?",
      "total ?",
      "toronto ?",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "t?st";
    std::vector<std::string> expected{
      "test",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "te*";
    std::vector<std::string> expected{
      "test",
      "tester",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "*er";
    std::vector<std::string> expected{
      "tester",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "**";
    std::vector<std::string> expected{
      "test",
      "tester",
      "total",
      "toronto",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "{ te* foo }";
    std::vector<std::string> expected{
      "test foo",
      "tester foo",
      "foo test",
      "foo tester",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "[ te* *o* ]";
    std::vector<std::string> expected{
      "test",
      "tester",
      "total",
      "toronto",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "foo [ *o* ]";
    std::vector<std::string> expected{
      "foo",
      "foo total",
      "foo toronto",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}

BOOST_AUTO_TEST_CASE(test_normalize_split) {
  {
    const auto query = "foo | bar | baz";
    std::vector<std::string> expected{
      "foo",
      "bar",
      "baz",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
  {
    const auto query = "foo | [ bar baz ]";
    std::vector<std::string> expected{
      "foo",
      "bar",
      "baz",
    };
    NORMALIZE_QUERY(query, expected, DEFAULT_INIT, DEFAULT_OPTIONS);
  }
}


BOOST_AUTO_TEST_SUITE_END()
