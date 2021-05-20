#ifndef NETSPEAK_CONFIGURATION_HPP
#define NETSPEAK_CONFIGURATION_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/optional.hpp"

#include "netspeak/util/Config.hpp"

namespace netspeak {

class Configuration {
private:
  util::Config config_;
  std::unique_ptr<Configuration> extends_;
  boost::filesystem::path base_dir_;

  void load_extends();
  void desugar_home();

public:
  Configuration() : config_(), extends_(), base_dir_() {}
  Configuration(
      std::initializer_list<util::Config::initializer_list_type> list);
  explicit Configuration(boost::filesystem::path file_name);
  explicit Configuration(const util::Config& config);

  std::string get_required(const std::string& key) const;
  boost::optional<std::string> get_optional(const std::string& key) const;
  std::string get(const std::string& key,
                  const std::string& defaultValue) const;

  boost::filesystem::path get_required_path(const std::string& key) const;
  boost::optional<boost::filesystem::path> get_optional_path(
      const std::string& key) const;
  boost::filesystem::path get_path(const std::string& key,
                                   const std::string& defaultValue) const;

  std::unordered_set<std::string> keys() const;

  util::Config full_config() const;

  Configuration extend(const Configuration& base) const;

public:
  static const std::string PATH_TO_HOME;
  static const std::string PATH_TO_PHRASE_INDEX;
  static const std::string PATH_TO_PHRASE_CORPUS;
  static const std::string PATH_TO_PHRASE_DICTIONARY;
  static const std::string PATH_TO_POSTLIST_INDEX;
  static const std::string PATH_TO_HASH_DICTIONARY;
  static const std::string PATH_TO_REGEX_VOCABULARY;
  static const std::string EXTENDS;

  static const std::string CORPUS_KEY;
  static const std::string CORPUS_NAME;
  static const std::string CORPUS_LANGUAGE;

  static const std::string CACHE_CAPACITY;

  static const std::string SEARCH_REGEX_MAX_MATCHES;
  static const std::string SEARCH_REGEX_MAX_TIME;

  static const std::string DEFAULT_PHRASE_INDEX_DIR_NAME;
  static const std::string DEFAULT_PHRASE_CORPUS_DIR_NAME;
  static const std::string DEFAULT_PHRASE_DICTIONARY_DIR_NAME;
  static const std::string DEFAULT_POSTLIST_INDEX_DIR_NAME;
  static const std::string DEFAULT_HASH_DICTIONARY_DIR_NAME;
  static const std::string DEFAULT_REGEX_VOCABULARY_DIR_NAME;
};

} // namespace netspeak

#endif
