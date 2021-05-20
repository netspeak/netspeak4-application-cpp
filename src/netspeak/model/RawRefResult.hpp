#ifndef NETSPEAK_MODEL_RAW_REF_RESULT_HPP
#define NETSPEAK_MODEL_RAW_REF_RESULT_HPP

#include <memory>
#include <string>
#include <vector>

#include "netspeak/model/Phrase.hpp"


namespace netspeak {
namespace model {


/**
 * @brief A result set of phrase references of a single norm query.
 *
 * The set and its phrase references don't hold any information about its norm
 * query (e.g. the norm query itself or its length). A result sets can only be
 * used, if the norm query that created it (or an equivalent norm query) is
 * known.
 */
class RawRefResult {
public:
  class Ref {
  public:
    typedef uint32_t Frequency;

  private:
    Phrase::Id::Local id_;
    Frequency freq_;

  public:
    Ref() = delete;
    Ref(Phrase::Id::Local id, Frequency freq) : id_(id), freq_(freq) {}

    /**
     * The local id of the referenced phrase.
     */
    Phrase::Id::Local id() const {
      return id_;
    }
    /**
     * A bounded frequency of the referenced phrase.
     *
     * This frequency is bounded by the underlying integer type and may not be
     * the exact frequency. Only very few phrases (most only stop words) will
     * have frequencies great enough to be affect by this.
     */
    Frequency freq() const {
      return freq_;
    }


    bool operator==(const Ref& rhs) const {
      // we don't need to compare the frequency because a phrase is uniquely
      // identified by its local id. The frequency is computed (more like looked
      // up) from the id and therefore doesn't hold any information.

      // Note: This operation is only defined for phrase references of norm
      // queries of the same length.
      return id() == rhs.id();
    }
    inline bool operator!=(const Ref& rhs) const {
      return !(*this == rhs);
    }

    // The natural ordering of references is by descending frequency.

    bool operator<(const Ref& rhs) const {
      // TODO: This definition doesn't take into account refs with the same
      // frequencies but different ids. This is a problem because functions
      // like std::union only use this operator to check equality
      // `(!(a < b) && !(b < a) => a == b)` leading to different phrase refs
      // being removed because there are seen as equal. (This was just taken
      // as is from legacy.)
      return freq() > rhs.freq();
    }
    inline bool operator<=(const Ref& rhs) const {
      return *this == rhs || *this < rhs;
    }
    inline bool operator>(const Ref& rhs) const {
      return !(*this <= rhs);
    }
    inline bool operator>=(const Ref& rhs) const {
      return !(*this < rhs);
    }
  };

private:
  std::vector<Ref> refs_;
  std::vector<std::string> unknown_words_;

public:
  /**
   * @brief The phrase references of this result set sorted by frequency in
   * descending order.
   */
  std::vector<Ref>& refs() {
    return refs_;
  }
  const std::vector<Ref>& refs() const {
    return refs_;
  }

  /**
   * @brief Returns whether this result set contains no phrase references.
   */
  bool empty() const {
    return refs_.empty();
  }

  std::vector<std::string>& unknown_words() {
    return unknown_words_;
  }
  const std::vector<std::string>& unknown_words() const {
    return unknown_words_;
  }

  /**
   * @brief Returns whether this and the given raw result set do not share any
   * common phrase references.
   *
   * The norm queries of this and the given other result set have to be
   * equivalent.
   */
  bool disjoint_with(const RawRefResult& other) const;

  /**
   * @brief Creates a new result set that contains all phrases (unknown words)
   * of this set and the given set.
   *
   * The norm queries of this and the given other result set have to be
   * equivalent.
   */
  std::shared_ptr<RawRefResult> merge(const RawRefResult& other) const;
};


} // namespace model
} // namespace netspeak


#endif
