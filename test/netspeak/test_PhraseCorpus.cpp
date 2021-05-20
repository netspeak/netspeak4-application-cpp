#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include "paths.hpp"

#include "netspeak/PhraseCorpus.hpp"
#include "netspeak/model/Phrase.hpp"


namespace netspeak {

using namespace netspeak::model;
typedef Phrase::Id Id;

BOOST_AUTO_TEST_SUITE(test_PhraseCorpus)

const boost::filesystem::path phrase_files_dir(test::INDEX_DIR +
                                               "/phrase-corpus/bin");

// the test index is fixed, so we know these numbers
const int INDEX_VOCAB_COUNT = 61741;
const int INDEX_1GRAM_COUNT = 61738;
const int INDEX_2GRAM_COUNT = 154797;
const int INDEX_3GRAM_COUNT = 77687;
const int INDEX_4GRAM_COUNT = 20569;
const int INDEX_5GRAM_COUNT = 7271;

Phrase read_phrase(PhraseCorpus& corpus, Id id) {
  std::vector<Id> ids{ id };
  const auto phrases = corpus.read_phrases(ids);
  BOOST_REQUIRE_EQUAL(phrases.size(), 1);
  return phrases[0];
}


BOOST_AUTO_TEST_CASE(test_open) {
  PhraseCorpus corpus;

  BOOST_REQUIRE_THROW(corpus.open(phrase_files_dir / "#"), std::runtime_error);
  BOOST_REQUIRE_NO_THROW(corpus.open(phrase_files_dir));
}

BOOST_AUTO_TEST_CASE(test_count_phrases) {
  PhraseCorpus corpus(phrase_files_dir);

  // this is trivially true for all indexes
  BOOST_CHECK_EQUAL(0, corpus.count_phrases(0));

  // we removed all phrases with > 5 words
  BOOST_CHECK_EQUAL(0, corpus.count_phrases(6));
  BOOST_CHECK_EQUAL(0, corpus.count_phrases(7));
  BOOST_CHECK_EQUAL(0, corpus.count_phrases(8));

  // We just need to see that there are some phrases
  BOOST_CHECK_EQUAL(INDEX_1GRAM_COUNT, corpus.count_phrases(1));
  BOOST_CHECK_EQUAL(INDEX_2GRAM_COUNT, corpus.count_phrases(2));
  BOOST_CHECK_EQUAL(INDEX_3GRAM_COUNT, corpus.count_phrases(3));
  BOOST_CHECK_EQUAL(INDEX_4GRAM_COUNT, corpus.count_phrases(4));
  BOOST_CHECK_EQUAL(INDEX_5GRAM_COUNT, corpus.count_phrases(5));

  BOOST_CHECK_EQUAL(5, corpus.max_length());
}

BOOST_AUTO_TEST_CASE(test_count_vocabulary) {
  PhraseCorpus corpus(phrase_files_dir);

  BOOST_CHECK_EQUAL(INDEX_VOCAB_COUNT, corpus.count_vocabulary());
}

BOOST_AUTO_TEST_CASE(test_read_phrase) {
  PhraseCorpus corpus(phrase_files_dir);

  // invalid ngram_len
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(0, 0)), std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(6, 0)), std::runtime_error);

  // invalid ngram_id
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(1, -1)), std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(2, -1)), std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(3, -1)), std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(4, -1)), std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(5, -1)), std::runtime_error);

  // invalid ngram_id
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(1, INDEX_1GRAM_COUNT)),
                      std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(2, INDEX_2GRAM_COUNT)),
                      std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(3, INDEX_3GRAM_COUNT)),
                      std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(4, INDEX_4GRAM_COUNT)),
                      std::runtime_error);
  BOOST_REQUIRE_THROW(read_phrase(corpus, Id(5, INDEX_5GRAM_COUNT)),
                      std::runtime_error);

  // initialize this with some data, it doesn't matter
  Phrase phrase(Id(0, 0), 0);

  // valid ngram_len and ngram_id
  BOOST_REQUIRE_NO_THROW(phrase = read_phrase(corpus, Id(1, 0)));
  BOOST_REQUIRE_EQUAL(1, phrase.words().size());
  BOOST_REQUIRE_NO_THROW(phrase =
                             read_phrase(corpus, Id(1, INDEX_1GRAM_COUNT - 1)));
  BOOST_REQUIRE_EQUAL(1, phrase.words().size());

  BOOST_REQUIRE_NO_THROW(phrase = read_phrase(corpus, Id(2, 0)));
  BOOST_REQUIRE_EQUAL(2, phrase.words().size());
  BOOST_REQUIRE_NO_THROW(phrase =
                             read_phrase(corpus, Id(2, INDEX_2GRAM_COUNT - 1)));
  BOOST_REQUIRE_EQUAL(2, phrase.words().size());

  BOOST_REQUIRE_NO_THROW(phrase = read_phrase(corpus, Id(3, 0)));
  BOOST_REQUIRE_EQUAL(3, phrase.words().size());
  BOOST_REQUIRE_NO_THROW(phrase =
                             read_phrase(corpus, Id(3, INDEX_3GRAM_COUNT - 1)));
  BOOST_REQUIRE_EQUAL(3, phrase.words().size());

  BOOST_REQUIRE_NO_THROW(phrase = read_phrase(corpus, Id(4, 0)));
  BOOST_REQUIRE_EQUAL(4, phrase.words().size());
  BOOST_REQUIRE_NO_THROW(phrase =
                             read_phrase(corpus, Id(4, INDEX_4GRAM_COUNT - 1)));
  BOOST_REQUIRE_EQUAL(4, phrase.words().size());

  BOOST_REQUIRE_NO_THROW(phrase = read_phrase(corpus, Id(5, 0)));
  BOOST_REQUIRE_EQUAL(5, phrase.words().size());
  BOOST_REQUIRE_NO_THROW(phrase =
                             read_phrase(corpus, Id(5, INDEX_5GRAM_COUNT - 1)));
  BOOST_REQUIRE_EQUAL(5, phrase.words().size());
}

