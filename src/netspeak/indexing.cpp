#include "netspeak/indexing.hpp"

#include <sys/stat.h>

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/filesystem/fstream.hpp>

#include "netspeak/Configuration.hpp"
#include "netspeak/Netspeak.hpp"
#include "netspeak/PhraseCorpus.hpp"
#include "netspeak/PhraseFileReader.hpp"
#include "netspeak/bighashmap/BigHashMap.hpp"
#include "netspeak/error.hpp"
#include "netspeak/invertedindex/Configuration.hpp"
#include "netspeak/invertedindex/ManagedIndexer.hpp"
#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/invertedindex/Searcher.hpp"
#include "netspeak/model/Phrase.hpp"
#include "netspeak/service/NetspeakService.pb.h"
#include "netspeak/util/check.hpp"
#include "netspeak/util/systemio.hpp"

namespace netspeak {

namespace ai = invertedindex;
namespace bfs = boost::filesystem;
using namespace model;

void BuildNetspeak(const bfs::path& phrase_dir, const bfs::path& netspeak_dir) {
  util::check(bfs::exists(phrase_dir), error_message::does_not_exist,
              phrase_dir);
  util::CreateOrCheckIfEmpty(netspeak_dir);

  // -------------------------------------------------------------------------
  // Build component "phrase-corpus"
  // -------------------------------------------------------------------------

  const bfs::path phrase_corpus_dir =
      netspeak_dir / Configuration::DEFAULT_PHRASE_CORPUS_DIR_NAME;
  util::check(bfs::create_directory(phrase_corpus_dir),
              error_message::cannot_create, phrase_corpus_dir);

  util::log("Building component", phrase_corpus_dir);
  const uint64_t num_records = BuildPhraseCorpus(phrase_dir, phrase_corpus_dir);
  SetReadOnly(phrase_corpus_dir);

  // -------------------------------------------------------------------------
  // Build component "phrase-dictionary"
  // -------------------------------------------------------------------------

  const bfs::path phrase_dictionary_dir =
      netspeak_dir / Configuration::DEFAULT_PHRASE_DICTIONARY_DIR_NAME;
  util::check(bfs::create_directories(phrase_dictionary_dir),
              error_message::cannot_create, phrase_dictionary_dir);

  util::log("Building component", phrase_dictionary_dir);
  const bfs::path corpus_dir_txt = phrase_corpus_dir / PhraseCorpus::txt_dir;
  BuildPhraseDictionary(corpus_dir_txt, phrase_dictionary_dir);
  SetReadOnly(phrase_dictionary_dir);

  // -------------------------------------------------------------------------
  // Build component "phrase-index"
  // -------------------------------------------------------------------------

  const bfs::path phrase_index_dir =
      netspeak_dir / Configuration::DEFAULT_PHRASE_INDEX_DIR_NAME;
  util::check(bfs::create_directories(phrase_index_dir),
              error_message::cannot_create, phrase_index_dir);

  util::log("Building component", phrase_index_dir);
  BuildPhraseIndex(corpus_dir_txt, phrase_index_dir, num_records);
  SetReadOnly(phrase_index_dir);

  // -------------------------------------------------------------------------
  // Build component "postlist-index"
  // -------------------------------------------------------------------------

  const bfs::path postlist_index_dir =
      netspeak_dir / Configuration::DEFAULT_POSTLIST_INDEX_DIR_NAME;
  util::check(bfs::create_directory(postlist_index_dir),
              error_message::cannot_create, postlist_index_dir);

  util::log("Building component", postlist_index_dir);
  BuildPostlistIndex(phrase_index_dir, postlist_index_dir);
  SetReadOnly(postlist_index_dir);

  // -------------------------------------------------------------------------
  // Build component "regex-vocabulary"
  // -------------------------------------------------------------------------

  const Configuration config = {
    { Configuration::PATH_TO_HOME, netspeak_dir.string() },
    { Configuration::PATH_TO_PHRASE_CORPUS, phrase_corpus_dir.string() },
    { Configuration::PATH_TO_PHRASE_DICTIONARY,
      phrase_dictionary_dir.string() },
    { Configuration::PATH_TO_PHRASE_INDEX, phrase_index_dir.string() },
    { Configuration::PATH_TO_POSTLIST_INDEX, postlist_index_dir.string() },
    { Configuration::CACHE_CAPACITY, "1000" },
  };

  const bfs::path regex_vocabulary_dir =
      netspeak_dir / Configuration::DEFAULT_REGEX_VOCABULARY_DIR_NAME;
  util::check(bfs::create_directory(regex_vocabulary_dir),
              error_message::cannot_create, regex_vocabulary_dir);

  util::log("Building component", regex_vocabulary_dir);
  BuildRegexVocabulary(regex_vocabulary_dir, phrase_corpus_dir, config);
  SetReadOnly(regex_vocabulary_dir);
}

uint64_t BuildPhraseCorpus(const bfs::path& phrase_dir,
                           const bfs::path& phrase_corpus_dir) {
  const bfs::path pc_txt_dir = phrase_corpus_dir / PhraseCorpus::txt_dir;
  const bfs::path pc_bin_dir = phrase_corpus_dir / PhraseCorpus::bin_dir;
  util::check(bfs::create_directory(pc_txt_dir), error_message::cannot_create,
              pc_txt_dir);
  util::check(bfs::create_directory(pc_bin_dir), error_message::cannot_create,
              pc_bin_dir);

  typedef std::shared_ptr<std::ostream> ostream_pointer;
  std::unordered_map<size_t, ostream_pointer> phrase_len_to_txt_os;
  std::unordered_map<size_t, ostream_pointer> phrase_len_to_bin_os;
  std::unordered_map<size_t, Phrase::Id::Local> phrase_len_to_id;
  std::unordered_map<std::string, Phrase::Id::Local> unigram_to_id;

  PhraseFileParserItem parser_item;
  Phrase::Id::Local phrase_id;
  Phrase::Frequency phrase_freq;
  const bfs::directory_iterator dir_end;
  for (bfs::directory_iterator it(phrase_dir); it != dir_end; ++it) {
    bfs::ifstream ifs(it->path());
    util::check(ifs.is_open(), error_message::cannot_open, it->path());
    PhraseFileParser<false> parser(ifs);
    util::log("Processing", it->path());
    while (parser.read_next(parser_item)) {
      auto length = parser_item.words.size();
      // Add words to the vocabulary (assigning a new id to new words).
      for (const auto& word : parser_item.words) {
        unigram_to_id.insert(std::make_pair(word, unigram_to_id.size()));
      }
      // Create text and binary file if not present.
      if (phrase_len_to_id.find(length) == phrase_len_to_id.end()) {
        std::ostringstream oss;
        oss << PhraseCorpus::phrase_file << '.' << length;
        const bfs::path txt_file = pc_txt_dir / oss.str();
        phrase_len_to_txt_os[length] =
            std::shared_ptr<std::ostream>(new bfs::ofstream(txt_file));
        oss.clear();
        oss.str("");
        oss << PhraseCorpus::phrase_file << '.' << length;
        const bfs::path bin_file = pc_bin_dir / oss.str();
        phrase_len_to_bin_os[length] = std::shared_ptr<std::ostream>(
            new bfs::ofstream(bin_file, std::ios_base::binary));
      }
      // Set phrase-id and auto-increment id.
      parser_item.id = phrase_len_to_id[length]++;
      // Write phrase in text representation.
      auto& txt_os = phrase_len_to_txt_os[length];
      PhraseFileParser<true>::write(*txt_os, parser_item);
      // Write phrase in binary representation.
      phrase_freq = parser_item.freq;
      auto& bin_os = *phrase_len_to_bin_os[length];
      bin_os.write(reinterpret_cast<const char*>(&phrase_freq),
                   sizeof(Phrase::Frequency));
      for (const auto& word : parser_item.words) {
        phrase_id = unigram_to_id[word];
        bin_os.write(reinterpret_cast<const char*>(&phrase_id),
                     sizeof(Phrase::Id::Local));
      }
    }
  }
  // Write vocabulary.
  const bfs::path vocab_file = pc_bin_dir / PhraseCorpus::vocab_file;
  bfs::ofstream ofs(vocab_file);
  util::check(ofs.is_open(), error_message::cannot_create, vocab_file);
  for (const auto& entry : unigram_to_id) {
    ofs << entry.first << '\t' << entry.second << '\n';
  }
  // Calculate expected number of records (need for indexing)
  uint64_t record_count = 0;
  for (const auto& entry : phrase_len_to_id) {
    record_count += entry.first * entry.second;
  }
  return record_count;
}

void BuildPhraseDictionary(const bfs::path& phrase_dir,
                           const bfs::path& phrase_dictionary_dir) {
  typedef value::pair<Phrase::Frequency, Phrase::Id::Local> Value;
  bighashmap::BigHashMap<Value>::Build(phrase_dir, phrase_dictionary_dir);
}

void BuildPhraseIndex(const bfs::path& phrase_dir,
                      const bfs::path& phrase_index_dir,
                      uint64_t expected_record_count) {
  ai::Configuration config;
  config.set_input_directory(phrase_dir.string());
  config.set_index_directory(phrase_index_dir.string());
  config.set_key_sorting(ai::key_sorting_type::unsorted);
  config.set_value_sorting(ai::value_sorting_type::descending);
  config.set_max_memory_usage(util::memory_type::mb4096);
  config.set_expected_record_count(expected_record_count);

  typedef PhraseFileReader<true> record_reader_type;
  typedef record_reader_type::record_type::value_type value_type;
  ai::ManagedIndexer<value_type, record_reader_type>::index(config);
}

void BuildPostlistIndex(const bfs::path& phrase_index_dir,
                        const bfs::path& postlist_index_dir) {
  // Load the n-gram index.
  ai::Configuration config;
  config.set_index_directory(phrase_index_dir.string());
  config.set_max_memory_usage(util::memory_type::min_required);
  ai::Searcher<PhraseIndexValue> searcher(config);

  // Setup postlist index.
  config.set_index_directory(postlist_index_dir.string());
  config.set_key_sorting(ai::key_sorting_type::sorted);
  config.set_value_sorting(ai::value_sorting_type::disabled);
  ai::Indexer<PostlistIndexValue> indexer(config);

  // Lookup each key and index the returned postlist.
  const bfs::path table_txt = phrase_index_dir / "table.txt";
  bfs::ifstream ifs(table_txt);
  util::check(ifs.is_open(), error_message::cannot_open, table_txt);
  ai::Record<PostlistIndexValue> postlist_index_record;
  std::string key, fileid, offset;
  while (ifs >> key >> fileid >> offset) {
    const auto postlist = searcher.search_postlist(key);
    util::check(postlist.get(), "unknown key", key);
    postlist_index_record.set_key(key);
    // Postlists shorter than 1000 will not be indexed.
    const auto postlist_index =
        IndexPostlist(*postlist, postlist->size() / 1000);
    for (const auto& entry : postlist_index) {
      postlist_index_record.set_value(entry);
      indexer.insert(postlist_index_record);
    }
  }
  indexer.index();
}

std::vector<PostlistIndexValue> IndexPostlist(
    const ai::Postlist<PostlistIndexValue>& postlist, std::size_t resolution) {
  std::vector<PostlistIndexValue> postlist_index(resolution);
  if (postlist_index.empty()) {
    return postlist_index;
  }

  // Compute normalized total frequency for 1.0 quantile.
  PostlistIndexValue value;
  double total_norm_freq = 0;
  while (postlist.next(value)) {
    total_norm_freq += static_cast<double>(value.e1()) /
                       std::numeric_limits<PhraseIndexValue::e1_type>::max();
  }

  // Compute partial normalized frequencies at quantiles.
  const double step_width = 1.0 / resolution;
  std::vector<double> norm_freq_at_quantiles(resolution);
  for (unsigned i = 0; i != norm_freq_at_quantiles.size(); ++i) {
    norm_freq_at_quantiles[i] = (i + 1) * step_width * total_norm_freq;
  }

  // Index postlist.
  postlist.rewind();
  double norm_freq = 0;
  size_t quantile_index = 0;
  double cur_total_norm_freq = 0;
  PostlistIndexValue index_value;
  for (unsigned i = 0; postlist.next(value); ++i) {
    norm_freq = static_cast<double>(value.e1()) /
                std::numeric_limits<PhraseIndexValue::e1_type>::max();
    cur_total_norm_freq += norm_freq;
    assert(cur_total_norm_freq <= total_norm_freq);
    while (cur_total_norm_freq >= norm_freq_at_quantiles.at(quantile_index)) {
      // Set current postlist index and value frequency.
      postlist_index.at(quantile_index++).set(i, value.e1());
      if (quantile_index == postlist_index.size())
        break;
    }
    if (quantile_index == postlist_index.size())
      break;
  }
  return postlist_index;
}

typedef std::pair<std::string, uint64_t> WordFreqPair;

std::vector<WordFreqPair> read_1grams(
    const boost::filesystem::path& phrase_corpus_dir) {
  std::vector<WordFreqPair> word_freq_pairs;

  const auto one_grams = phrase_corpus_dir / PhraseCorpus::txt_dir /
                         (PhraseCorpus::phrase_file + ".1");
  bfs::ifstream ifs(one_grams);
  util::check(ifs.is_open(), error_message::cannot_open, one_grams);

  std::string word;
  uint64_t frequency;
  uint32_t id;
  while (ifs >> word >> frequency >> id) {
    const WordFreqPair pair(word, frequency);
    word_freq_pairs.push_back(pair);
  }

  return word_freq_pairs;
}
std::vector<std::string> find_missing_words(
    const boost::filesystem::path& phrase_corpus_dir,
    const std::vector<WordFreqPair>& word_freq_pairs) {
  std::unordered_set<std::string> words;
  for (const auto& pair : word_freq_pairs) {
    words.insert(pair.first);
  }

  std::vector<std::string> missing;

  const auto vocab_file =
      phrase_corpus_dir / PhraseCorpus::bin_dir / PhraseCorpus::vocab_file;
  bfs::ifstream ifs(vocab_file);
  util::check(ifs.is_open(), error_message::cannot_open, vocab_file);

  std::string word;
  WordId word_id;
  while (ifs >> word >> word_id) {
    if (words.find(word) == words.end()) {
      missing.push_back(word);
    }
  }

  return missing;
}
void add_missing(std::vector<WordFreqPair>& word_freq_pairs,
                 const boost::filesystem::path& phrase_corpus_dir,
                 const Configuration& config) {
  // read "phrase-corpus/bin/vocab" file to find missing words
  const auto missing = find_missing_words(phrase_corpus_dir, word_freq_pairs);
  if (!missing.empty()) {
    // since the index is basically ready, we will just query "* <word> *" for
    // every missing word and use the highest frequency.
    Netspeak netspeak;
    netspeak.initialize(config);

    for (const auto& word : missing) {
      service::SearchRequest req;
      req.set_query("* " + word + " *");
      req.set_max_phrases(2);
      req.mutable_phrase_constraints()->set_words_min(2); // it's not a 1-gram

      uint64_t freq = 0;

      service::SearchResponse res;
      netspeak.search(req, res);
      if (res.has_result()) {
        const auto& result = res.result();

        if (result.phrases_size() > 0) {
          // found at least one phrase with that word
          freq = result.phrases(0).frequency();
        } else {
          // no phrases with that word
          std::printf(
              "Warning: There are no phrases that with the word \"%s\".",
              word.c_str());
        }
      } else {
        const auto& error = res.error();

        // error occurred
        std::printf(
            "Warning: Unknown error occurred while searching for \"%s\": "
            "%s : %s\n",
            word.c_str(),
            service::SearchResponse::Error::Kind_Name(error.kind()).c_str(),
            error.message().c_str());
      }

      std::cout << "Assumed frequency " << freq << " for missing word: " << word
                << std::endl;
      word_freq_pairs.push_back(WordFreqPair(word, freq));
    }
  }
}

void BuildRegexVocabulary(const boost::filesystem::path& regex_vocabulary_dir,
                          const boost::filesystem::path& phrase_corpus_dir,
                          const Configuration& config) {
  auto word_freq_pairs = read_1grams(phrase_corpus_dir);

  // Simply going though all 1-grams is not enough because there might be some
  // words in higher n-grams that are not a 1-gram. There are some reason for
  // why this might happen but the two most simple are incorrect data sets or
  // incorrect preprocessing.
  add_missing(word_freq_pairs, phrase_corpus_dir, config);

  std::sort(word_freq_pairs.begin(), word_freq_pairs.end(),
            [](const WordFreqPair& a, const WordFreqPair& b) {
              // sort by frequency
              return a.second > b.second;
            });

  // output to file
  const bfs::path regex_vocabulary_file = regex_vocabulary_dir / "vocab.sorted";
  bfs::ofstream ofs(regex_vocabulary_file);
  util::check(ofs.is_open(), error_message::cannot_create,
              regex_vocabulary_file);
  for (const auto& entry : word_freq_pairs) {
    ofs << entry.first << '\n';
  }
}

void MergeDuplicates(const bfs::path& phrase_src_dir,
                     const bfs::path& phrase_dst_dir) {
  const char separator = '\t';
  const std::string tmp_file_prefix = "not_unique.";

  // Create phrase_dst_dir_ph if does not exists
  if (!bfs::exists(phrase_dst_dir)) {
    bfs::create_directory(phrase_dst_dir);
  }

  // Create partition files in tmp directory.
  const bfs::path tmp_dir = phrase_dst_dir / "tmp";
  util::check(bfs::create_directory(tmp_dir), __func__,
              error_message::cannot_create, tmp_dir);
  std::vector<std::shared_ptr<bfs::ofstream> > tmp_files(
      util::next_prime(std::distance(bfs::directory_iterator(phrase_src_dir),
                                     bfs::directory_iterator())));
  for (unsigned i = 0; i != tmp_files.size(); ++i) {
    const bfs::path ph = tmp_dir / (tmp_file_prefix + util::to_string(i));
    tmp_files[i].reset(new bfs::ofstream(ph));
    util::check(tmp_files[i]->is_open(), __func__, error_message::cannot_create,
                ph);
  }
  // Partition all n-grams (equal n-grams go to the same tmp file).
  std::string line;
  std::string::size_type tabpos;
  const bfs::directory_iterator dir_end;
  for (bfs::directory_iterator it(phrase_src_dir); it != dir_end; ++it) {
    bfs::ifstream ifs(it->path());
    util::check(ifs.is_open(), __func__, error_message::cannot_open,
                it->path());
    util::log("Processing", it->path());
    while (std::getline(ifs, line)) {
      tabpos = line.find(separator);
      if (tabpos == std::string::npos)
        continue;
      const size_t hash = util::hash32(line.substr(0, tabpos));
      *tmp_files[hash % tmp_files.size()] << line << '\n';
    }
  }
  tmp_files.clear(); // Flushes and closes all tmp files.

  // Merge n-grams of each tmp file.
  std::unordered_map<std::string, uint64_t> ngram_to_freq;
  for (bfs::directory_iterator dir_it(tmp_dir); dir_it != dir_end; ++dir_it) {
    bfs::ifstream ifs(dir_it->path());
    util::check(ifs.is_open(), __func__, error_message::cannot_open,
                dir_it->path());
    util::log("Processing", dir_it->path());
    while (std::getline(ifs, line)) {
      tabpos = line.find(separator);
      if (tabpos == std::string::npos) {
        continue;
      }
      const uint64_t phrase_freq = std::stoull(line.substr(tabpos));
      const auto ret = ngram_to_freq.insert(
          std::make_pair(line.substr(0, tabpos), phrase_freq));
      if (!ret.second) {
        ret.first->second += phrase_freq;
      }
    }
    ifs.close();
    bfs::remove(dir_it->path());

    // Write file with unique n-grams.
    const bfs::path ph =
        phrase_dst_dir /
        dir_it->path().filename().string().substr(4); // trim "not_" prefix
    bfs::ofstream ofs(ph);
    util::check(ofs.is_open(), __func__, error_message::cannot_create, ph);
    for (const auto& entry : ngram_to_freq) {
      ofs << entry.first << separator << entry.second << '\n';
    }
    ngram_to_freq.clear();
  }
  bfs::remove_all(tmp_dir);
}

void SetReadOnly(const bfs::path& path) {
  if (bfs::is_regular_file(path)) {
    ::chmod(path.string().c_str(), S_IRUSR | S_IRGRP | S_IROTH);
  } else if (bfs::is_directory(path)) {
    const bfs::directory_iterator dir_end;
    for (bfs::directory_iterator it(path); it != dir_end; ++it) {
      SetReadOnly(*it);
    }
  }
}

} // namespace netspeak
