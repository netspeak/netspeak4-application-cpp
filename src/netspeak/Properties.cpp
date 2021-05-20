#include "netspeak/Properties.hpp"

namespace netspeak {

#define PREFIX const std::string Properties

// cache properties
PREFIX::cache_size("cache.size");
PREFIX::cache_policy("cache.policy");
PREFIX::cache_capacity("cache.capacity");
PREFIX::cache_access_count("cache.access.count");
PREFIX::cache_hit_rate("cache.hit.rate");
PREFIX::cache_top_100("cache.top.100");

// phrase corpus properties
PREFIX::phrase_corpus_1gram_count("phrase.corpus.1gram.count");
PREFIX::phrase_corpus_2gram_count("phrase.corpus.2gram.count");
PREFIX::phrase_corpus_3gram_count("phrase.corpus.3gram.count");
PREFIX::phrase_corpus_4gram_count("phrase.corpus.4gram.count");
PREFIX::phrase_corpus_5gram_count("phrase.corpus.5gram.count");

// phrase dictionary properties
PREFIX::phrase_dictionary_size("phrase.dictionary.size");
PREFIX::phrase_dictionary_value_type("phrase.dictionary.value.type");

// phrase index properties
PREFIX::phrase_index_key_count("phrase.index.key.count");
PREFIX::phrase_index_value_type("phrase.index.value.type");
PREFIX::phrase_index_value_sorting("phrase.index.value.sorting");
PREFIX::phrase_index_value_count("phrase.index.value.count");
PREFIX::phrase_index_total_size("phrase.index.total.size");

// postlist index properties
PREFIX::postlist_index_key_count("postlist.index.key.count");
PREFIX::postlist_index_value_type("postlist.index.value.type");
PREFIX::postlist_index_value_sorting("postlist.index.value.sorting");
PREFIX::postlist_index_value_count("postlist.index.value.count");
PREFIX::postlist_index_total_size("postlist.index.total.size");

// hash dictionary properties
PREFIX::hash_dictionary_size("hash.dictionary.size");
PREFIX::hash_dictionary_value_type("hash.dictionary.value.type");

// regex vocabulary properties
PREFIX::regex_vocabulary_size("regex.vocabulary.size");
PREFIX::regex_vocabulary_value_type("regex.vocabulary.value.type");

} // namespace netspeak
