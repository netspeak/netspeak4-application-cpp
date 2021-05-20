#ifndef NETSPEAK_MODEL_TYPEDEFS_HPP
#define NETSPEAK_MODEL_TYPEDEFS_HPP

#include <cstdint>

#include "netspeak/value/pair.hpp"

namespace netspeak {
namespace model {


// The local (= within a specific n-gram class) id of a phrase.
// Note: This is a purely internal type. Use Phrase::Id::Local instead.
typedef uint32_t __PhraseLocalId;

// The type of the id used to represent a single unique word.
typedef uint32_t WordId;


// The type of a phrases frequency used by the phrase index (Searcher).
typedef uint32_t PhraseIndexPhraseFreq;

// The type of entries in the phrase index.
typedef value::pair<PhraseIndexPhraseFreq, __PhraseLocalId> PhraseIndexValue;

// The type of entries in the postlist index.
// Semantic: (index-of-phrase-index-postlist-entry, phrase-frequency)
typedef value::pair<uint32_t, PhraseIndexPhraseFreq> PostlistIndexValue;


} // namespace model
} // namespace netspeak

#endif
