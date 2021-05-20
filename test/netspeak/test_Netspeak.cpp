#include <iostream>

#include <boost/test/unit_test.hpp>

#include "paths.hpp"

#include "netspeak/Netspeak.hpp"
#include "netspeak/service/NetspeakService.pb.h"

namespace netspeak {

/* In terms of unit testing, objects that are responsible for global setup and
 * teardown operations are called fixtures. With Boost.Test you can define
 * fixtures as structs/classes with an constructor/destructor.
 *
 * http://www.boost.org/doc/libs/1_40_0/libs/test/doc/html/utf/user-guide/fixture/test-suite-shared.html
 */

static Netspeak netspeak;

service::SearchResponse::Result search(const service::SearchRequest& request) {
  service::SearchResponse response;
  netspeak.search(request, response);
  BOOST_REQUIRE(response.has_result());
  return response.result();
}

struct netspeak_service_fixture {
  netspeak_service_fixture() {
    BOOST_TEST_MESSAGE("netspeak_service_fixture setup");
    const Configuration config = {
      { Configuration::PATH_TO_HOME, test::INDEX_DIR },
      { Configuration::CACHE_CAPACITY, "1000" },
    };
    try {
      netspeak.initialize(config);
    } catch (std::exception& error) {
      std::cerr << "Some error occured: " << error.what() << std::endl;
    }
  }

  ~netspeak_service_fixture() {
    BOOST_TEST_MESSAGE("netspeak_service_fixture teardown");
  }
};

BOOST_GLOBAL_FIXTURE(netspeak_service_fixture);

BOOST_AUTO_TEST_SUITE(netspeak_service)

BOOST_AUTO_TEST_CASE(test_search_with_valid_queries) {
  const std::vector<std::string> test_cases = {
    "* ? {the world}",
    "+ {the world}",
    "[but besides]*",
    "* what *",
    "i love",
    "foo",
    "jr.",
    "foo | bar"
    // Add more valid queries here ...
  };

  uint32_t max_phrases = 100;

  for (const auto& query : test_cases) {
    service::SearchRequest request;
    request.set_query(query);
    request.set_max_phrases(max_phrases);

    const auto result = search(request);

    // just make sure that some phrases are there
    BOOST_CHECK_GE(result.phrases_size(), 0);
    BOOST_CHECK_LE(result.phrases_size(), max_phrases);
  }
}

BOOST_AUTO_TEST_CASE(test_search_with_invalid_queries) {
  const std::vector<std::string> test_cases = {
    "this is invalid [",      "this is invalid ]",
    "this is invalid {",      "this is invalid }",
    "this is invalid #",      "this is invalid [ # ]",
    "this is invalid [ ? ]",  "this is invalid [ * ]",
    "this is invalid { # }",  "this is invalid { ? }",
    "this is invalid { * }",  "this is invalid { [ ] }",
    "this is invalid [ { } ]"
    // Add more invalid queries here ...
  };

  service::SearchRequest request;
  request.set_max_phrases(100);

  for (const auto& query : test_cases) {
    request.set_query(query);

    service::SearchResponse response;
    netspeak.search(request, response);

    BOOST_REQUIRE(response.has_error());
    const auto& error = response.error();
    BOOST_REQUIRE_EQUAL(error.kind(),
                        service::SearchResponse::Error::INVALID_QUERY);
    BOOST_REQUIRE(!error.message().empty());
  }

  // TODO: Figure out whether this should be the correct behavior.
  /*request.set_query("this query is too long now"); // which is indeed no error
  std::shared_ptr<generated::Response> response = netspeak.search(request);
  BOOST_REQUIRE_EQUAL(response->error_code(),
                      to_ordinal(error_code::invalid_query));
  BOOST_REQUIRE(!(response->error_message().empty()));*/
}

// Note: The list of words in Response::unknown_words is always sorted
// alphabetically and not by the order of occurrence in the query.
BOOST_AUTO_TEST_CASE(test_search_with_unknown_word) {
  service::SearchRequest request;
  request.set_max_phrases(100);
  {
    request.set_query("_this-word-is-unknown_");
    const auto result = search(request);
    BOOST_CHECK_EQUAL(result.phrases_size(), 0);

    BOOST_CHECK_EQUAL(result.unknown_words_size(), 1);
    BOOST_CHECK_EQUAL(result.unknown_words(0), "_this-word-is-unknown_");
  }
  {
    request.set_query("_this-two-gram_ _is-unknown_");
    const auto result = search(request);
    BOOST_CHECK_EQUAL(result.phrases_size(), 0);

    BOOST_CHECK_EQUAL(result.unknown_words_size(), 2);
    BOOST_CHECK_EQUAL(result.unknown_words(0), "_is-unknown_");
    BOOST_CHECK_EQUAL(result.unknown_words(1), "_this-two-gram_");
  }
  {
    request.set_query("_this-is_ _also-unknown_ ? *");
    const auto result = search(request);
    BOOST_CHECK_EQUAL(result.phrases_size(), 0);

    // TODO: Is this really expected behavior?
    BOOST_CHECK_EQUAL(result.unknown_words_size(), 1);
    BOOST_CHECK_EQUAL(result.unknown_words(0), "_this-is_");
  }
  {
    request.set_query("hello _also-unknown_ ? *");
    const auto result = search(request);
    BOOST_CHECK_EQUAL(result.phrases_size(), 0);

    BOOST_CHECK_EQUAL(result.unknown_words_size(), 1);
    BOOST_CHECK_EQUAL(result.unknown_words(0), "_also-unknown_");
  }
}

BOOST_AUTO_TEST_CASE(test_properties) {
  const auto properties = netspeak.properties();
  BOOST_REQUIRE(!properties.empty());
  for (auto it = properties.begin(); it != properties.end(); ++it) {
    if (it->first == netspeak::Properties::cache_top_100)
      continue;
    BOOST_REQUIRE(!it->second.empty());
  }
}

// some test util
struct SimplePhrase {
  std::string text;
  uint64_t frequency;

