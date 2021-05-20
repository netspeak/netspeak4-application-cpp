#include "netspeak/regex/DefaultRegexIndex.hpp"

#include <chrono>
#include <codecvt>
#include <functional>
#include <locale>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/regex.hpp>

#include "netspeak/regex/RegexIndex.hpp"
#include "netspeak/regex/RegexQuery.hpp"


namespace netspeak {
namespace regex {

/**
 * This will be a rough explanation of the implementation of the index below, a
 * few of the optimizations, and the costs of it all.
 *
 *
 * # The purpose of the index
 *
 * The index is given a _vocabulary_. This is just a simple UTF-8 string where
 * each non-empty line is interpreted as a word (can be any non-empty string).
 * The purpose of the index is to retrieve words from the vocabulary that match
 * a given regex query. The words have to be returned in the order of occurrence
 * in the vocabulary. This means that the index will and cannot do any form of
 * ranking.
 *
 * Netspeak will provide all 1-grams sorted by descending frequency as the
 * vocabulary, so ranking isn't an issue as we will assume that frequency is the
 * only ranking criteria for 1-grams.
 * Note: While Netspeak will only provide 1-grams, the index can deal with any
 * list of non-empty strings (referred to as words).
 *
 * To retrieve words matching the given query as fast as possible, we use
 * several optimizations, some more costly than others.
 *
 * Before everything else, it's important to note that this index requires the
 * full vocabulary (UTF-8) to be in-memory at all times. This means that the
 * vocabulary alone will use between 100MB and 200MB of memory.
 *
 *
 * # Word list
 *
 * The first optimization is the most important one: We keep a separate list of
 * the offset and length of every word in the vocabulary in the order of
 * occurrence (first offset is the first word and so on).
 *
 * This, among other things, allows us to match regular expressions more
 * efficiently. We can now define a string which has to match the regular
 * expression (of the query) as a whole. This is huge because the RE engine
 * doesn't have to move the matcher accross the string, potentially, character
 * by character. Instead it can jump from word to word resulting in a
 * considerable speedup (4~10x faster on average). Since we also known the exact
 * length of each word, we can reject some word based on their length alone.
 *
 * The only cost is the memory to store all the offsets and length. We need 4
 * bytes per offset and 2 bytes per length (plus some padding for memory
 * alignment by the compiler), so 8 bytes per entry. The datasets of European
 * languages have between 10e6 and 20e6 words, so the list will need between
 * 80MB and 160MB of memory which is about the size of the vocabulary itself.
 *
 *
 * # Unicode character set
 *
 * We keep a set of all Unicode characters in the vocabulary. This allows us to
 * quickly answer queries which cannot match any words in the vocabulary because
 * they require a certain character which none of the words has. It also allows
 * us to simply other queries (the idea being to eliminate parts of the query
 * which cannot match) which in turn enables further optimization.
 *
 * Example: Let's say the vocabulary does not contain upper-case letters. In
 * this case, we know that the query `Foo*` cannot match any words. The query
 * `[fF]oo` can be simplified to `foo` and similarly, `(B)ar` (`()` denotes
 * optional words) can be simplified to `ar`.
 *
 * This won't need a lot of memory, only 4 bytes per unique character in the
 * vocabulary plus the overhead of the \c std::unordered_set implementation. The
 * English dataset contains about 500 unique characters but for Asian language
 * this number much larger with around 50K unique characters. So even assuming
 * the worst-case, we can safely say that the set won't require more than about
 * 2MB of memory.
 *
 *
 * # Word hash table
 *
 * A hash table from every word to its index in the _word list_ can be used to
 * answer queries which can only match a finite number of words.
 *
 * Example: The query `[bp]et` can only every match the words "bet" and "pet",
 * so instead of searching through all words linearly, it's **a lot** faster to
 * just lookup these two words in a hash table to check whether they are part of
 * the vocabulary.
 *
 * One problem of this approach an potential exponential blowup of combination
 * (e.g. for the query `{abcdef}` (`{}` denotes a Netspeak order set) 46656
 * lookups are necessary). To keep the number of lookups (and generated strings)
 * small, the implementation will only apply the lookup strategy to queries to
 * queries with less combinatory possibilities than a certain threshold.
 *
 * Queries which contain wildcards (`?` and `*`) theoretically describe a
 * regular language with infinitely many words but practically, this number is
 * still finite because the vocabulary only has finitely many unique characters
 * (see _character set_) and a word with a maximum length. However, despite
 * this, queries with wildcard will not considered for hash lookups because
 * their combinations blow up very fast and for small number of combinations
 * (basically just one `?`), it is usually faster to use other methods.
 *
 * The hash table is implemented using linear probing, so we only need to know
 * how many 4 byte slots the table has. Since the size of the hash table n will
 * be a power of two, we will choose the next power of two of the number of
 * words w as a base. This will cause n to be 1.5 time greater than w on
 * average. However it is very important that there are enough free slot for the
 * lookup to be efficient, so if n and w are relatively close (n < w * 1.5), we
 * will double n. This means that n will be in the interval [w * 1.5, w * 3).
 * Given the datasets of European languages, this will result in  between
 * 60MB/120MB and 120MB/240MB for the hash table.
 */

uint32_t next_pow_of_2(uint32_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}

uint32_t hash_word(const std::string& str, std::string::size_type offset,
                   std::string::size_type length) {
  uint32_t h = 0x12345678U;

  // the Java hashcode implementation
  std::string::size_type end = offset + length;
  for (; offset != end; offset++) {
    h = h * 31 + str[offset];
  }

  return h;
}

bool contains_unknown_characters(
    const std::u32string& str,
    const std::unordered_set<char32_t>& known_chars) {
  for (auto it = str.begin(); it != str.end(); it++) {
    if (known_chars.find(*it) == known_chars.end()) {
      // character is not in the set of all characters
      return true;
    }
  }
  return false;
}

void regex_pattern_append_char(std::u32string& pattern, char32_t c) {
  switch (c) {
    case '+':
    case '*':
    case '?':
    case '|':
    case '\\':
    case '/':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '.':
    case '-':
    case '^':
    case '$':
      pattern.push_back('\\');
      pattern.push_back(c);
      break;
    default:
      pattern.push_back(c);
      break;
  }
}

std::string create_regex_pattern(const RegexQuery& query) {
  std::u32string pattern;

  for (const auto& unit : query.get_units()) {
    switch (unit.type) {
      case RegexUnit::Type::QMARK:
        // https://stackoverflow.com/a/13163802/7595472
        pattern.append(
            U"(?:[\\x00-\\x7F]|(?:[\\xC2-\\xDF]|\\xE0[\\xA0-\\xBF]|\\xED["
            U"\\x80-"
            U"\\x9F]|(?:[\\xE1-\\xEC]|[\\xEE-\\xEF]|\\xF0[\\x90-\\xBF]|["
            U"\\xF1-"
            U"\\xF3][\\x80-\\xBF]|\\xF4[\\x80-\\x8F])[\\x80-\\xBF])[\\x80-"
            U"\\xBF])");
        break;

      case RegexUnit::Type::STAR:
        pattern.append(U".*");
        break;

      case RegexUnit::Type::CHAR_SET: {
        pattern.append(U"(?:");
        bool first = true;
        for (const auto c : unit.value) {
          if (first) {
            first = false;
          } else {
            pattern.push_back('|');
          }
          regex_pattern_append_char(pattern, c);
        }
        pattern.append(U")");
        break;
      }

      case RegexUnit::Type::OPTIONAL_WORD:
        pattern.append(U"(?:");
        for (const auto c : unit.value) {
          regex_pattern_append_char(pattern, c);
        }
        pattern.append(U")?");
        break;

      default:
        for (const auto c : unit.value) {
          regex_pattern_append_char(pattern, c);
        }
        break;
    }
  }

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.to_bytes(pattern);
}


void DefaultRegexIndex::initialize_words() {
  words_ = std::vector<struct WordEntry>();

  // split vocabulary into words
  std::string::size_type pos;
  std::string::size_type prev = 0;
  while ((pos = vocabulary_.find('\n', prev)) != std::string::npos) {
    auto const length = pos - prev;
    if (length > 0) {
      WordEntry entry = { (uint32_t)prev, (uint16_t)length };
      words_.push_back(entry);
    }
    prev = pos + 1;
  }
  if (vocabulary_.size() > prev) {
    WordEntry entry = { (uint32_t)prev, (uint16_t)(vocabulary_.size() - prev) };
    words_.push_back(entry);
  }

  words_.shrink_to_fit();
}

void DefaultRegexIndex::initialize_all_chars() {
  // Since C++ provides no easy way to just iterate all UTF-32 characters of a
  // UTF-8 string, we use the std::string to std::u23string methods on
  // substrings of the whole vocabulary. This will keep the memory overhead
  // minimal while still being reasonably performant.

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

  // resever a few slots for good measure
  all_chars_ = std::unordered_set<char32_t>();
  all_chars_.reserve(10000);

  // one substring will have this many words.
  size_t group_size = 1024;
  for (size_t i = 0; i < words_.size(); i += group_size) {
    size_t from = (size_t)words_[i].offset;
    size_t to;
    if (i + group_size < words_.size()) {
      to = words_[i + group_size].offset;
    } else {
      to = vocabulary_.size();
    }

    const std::string& sub = vocabulary_.substr(from, to - from);
    std::u32string u32 = conv.from_bytes(sub);
    all_chars_.insert(u32.begin(), u32.end());
  }

  all_chars_.erase('\n');
  all_chars_.erase('\r');
}

void DefaultRegexIndex::initialize_word_hash_table() {
  // the hash table will be implemented via linear probing, so we need enough
  // spaces between entries.
  size_t n = next_pow_of_2(words_.size());
  if (n < words_.size() + (words_.size() / 2))
    n *= 2; // n is too small, so let's make it bigger

  word_hash_table_ = std::vector<uint32_t>(n);

  // initialize all slots of the hash table as empty
  for (size_t i = 0; i < n; i++) {
    word_hash_table_[i] = UINT32_MAX;
  }

  // insert all words
  const uint32_t mask = n - 1;
  for (uint32_t index = 0, count = words_.size(); index < count; index++) {
    const auto entry = words_[index];
    uint32_t hash = hash_word(vocabulary_, entry.offset, entry.length) & mask;

    while (true) {
      if (word_hash_table_[hash] == UINT32_MAX) {
        // empty slot
        word_hash_table_[hash] = index;
        break;
      }

      // linear probing
      hash = (hash + 1) & mask;
    }
  }
}

DefaultRegexIndex::DefaultRegexIndex(std::string vocabulary)
    : vocabulary_(std::move(vocabulary)) {
  initialize_words();
  initialize_all_chars();
  initialize_word_hash_table();
}


RegexQuery DefaultRegexIndex::optimize_query(const RegexQuery& query) const {
  RegexQueryBuilder builder;

  for (const auto& unit : query.get_units()) {
    const std::u32string& value = unit.value;

    switch (unit.type) {
      case RegexUnit::Type::WORD:
        if (contains_unknown_characters(value, all_chars_)) {
          // add the empty set causing the query to reject all words
          builder.add(RegexUnit::char_set(U""));
        } else {
          // just add the word
          builder.add(unit);
        }
        break;

      case RegexUnit::Type::OPTIONAL_WORD:
        if (contains_unknown_characters(value, all_chars_)) {
          // An optional word which contains an unknown character cannot be
          // matched, so we don't add it to the new query.
        } else {
          // just add the optional word
          builder.add(unit);
        }
        break;

      case RegexUnit::Type::CHAR_SET: {
        if (contains_unknown_characters(value, all_chars_)) {
          // remove all unknown character from the character set
          std::unordered_set<char32_t> set;
          for (const auto& c : value) {
            if (all_chars_.find(c) != all_chars_.end()) {
              set.insert(c);
            }
          }
          std::u32string new_value(set.begin(), set.end());
          builder.add(RegexUnit::char_set(new_value));
        } else {
          // just add the character set
          builder.add(unit);
        }
        break;
      }

      default:
        builder.add(unit);
        break;
    }
  }

  return builder.to_query();
}


std::string DefaultRegexIndex::word_from_entry(const WordEntry& entry) const {
  return vocabulary_.substr(entry.offset, entry.length);
}
std::string DefaultRegexIndex::word_at_index(uint32_t index) const {
  return word_from_entry(words_[index]);
}
uint32_t DefaultRegexIndex::find_word(const std::string& word) const {
  const uint32_t mask = word_hash_table_.size() - 1;
  uint32_t hash = hash_word(word, 0, word.size()) & mask;

  while (true) {
    auto index = word_hash_table_[hash];
    if (index == UINT32_MAX)
      return UINT32_MAX; // not found

    // found a candidate
    const auto entry = words_[index];

    // compare words
    if (entry.length == word.size() &&
        vocabulary_.compare(entry.offset, entry.length, word) == 0) {
      // found the given word in the vocabulary
      return index;
    }

    // the candidate isn't the given word
    hash = (hash + 1) & UINT32_MAX;
  }
}


/**
 * @brief A UTF8 encoded regex unit which describes a finite formal language.
 */
struct utf8_finite_regex_unit {
public:
  std::vector<std::string> alternatives;
  utf8_finite_regex_unit() : alternatives() {}
  utf8_finite_regex_unit(const utf8_finite_regex_unit&) = delete;
  utf8_finite_regex_unit(utf8_finite_regex_unit&&) = default;
};

std::vector<utf8_finite_regex_unit> finite_query_to_utf8(
    const RegexQuery& query) {
  std::vector<utf8_finite_regex_unit> result;

  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;

  for (const auto& unit : query.get_units()) {
    utf8_finite_regex_unit utf8_unit;

    switch (unit.type) {
      case RegexUnit::Type::CHAR_SET: {
        for (size_t i = 0; i < unit.value.size(); i++) {
          auto c = unit.value.substr(i, 1);
          utf8_unit.alternatives.push_back(conv.to_bytes(c));
        }
        break;
      }
      case RegexUnit::Type::OPTIONAL_WORD: {
        utf8_unit.alternatives.push_back(conv.to_bytes(unit.value));
        utf8_unit.alternatives.push_back("");
        break;
      }
      default: {
        utf8_unit.alternatives.push_back(conv.to_bytes(unit.value));
        break;
      }
    }

    result.push_back(std::move(utf8_unit));
  }

  return result;
}

void match_query_hash_lookup_impl(
    const std::vector<utf8_finite_regex_unit>& stack, size_t stack_index,
    std::string& word, std::vector<uint32_t>& matches,
    std::function<uint32_t(const std::string&)>& find_word) {
  if (stack_index >= stack.size()) {
    // we reached the end of the stack, so just check the word
    const uint32_t index = find_word(word);
    if (index != UINT32_MAX) {
      // found a word
      matches.push_back(index);
    }
  } else {
    // go through all combinations
    const auto& current = stack[stack_index];
    for (const std::string& alternative : current.alternatives) {
      const auto len = word.size();
      word.append(alternative);
      // recursively call the function
      match_query_hash_lookup_impl(stack, stack_index + 1, word, matches,
                                   find_word);
      word.erase(len); // remove all appended characters
    }
  }
}
void DefaultRegexIndex::match_query_hash_lookup(
    const RegexQuery& query, std::vector<std::string>& matches,
    uint32_t max_matches) const {
  const auto stack = finite_query_to_utf8(query);
  std::string temp_word;
  std::vector<uint32_t> indexes;
  std::function<uint32_t(const std::string&)> find_word_fn =
      [&](const std::string& word) { return find_word(word); };
  match_query_hash_lookup_impl(stack, 0, temp_word, indexes, find_word_fn);

  // sort indexes
  std::sort(indexes.begin(), indexes.end());

  const auto count = indexes.size();
  if (count) {
    // add words without duplicates
    uint32_t last_index = indexes[0];
    matches.push_back(word_at_index(last_index));
    uint32_t added = 1;
    for (size_t i = 1; i < count && added < max_matches; i++) {
      const auto index = indexes[i];
      if (index != last_index) {
        last_index = index;
        matches.push_back(word_at_index(last_index));
        added++;
      }
    }
  }
}

void DefaultRegexIndex::match_query_regex(
    const RegexQuery& query, std::vector<std::string>& matches,
    uint32_t max_matches, std::chrono::nanoseconds timeout) const {
  // build a regex and match it against all words.
  const boost::regex expression(create_regex_pattern(query));
  const uint16_t min_length = query.min_utf8_input_length();
  const uint16_t max_length = query.max_utf8_input_length();

  const auto start = std::chrono::steady_clock::now();
  uint32_t matches_added = 0;
  for (size_t i = 0; i < words_.size(); i++) {
    const auto entry = words_[i];

    if (entry.length < min_length || entry.length > max_length) {
      // we can reject this word based on length alone
      continue;
    }

    const auto begin = vocabulary_.begin() + entry.offset;
    const auto end = vocabulary_.begin() + (entry.offset + entry.length);
    if (boost::regex_match(begin, end, expression)) {
      // found a match
      matches.push_back(word_from_entry(entry));
      matches_added++;
      if (matches_added >= max_matches)
        break; // done
    }

    // check the time every now and then
    if (i % 256 == 0) {
      if ((std::chrono::steady_clock::now() - start) > timeout)
        break; // timeout
    }
  }
}


void DefaultRegexIndex::match_query(const RegexQuery& query,
                                    std::vector<std::string>& matches,
                                    uint32_t max_matches,
                                    std::chrono::nanoseconds timeout) const {
  // no matches are required
  if (max_matches == 0) {
    return;
  }

  const RegexQuery& q = optimize_query(query);

  if (query.accept_all_non_empty()) {
    // the query will match all words
    // Note: Technically, the empty word may be part of the index, but that's
    // unlikely because it doesn't make sense in the context of Netspeak.
    uint32_t to_add = std::min(max_matches, (uint32_t)words_.size());
    for (uint32_t i = 0; i < to_add; i++) {
      matches.push_back(word_at_index(i));
    }
    return;
  }

  if (query.reject_all()) {
    // if the query will reject all words anyway, so there's nothing to do.
    return;
  }

  if (q.combinations_upper_bound() < 1000) {
    // the query can match only finitely many words
    // We can reasonably assume that we're going to be faster than any
    // timeout, so we will just ignore it.
    match_query_hash_lookup(query, matches, max_matches);
    return;
  }

  // do regex matching
  match_query_regex(query, matches, max_matches, timeout);
}

} // namespace regex
} // namespace netspeak
