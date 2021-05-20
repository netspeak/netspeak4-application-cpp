#ifndef NETSPEAK_PHRASE_FILE_PARSER_HPP
#define NETSPEAK_PHRASE_FILE_PARSER_HPP

#include <istream>
#include <ostream>

#include "netspeak/model/Phrase.hpp"
#include "netspeak/model/Words.hpp"
#include "netspeak/util/string.hpp"

namespace netspeak {

struct PhraseFileParserItem {
  model::Phrase::Id::Local id;
  model::Phrase::Frequency freq;
  model::Words words;
};

/**
 * Parses text files in the ngram-file format, which defines a line as follows:
 * <ngram> TAB <frequency>
 *
 * Optionally there can be an additional value per line to assign some phrase
 * id (to set as template parameter). The line format then reads as follows:
 * <ngram> TAB <frequency> TAB <id>
 */
template <bool stream_provides_phrase_id>
class PhraseFileParser {
public:
  PhraseFileParser(std::istream& is) : is_(is), id_offset_(0) {}

  PhraseFileParser(std::istream& is, model::Phrase::Id::Local id_offset)
      : is_(is), id_offset_(id_offset) {}

  virtual ~PhraseFileParser() {}

  bool good() const {
    return is_.good();
  }

  size_t tellg() {
    return is_.tellg();
  }

  bool read_next(PhraseFileParserItem& res) {
    read(is_, res);
    if (!stream_provides_phrase_id) {
      res.id = ++id_offset_;
    }
    return is_.good();
  }

  static std::ostream& write(std::ostream& out,
                             const PhraseFileParserItem& res) {
    // words
    auto it = res.words.begin();
    auto end = res.words.end();
    if (it != end) {
      out << *it;
      it++;
    }
    for (; it != end; it++) {
      out << ' ' << *it;
    }

    // frequency
    out << '\t' << res.freq;

    // id
    if (stream_provides_phrase_id) {
      out << '\t' << res.id;
    }

    return out << '\n';
  }
  static std::istream& read(std::istream& is, PhraseFileParserItem& res) {
    res.words.data().clear();
    std::string line, word;
    std::string::size_type tabpos;
    while (std::getline(is, line)) {
      util::check_valid_utf8(line);
      tabpos = line.find('\t');
      if (tabpos != std::string::npos) {
        std::istringstream iss(line.substr(0, tabpos));
        while (iss >> word) {
          res.words.push_back(word);
        }
        iss.clear();
        iss.str(line.substr(tabpos));
        iss >> res.freq;
        if (stream_provides_phrase_id) {
          iss >> res.id;
        }
        return is;
      }
    }
    return is;
  }

private:
  std::istream& is_;
  model::Phrase::Id::Local id_offset_;
};

} // namespace netspeak

#endif // NETSPEAK_PHRASE_FILE_PARSER_HPP