BOOST_AUTO_TEST_CASE(test_read_phrases) {
  PhraseCorpus corpus(phrase_files_dir);
  std::vector<Id> ids;

  // invalid ngram_len
  ids = { Id(0, 0) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);
  ids = { Id(6, 0) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);

  // invalid ngram_id
  ids = { Id(1, -1) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);
  ids = { Id(2, -1) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);
  ids = { Id(3, -1) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);
  ids = { Id(4, -1) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);
  ids = { Id(5, -1) };
  BOOST_REQUIRE_THROW(corpus.read_phrases(ids), std::runtime_error);

  std::vector<Phrase> phrases;

  // valid ngram_len = 1
  ids = { Id(1, 0) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(1, phrases.size());
  BOOST_REQUIRE_EQUAL(1, phrases.front().words().size());
  ids = { Id(1, 0), Id(1, INDEX_1GRAM_COUNT - 1) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(2, phrases.size());
  BOOST_REQUIRE_EQUAL(1, phrases.front().words().size());
  BOOST_REQUIRE_EQUAL(1, phrases.back().words().size());

  // valid ngram_len = 2
  ids = { Id(2, 0) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(1, phrases.size());
  BOOST_REQUIRE_EQUAL(2, phrases.front().words().size());
  ids = { Id(2, 0), Id(2, INDEX_2GRAM_COUNT - 1) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(2, phrases.size());
  BOOST_REQUIRE_EQUAL(2, phrases.front().words().size());
  BOOST_REQUIRE_EQUAL(2, phrases.back().words().size());

  // valid ngram_len = 3
  ids = { Id(3, 0) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(1, phrases.size());
  BOOST_REQUIRE_EQUAL(3, phrases.front().words().size());
  ids = { Id(3, 0), Id(3, INDEX_3GRAM_COUNT - 1) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(2, phrases.size());
  BOOST_REQUIRE_EQUAL(3, phrases.front().words().size());
  BOOST_REQUIRE_EQUAL(3, phrases.back().words().size());

  // valid ngram_len = 4
  ids = { Id(4, 0) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(1, phrases.size());
  BOOST_REQUIRE_EQUAL(4, phrases.front().words().size());
  ids = { Id(4, 0), Id(4, INDEX_4GRAM_COUNT - 1) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(2, phrases.size());
  BOOST_REQUIRE_EQUAL(4, phrases.front().words().size());
  BOOST_REQUIRE_EQUAL(4, phrases.back().words().size());

  // valid ngram_len = 5
  ids = { Id(5, 0) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(1, phrases.size());
  BOOST_REQUIRE_EQUAL(5, phrases.front().words().size());
  ids = { Id(5, 0), Id(5, INDEX_5GRAM_COUNT - 1) };
  BOOST_REQUIRE_NO_THROW(phrases = corpus.read_phrases(ids));
  BOOST_REQUIRE_EQUAL(2, phrases.size());
  BOOST_REQUIRE_EQUAL(5, phrases.front().words().size());
  BOOST_REQUIRE_EQUAL(5, phrases.back().words().size());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace netspeak
