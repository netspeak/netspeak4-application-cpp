#ifndef NETSPEAK_PROPERTIES_HPP
#define NETSPEAK_PROPERTIES_HPP

#include "netspeak/util/Config.hpp"

namespace netspeak {

class Properties : public util::Config {
public:
  Properties() : util::Config() {}
  Properties(const std::string& file_name) : util::Config(file_name) {}
  Properties(std::initializer_list<util::Config::initializer_list_type> list)
      : util::Config(list) {}

public:
  // cache properties
  static const std::string cache_size;
  static const std::string cache_policy;
  static const std::string cache_capacity;
  static const std::string cache_access_count;
  static const std::string cache_hit_rate;
  static const std::string cache_top_100;

  // phrase corpus properties
  static const std::string phrase_corpus_1gram_count;
  static const std::string phrase_corpus_2gram_count;
  static const std::string phrase_corpus_3gram_count;
  static const std::string phrase_corpus_4gram_count;
  static const std::string phrase_corpus_5gram_count;

  // phrase dictionary properties
  static const std::string phrase_dictionary_size;
  static const std::string phrase_dictionary_value_type;

  // phrase index properties
  static const std::string phrase_index_key_count;
  static const std::string phrase_index_value_type;
  static const std::string phrase_index_value_sorting;
  static const std::string phrase_index_value_count;
  static const std::string phrase_index_total_size;

  // postlist index properties
  static const std::string postlist_index_key_count;
  static const std::string postlist_index_value_type;
  static const std::string postlist_index_value_sorting;
  static const std::string postlist_index_value_count;
  static const std::string postlist_index_total_size;

  // hash dictionary properties
  static const std::string hash_dictionary_size;
  static const std::string hash_dictionary_value_type;

  // regex vocabulary properties
  static const std::string regex_vocabulary_size;
  static const std::string regex_vocabulary_value_type;
};

} // namespace netspeak

#endif