  SimplePhrase(const std::string& text, uint64_t freq)
      : text(text), frequency(freq) {}

  bool operator==(const SimplePhrase& rhs) const {
    return frequency == rhs.frequency && text == rhs.text;
  }
  inline bool operator!=(const SimplePhrase& rhs) const {
    return !(*this == rhs);
  }
};
std::ostream& operator<<(std::ostream& out, const SimplePhrase& phrase) {
  return out << "SimplePhrase(\"" << phrase.text << "\", " << phrase.frequency
             << ")";
}
std::vector<SimplePhrase> to_simple(
    const service::SearchResponse::Result& result) {
  std::vector<SimplePhrase> simple;
  for (const auto& phrase : result.phrases()) {
    std::string text;
    for (const auto& word : phrase.words()) {
      if (!text.empty()) {
        text.push_back(' ');
      }
      text.append(word.text());
    }
    simple.push_back({ text, phrase.frequency() });
  }
  return simple;
}
#define CHECK_RESULT_PHRASES(result, expected)                       \
  do {                                                               \
    const auto actual = to_simple(result);                           \
    BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(),      \
                                  expected.begin(), expected.end()); \
  } while (0)
#define CHECK_PHRASES(request, expected)           \
  do {                                             \
    const auto result = search(request);           \
    BOOST_CHECK(result.unknown_words_size() == 0); \
    CHECK_RESULT_PHRASES(result, expected);        \
  } while (0)


BOOST_AUTO_TEST_CASE(test_search_with_orderset) {
  service::SearchRequest request;
  request.set_max_phrases(100);
  request.set_query("{ the of life }");

  std::vector<SimplePhrase> expected{
    SimplePhrase("the life of", 16535151),
    SimplePhrase("life of the", 9469316),
    SimplePhrase("of the life", 3507490),
  };

  CHECK_PHRASES(request, expected);
}

