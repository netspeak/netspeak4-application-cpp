#include "netspeak/model/Words.hpp"


namespace netspeak {
namespace model {


bool Words::is_subsequence_of(const Words& words) const {
  return is_subsequence_of(words.data());
}
bool Words::is_subsequence_of(const std::vector<Word>& words) const {
  std::ptrdiff_t _size = size();
  for (auto it = words.begin(); it != words.end(); ++it) {
    if (_size > std::distance(it, words.end())) {
      break;
    }

    if (std::equal(begin(), end(), it)) {
      return true;
    }
  }

  return false;
}

std::ostream& operator<<(std::ostream& out, const Words& words) {
  const auto& data = words.data();

  auto it = data.begin();
  auto end = data.end();
  if (it == end) {
    out << "<empty>";
  } else {
    out << '"' << *it << '"';
    it++;
    for (; it != end; it++) {
      out << " " << '"' << *it << '"';
    }
  }

  return out;
}

} // namespace model
} // namespace netspeak
