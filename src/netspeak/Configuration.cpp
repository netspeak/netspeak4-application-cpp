#include "netspeak/Configuration.hpp"

#include <string>

#include "boost/filesystem.hpp"

#include "netspeak/util/traceable_error.hpp"

namespace netspeak {

namespace bfs = boost::filesystem;


// CONSTANTS


#define PREFIX const std::string Configuration

PREFIX::PATH_TO_HOME("path.to.home");
PREFIX::PATH_TO_PHRASE_INDEX("path.to.phrase-index");
PREFIX::PATH_TO_PHRASE_CORPUS("path.to.phrase-corpus");
PREFIX::PATH_TO_PHRASE_DICTIONARY("path.to.phrase-dictionary");
PREFIX::PATH_TO_POSTLIST_INDEX("path.to.postlist-index");
PREFIX::PATH_TO_HASH_DICTIONARY("path.to.hash-dictionary");
PREFIX::PATH_TO_REGEX_VOCABULARY("path.to.regex-vocabulary");
PREFIX::EXTENDS("extends");

PREFIX::CORPUS_KEY("corpus.key");
PREFIX::CORPUS_NAME("corpus.name");
PREFIX::CORPUS_LANGUAGE("corpus.language");

PREFIX::CACHE_CAPACITY("cache.capacity");

PREFIX::SEARCH_REGEX_MAX_MATCHES("search.regex.max-matches");
PREFIX::SEARCH_REGEX_MAX_TIME("search.regex.max-time");

PREFIX::DEFAULT_PHRASE_INDEX_DIR_NAME("phrase-index");
PREFIX::DEFAULT_PHRASE_CORPUS_DIR_NAME("phrase-corpus");
PREFIX::DEFAULT_PHRASE_DICTIONARY_DIR_NAME("phrase-dictionary");
PREFIX::DEFAULT_POSTLIST_INDEX_DIR_NAME("postlist-index");
PREFIX::DEFAULT_HASH_DICTIONARY_DIR_NAME("hash-dictionary");
PREFIX::DEFAULT_REGEX_VOCABULARY_DIR_NAME("regex-vocabulary");


const std::unordered_set<std::string> PATHS{
  Configuration::PATH_TO_HOME,
  Configuration::PATH_TO_PHRASE_INDEX,
  Configuration::PATH_TO_PHRASE_CORPUS,
  Configuration::PATH_TO_PHRASE_DICTIONARY,
  Configuration::PATH_TO_POSTLIST_INDEX,
  Configuration::PATH_TO_HASH_DICTIONARY,
  Configuration::PATH_TO_REGEX_VOCABULARY,
};


// CONSTRUCTOR


void Configuration::load_extends() {
  if (config_.contains(EXTENDS)) {
    auto extends_path = bfs::path(config_.get(EXTENDS));
    if (extends_path.empty()) {
      return;
    }
    if (!base_dir_.empty()) {
      extends_path = bfs::absolute(extends_path, base_dir_);
    }
    extends_ = std::make_unique<Configuration>(extends_path);
  }
}
void Configuration::desugar_home() {
  if (config_.contains(PATH_TO_HOME)) {
    auto home_path = bfs::path(config_.get(PATH_TO_HOME));
    if (home_path.empty()) {
      return;
    }
    if (!base_dir_.empty()) {
      home_path = bfs::absolute(home_path, base_dir_);
    }

    auto set_default = [&](const std::string& key,
                           const std::string& default_value) {
      if (!config_.contains(key)) {
        config_[key] =
            bfs::weakly_canonical(bfs::absolute(default_value, home_path))
                .string();
      }
    };

    set_default(PATH_TO_PHRASE_CORPUS, DEFAULT_PHRASE_CORPUS_DIR_NAME);
    set_default(PATH_TO_PHRASE_DICTIONARY, DEFAULT_PHRASE_DICTIONARY_DIR_NAME);
    set_default(PATH_TO_PHRASE_INDEX, DEFAULT_PHRASE_INDEX_DIR_NAME);
    set_default(PATH_TO_POSTLIST_INDEX, DEFAULT_POSTLIST_INDEX_DIR_NAME);
    set_default(PATH_TO_HASH_DICTIONARY, DEFAULT_HASH_DICTIONARY_DIR_NAME);
    set_default(PATH_TO_REGEX_VOCABULARY, DEFAULT_REGEX_VOCABULARY_DIR_NAME);
  }
}

Configuration::Configuration(
    std::initializer_list<util::Config::initializer_list_type> list)
    : config_(list), extends_(), base_dir_() {
  load_extends();
  desugar_home();
}
Configuration::Configuration(bfs::path file_name)
    : config_(file_name), extends_(), base_dir_(file_name.parent_path()) {
  load_extends();
  desugar_home();
}
Configuration::Configuration(const util::Config& config)
    : config_(config), extends_(), base_dir_() {
  load_extends();
  desugar_home();
}


// METHODS


std::string Configuration::get_required(const std::string& key) const {
  auto value = get_optional(key);
  if (value) {
    return *value;
  } else {
    throw util::tracable_runtime_error("The required key " + key +
                                       " is missing.");
  }
}
boost::optional<std::string> Configuration::get_optional(
    const std::string& key) const {
  if (config_.contains(key)) {
    return config_.get(key);
  } else if (extends_) {
    return extends_->get_optional(key);
  } else {
    return boost::none;
  }
}
std::string Configuration::get(const std::string& key,
                               const std::string& defaultValue) const {
  auto value = get_optional(key);
  if (value) {
    return *value;
  } else {
    return defaultValue;
  }
}

bfs::path relative_to(std::string path, const bfs::path& to) {
  if (to.empty()) {
    return bfs::weakly_canonical(path);
  } else {
    return bfs::weakly_canonical(bfs::absolute(path, to));
  }
}
bfs::path Configuration::get_required_path(const std::string& key) const {
  return relative_to(get_required(key), base_dir_);
}
boost::optional<bfs::path> Configuration::get_optional_path(
    const std::string& key) const {
  auto value = get_optional(key);
  if (value) {
    return relative_to(*value, base_dir_);
  } else {
    return boost::none;
  }
}
bfs::path Configuration::get_path(const std::string& key,
                                  const std::string& defaultValue) const {
  return relative_to(get(key, defaultValue), base_dir_);
}


std::unordered_set<std::string> Configuration::keys() const {
  std::unordered_set<std::string> result;
  if (extends_) {
    result = extends_->keys();
  }

  for (auto& foo : config_) {
    result.insert(foo.first);
  }

  result.erase(EXTENDS);

  return result;
}


util::Config Configuration::full_config() const {
  util::Config config;

  for (auto& key : keys()) {
    if (PATHS.find(key) == PATHS.end()) {
      // not a path
      config[key] = get_required(key);
    } else {
      // path
      config[key] = get_required_path(key).string();
    }
  }

  return config;
}


Configuration Configuration::extend(const Configuration& base) const {
  auto config = base.full_config();

  auto this_config = full_config();
  config.merge(this_config.begin(), this_config.end());

  return Configuration(config);
}

} // namespace netspeak