BOOST_AUTO_TEST_CASE(test_search_with_optionset) {
  service::SearchRequest request;
  request.set_max_phrases(100);
  request.set_query("so [ good beautiful ]");

  std::vector<SimplePhrase> expected{
    SimplePhrase("so good", 5603814),
    SimplePhrase("so beautiful", 1471556),
  };

  CHECK_PHRASES(request, expected);
}

BOOST_AUTO_TEST_CASE(test_search_with_maxfreq) {
  const uint32_t max_phrases = 10;

  service::SearchRequest request;
  request.set_max_phrases(max_phrases);
  request.set_query("the *");

  BOOST_TEST_CHECKPOINT("First request");
  std::vector<SimplePhrase> expected{
    SimplePhrase("the", 43760129185),
    SimplePhrase("the same", 436080161),
    SimplePhrase("the first", 387093132),
    SimplePhrase("the world", 286492739),
    SimplePhrase("the most", 261699781),
    SimplePhrase("the other", 246143010),
    SimplePhrase("the following", 240678040),
    SimplePhrase("the new", 209513100),
    SimplePhrase("the best", 189003982),
    SimplePhrase("the time", 182107517),
  };
  CHECK_PHRASES(request, expected);

  BOOST_TEST_CHECKPOINT("Second request");
  request.mutable_phrase_constraints()->set_frequency_max(240678040);
  expected = {
    SimplePhrase("the following", 240678040),
    SimplePhrase("the new", 209513100),
    SimplePhrase("the best", 189003982),
    SimplePhrase("the time", 182107517),
    SimplePhrase("the united", 155295162),
    SimplePhrase("the united states", 150747671),
    SimplePhrase("the state", 144601913),
    SimplePhrase("the last", 141335946),
    SimplePhrase("the end", 136911617),
    SimplePhrase("the right", 132405119),
  };
  CHECK_PHRASES(request, expected);

  BOOST_TEST_CHECKPOINT("Third request");
  request.mutable_phrase_constraints()->set_frequency_max(155295162);
  expected = {
    SimplePhrase("the united", 155295162),
    SimplePhrase("the united states", 150747671),
    SimplePhrase("the state", 144601913),
    SimplePhrase("the last", 141335946),
    SimplePhrase("the end", 136911617),
    SimplePhrase("the right", 132405119),
    SimplePhrase("the next", 131585065),
    SimplePhrase("the two", 130679329),
    SimplePhrase("the way", 124272940),
    SimplePhrase("the second", 121079153),
  };
  CHECK_PHRASES(request, expected);
}

/*BOOST_AUTO_TEST_CASE(test_search_with_phrase_tag_bug) {
  generated::Request request;
  request.set_query("waiting * #response");
  std::shared_ptr<generated::Response> response = netspeak.search(request);
  for (const auto& phrase : response->phrase()) {
    BOOST_REQUIRE_EQUAL(phrase.word(0).tag(), generated::Phrase::Word::WORD);
    for (int i = 1; i < phrase.word_size() - 1; ++i) {
      BOOST_REQUIRE_EQUAL(phrase.word(i).tag(),
                          generated::Phrase::Word::WORD_FOR_ASTERISK);
    }
    BOOST_REQUIRE_EQUAL(phrase.word(phrase.word_size() - 1).tag(),
                        generated::Phrase::Word::WORD_IN_DICTSET);
  }

  // This response comes partly from the cache
  request.set_query("waiting ? * #response");
  response = netspeak.search(request);
  for (const auto& phrase : response->phrase()) {
    BOOST_REQUIRE_EQUAL(phrase.word(0).tag(), generated::Phrase::Word::WORD);
    BOOST_REQUIRE_EQUAL(phrase.word(1).tag(),
                        generated::Phrase::Word::WORD_FOR_QMARK);
    for (int i = 2; i < phrase.word_size() - 1; ++i) {
      BOOST_REQUIRE_EQUAL(phrase.word(i).tag(),
                          generated::Phrase::Word::WORD_FOR_ASTERISK);
    }
    BOOST_REQUIRE_EQUAL(phrase.word(phrase.word_size() - 1).tag(),
                        generated::Phrase::Word::WORD_IN_DICTSET);
  }
}*/

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
