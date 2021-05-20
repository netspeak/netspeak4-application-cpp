#ifndef NETSPEAK_INDEXING_HPP
#define NETSPEAK_INDEXING_HPP

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "netspeak/Configuration.hpp"
#include "netspeak/invertedindex/Postlist.hpp"
#include "netspeak/model/typedefs.hpp"

namespace netspeak {

/**
 * Builds all Netspeak components from a collection of n-grams (phrases).
 * Preconditions:
 *  - phrase_dir contains n-gram files only.
 *  - netspeak_dir does not exist and will be created or is empty.
 *  - All n-grams in phrase_dir are unique.
 *
 * @param phrase_dir
 * @param netspeak_dir
 */
void BuildNetspeak(const boost::filesystem::path& phrase_dir,
                   const boost::filesystem::path& netspeak_dir);

/**
 * This function builds the "phrase-corpus" component of Netspeak.
 *
 * It primarily converts the textual n-gram files into binary n-gram files.
 * To do so it assigns a unique id to each unigram first, and then
 * replaces each n-gram with its corresponding sequence of ids.
 * E.g. 'hello' gets the id 23, and 'world' gets the id 42, then the 2-gram
 *      'hello world' can be represented by '23 42'.
 *
 * The id-represented n-grams are stored in binary encoding with 4 byte per id
 * and 4 byte for the n-gram count. The 2-gram "hello world 123" becomes the
 * following binary encoding: 123 (4 byte), 23 (4 byte), 42 (4 byte) = 12 byte
 *
 * Furthermore, just as the unigrams, each textual n-gram (n > 1) gets its
 * own unique id. This general n-gram id is later used by the indexer to refer
 * to n-gram instances (similar to some doc-id).
 *
 * Since the output of an n-gram index lookup is a set of n-gram ids, the
 * purpose of this binary corpus is to provide a fast mapping from n-gram ids
 * to its textual representation. The key concept is to be able to calculate a
 * disk address (file offset) of each n-gram just given its id. With a textual
 * n-gram file this is not possible, b/c here n-grams have variable length
 * (in bytes) and one cannot calculate any offsets.
 *
 * Example:
 *
 * Some index lookup returns the id 1983, which represents some n-gram.
 * We know that the constant size of a binary 2-gram is 12 byte.
 * Then the address of the binary n-gram is: offset = 1983 * 12
 * We seek to that file offset in the binary corpus, read the 12 bytes of
 * data and convert them to the sequence: 123 (count) 23 (word1) 42 (word2)
 *
 * With the mapping for the unigrams in memory (id -> unigram) we can now
 * recover the textual ngram: 123 hello world
 *
 * Preconditions:
 * - phrase_dir is a directory that contains n-gram files.
 * - All n-grams in phrase_dir are unique.
 * - The phrase_corpus_dir is empty and writable.
 */
uint64_t BuildPhraseCorpus(const boost::filesystem::path& phrase_dir,
                           const boost::filesystem::path& phrase_corpus_dir);

/**
 * @param phrase_dir
 * @param phrase_dictionary_dir
 */
void BuildPhraseDictionary(
    const boost::filesystem::path& phrase_dir,
    const boost::filesystem::path& phrase_dictionary_dir);

/**
 * @param phrase_dir
 * @param phrase_index_dir
 * @param expected_record_count
 */
void BuildPhraseIndex(const boost::filesystem::path& phrase_dir,
                      const boost::filesystem::path& phrase_index_dir,
                      uint64_t expected_record_count);

std::vector<model::PostlistIndexValue> IndexPostlist(
    const invertedindex::Postlist<model::PhraseIndexValue>& postlist,
    std::size_t resolution);

/**
 * This function builds an inverted index
 * of the postlists of an invertded n-gram index.
 *
 * @param phrase_index_dir
 * @param postlist_index_dir
 */
void BuildPostlistIndex(const boost::filesystem::path& phrase_index_dir,
                        const boost::filesystem::path& postlist_index_dir);

/**
 * Creates the regex vocabulary for the index.
 *
 * @param regex_vocabulary_dir
 * @param phrase_corpus_dir
 */
void BuildRegexVocabulary(const boost::filesystem::path& regex_vocabulary_dir,
                          const boost::filesystem::path& phrase_corpus_dir,
                          const Configuration& config);

/**
 * Merges n-gram duplicates by adding their frequency values to form a set of
 * unique n-grams, which can be indexed for the Netspeak service.
 *
 * @param phrase_src_dir Directory with n-gram files.
 * @param phrase_dst_dir Directory to store n-grams files with unique n-grams.
 */
void MergeDuplicates(const boost::filesystem::path& phrase_src_dir,
                     const boost::filesystem::path& phrase_dst_dir);

/**
 * Sets the file denoted by \c path to read-only mode. If \c path points to a
 * directory the function calls itself recursively on contained files.
 *
 * @param path
 */
void SetReadOnly(const boost::filesystem::path& path);

} // namespace netspeak

#endif // NETSPEAK_INDEXING_HPP
