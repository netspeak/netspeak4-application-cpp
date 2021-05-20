#ifndef NETSPEAK_UTIL_STRING_ID_MAP_HPP
#define NETSPEAK_UTIL_STRING_ID_MAP_HPP

#include <stdint.h>

#include <cstring>
#include <string>
#include <vector>


namespace netspeak {
namespace util {


/**
 * @brief This is a highly specialized data structure for strings with ids.
 *
 * The data stored is a collection of strings where each string has a unique id
 * of type `Id`. Ids are assumed to incrementing number. Random ids are not
 * supported.
 *
 * This collection implements efficient maps mapping from id to string and
 * string to id. At most 2^32-1 elements are supported.
 *
 * Memory consumption is tuned to be as minimal as possible.
 *
 * To get the string data from an iterator, use the `get_c_str` methods.
 *
 * @tparam Id
 */
template <typename Id>
class StringIdMap {
private:
  typedef uint32_t CharDataOffset;

public:
  struct string_entry {
  private:
    CharDataOffset offset_;
    friend class StringIdMap;
    friend class Builder;

    string_entry() = delete;
    string_entry(CharDataOffset offset) : offset_(offset) {}
  };

  typedef std::pair<string_entry, Id> ValueType;
  typedef typename std::vector<ValueType>::const_iterator const_iterator;

  class Builder {
  private:
    std::vector<char> char_data_;
    std::vector<ValueType> id_data_;
    friend class StringIdMap;

  public:
    Builder() : char_data_(), id_data_() {}
    Builder(const Builder&) = delete;

    void append(const std::string& word, Id id) {
      // new entry
      string_entry offset(static_cast<CharDataOffset>(char_data_.size()));
      id_data_.emplace_back(offset, id);
      // append characters
      char_data_.insert(char_data_.end(), word.begin(), word.end());
      char_data_.push_back('\0');
    }
  };

private:
  static const char* get_c_str(const char* data, const string_entry& entry) {
    return data + static_cast<size_t>(entry.offset_);
  }
  static const char* get_c_str(const char* data, const ValueType& value) {
    return get_c_str(data, value.first);
  }
  static bool str_less_than(const char* a, const char* b) {
    return ::strcmp(a, b) < 0;
  }

  static std::vector<ValueType> create_sorted_words(
      const std::vector<char>& char_data_, std::vector<ValueType>&& id_data) {
    std::vector<ValueType> sorted_words(std::move(id_data));
    sorted_words.shrink_to_fit();

    const auto data = char_data_.data();
    std::sort(sorted_words.begin(), sorted_words.end(),
              [data](const ValueType& a, const ValueType& b) {
                return str_less_than(get_c_str(data, a), get_c_str(data, b));
              });

    return std::move(sorted_words);
  }

  static std::vector<uint32_t> create_id_map(
      const std::vector<ValueType>& sorted_words_) {
    std::vector<uint32_t> id_map;

    const uint32_t len = static_cast<uint32_t>(sorted_words_.size());
    for (uint32_t i = 0; i < len; i++) {
      size_t index = static_cast<size_t>(sorted_words_[i].second);
      if (index < id_map.size()) {
        id_map[index] = i;
      } else {
        // fill the gaps with end()
        for (size_t i = id_map.size(); i < index; i++) {
          id_map.push_back(len);
        }
        id_map.push_back(i);
      }
    }

    id_map.shrink_to_fit();

    return id_map;
  }

public:
  explicit StringIdMap(Builder&& builder) {
    char_data_ = std::move(builder.char_data_);
    char_data_.shrink_to_fit();

    sorted_words_ =
        std::move(create_sorted_words(char_data_, std::move(builder.id_data_)));

    id_map_ = std::move(create_id_map(sorted_words_));
  }
  StringIdMap(const StringIdMap&) = delete;

public:
  size_t size() const {
    return sorted_words_.size();
  }
  bool empty() const {
    return size() == 0;
  }

  const_iterator begin() const {
    return sorted_words_.begin();
  }
  const_iterator end() const {
    return sorted_words_.end();
  }

  const_iterator find_for_id(Id id) const {
    size_t index = static_cast<size_t>(id);
    if (index < id_map_.size()) {
      auto it = begin();
      std::advance(it, id_map_[index]);
      return it;
    }
    return end();
  }

  const_iterator find_for_word(const char* word) const {
    // see the following for the implementation:
    // https://en.cppreference.com/w/cpp/algorithm/binary_search#Possible_implementation

    const auto data = char_data_.data();

    typedef const char* CharPtr;
    auto it = std::lower_bound(begin(), end(), word,
                               [data](const ValueType& a, const CharPtr& b) {
                                 return str_less_than(get_c_str(data, a), b);
                               });

    if (!(it == end()) && !str_less_than(word, get_c_str(data, *it))) {
      return it;
    }
    return end();
  }
  const_iterator find_for_word(const std::string& str) const {
    return find_for_word(str.c_str());
  }

  const char* get_c_str(const string_entry& entry) const {
    return get_c_str(char_data_.data(), entry);
  }
  const char* get_c_str(const ValueType& value) const {
    return get_c_str(char_data_.data(), value);
  }

private:
  // This stores the raw character data.
  // All words are terminated by a NUL character.
  std::vector<char> char_data_;
  // This stores the `const char*` (compressed as an uint32_t) and the id for
  // the word.
  std::vector<ValueType> sorted_words_;
  // This stores the index in `sorted_words_` each id maps to.
  // Gaps are filled with `sorted_words_.size()`.
  std::vector<uint32_t> id_map_;
};


} // namespace util
} // namespace netspeak

#endif
