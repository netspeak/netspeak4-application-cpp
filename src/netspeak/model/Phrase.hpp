#ifndef NETSPEAK_MODEL_PHRASE_HPP
#define NETSPEAK_MODEL_PHRASE_HPP

#include <ostream>
#include <string>
#include <vector>

#include "netspeak/model/Words.hpp"
#include "netspeak/model/typedefs.hpp"


namespace netspeak {
namespace model {

/**
 * A data container for the global id of phrase.
 *
 * The global id of a phrase is the combination of the local id of the phrase (=
 * the id within a specific n-gram class) and the length of the phrase (= the
 * number of words = the n-gram class of the phrase).
 */
struct PhraseId__ {
public:
  typedef uint32_t Length;
  typedef __PhraseLocalId Local;

private:
  Length length_;
  Local local_;

public:
  PhraseId__() = delete;
  PhraseId__(Length len, Local local_id) : length_(len), local_(local_id) {}

  Length length() const {
    return length_;
  }
  Local local() const {
    return local_;
  }

  operator uint64_t() const {
    return (((uint64_t)length_) << 32) | local_;
  }

  inline bool operator==(const PhraseId__& rhs) const {
    return local() == rhs.local() && length() == rhs.length();
  }
  inline bool operator!=(const PhraseId__& rhs) const {
    return !(*this == rhs);
  }
  inline bool operator<(const PhraseId__& rhs) const {
    // This operation is equivalent to:
    //   ((uint64_t)*this) < ((uint64_t)rhs)
    if (length() != rhs.length()) {
      return length() < rhs.length();
    } else {
      return local() < rhs.local();
    }
  }
  inline bool operator<=(const PhraseId__& rhs) const {
    return *this < rhs || *this == rhs;
  }
  inline bool operator>(const PhraseId__& rhs) const {
    return !(*this <= rhs);
  }
  inline bool operator>=(const PhraseId__& rhs) const {
    return !(*this < rhs);
  }
};


class Phrase {
public:
  typedef PhraseId__ Id;
  typedef uint64_t Frequency;

  struct Count {
    typedef uint64_t Global;
    typedef uint32_t Local;
    typedef uint32_t Length;
  };

private:
  Id id_;
  Frequency freq_;
  Words words_;

public:
  Phrase() = delete;
  Phrase(Id id, Frequency freq) : id_(id), freq_(freq), words_() {}

  Id id() const {
    return id_;
  }
  Frequency freq() const {
    return freq_;
  }

  const Words& words() const {
    return words_;
  }
  Words& words() {
    return words_;
  }

  inline bool operator==(const Phrase& rhs) const {
    // The frequency and words of the phrase are both derived from the phrase
    // id, they don't hold any more information.
    return id() == rhs.id();
  }
  inline bool operator!=(const Phrase& rhs) const {
    return !(*this == rhs);
  }
  inline bool operator<(const Phrase& rhs) const {
    if (freq() != rhs.freq()) {
      // Phrases are naturally sorted by descending frequency.
      return freq() > rhs.freq();
    } else {
      // If the frequency is that same, then we will sort by id. The order of
      // ids within a frequency class doesn't really matter, so we will just go
      // with their natural order.
      return id() < rhs.id();
    }
  }
  inline bool operator<=(const Phrase& rhs) const {
    return *this < rhs || *this == rhs;
  }
  inline bool operator>(const Phrase& rhs) const {
    return !(*this <= rhs);
  }
  inline bool operator>=(const Phrase& rhs) const {
    return !(*this < rhs);
  }
};


std::ostream& operator<<(std::ostream& out, const Phrase::Id& id);
std::ostream& operator<<(std::ostream& out, const Phrase& phrase);


} // namespace model
} // namespace netspeak


#endif
