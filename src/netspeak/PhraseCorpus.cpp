#include "netspeak/PhraseCorpus.hpp"

#include <aio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <algorithm>
#include <string>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include "netspeak/error.hpp"
#include "netspeak/invertedindex/ByteBuffer.hpp"
#include "netspeak/util/check.hpp"
#include "netspeak/value/value_traits.hpp"


namespace netspeak {

namespace bfs = boost::filesystem;
using namespace model;

const std::string PhraseCorpus::txt_dir("txt");
const std::string PhraseCorpus::bin_dir("bin");
const std::string PhraseCorpus::vocab_file("vocab");
const std::string PhraseCorpus::phrase_file("phrases");

PhraseCorpus::PhraseCorpus() : max_length_(0) {}

PhraseCorpus::PhraseCorpus(const bfs::path& phrase_dir) {
  open(phrase_dir);
}

/**
 * @brief Returns the size in bytes each entry in the binary file of the given
 * length (= n-gram class).
 *
 * You can think of each `phrase-corpus/bin/phrase.{n}` file as an array. This
 * function will return the size of each entry given the n-gram class. The
 * return size will be constant within one n-gram class but will be different
 * for different n-gram classes.
 *
 * @param length
 * @return size_t
 */
size_t entry_size(Phrase::Id::Length length) {
  return sizeof(Phrase::Frequency) + sizeof(WordId) * (size_t)length;
}

Phrase::Count::Local PhraseCorpus::count_phrases(
    Phrase::Id::Length phrase_len) const {
  const auto it_fd = fd_map.find(phrase_len);

  if (it_fd != fd_map.end()) {
    struct stat stats;
    ::fstat(it_fd->second, &stats);
    return stats.st_size / entry_size(phrase_len);
  }

  return 0;
}

size_t PhraseCorpus::count_vocabulary() const {
  return id_map->size();
}

bool PhraseCorpus::is_open() const {
  return !id_map->empty();
}

bool PhraseCorpus::contains(const std::string& word) const {
  auto it = id_map->find_for_word(word);
  return it != id_map->end();
}
bool PhraseCorpus::contains(const WordId& id) const {
  auto it = id_map->find_for_id(id);
  return it != id_map->end();
}

void PhraseCorpus::open(const bfs::path& phrase_dir) {
  init_vocabulary_(phrase_dir / vocab_file);
  open_phrase_files_(phrase_dir);
}

std::vector<Phrase> PhraseCorpus::read_phrases(
    const std::vector<Phrase::Id>& phrase_ids) const {
  // The gist of this method is the following:
  // We use async IO to read all the raw phrases data in parallel into memory
  // and then we decode the raw data sequentially. This is a lot faster than
  // using sync IO.
  //
  // Instead of making many small buffers, we will go through all phrase ids
  // once and figure out how much memory we need in total and allocate that in
  // one block.

  size_t total_mem = 0;
  for (const auto& id : phrase_ids) {
    total_mem += entry_size(id.length());
  }

  // allocate buffer
  std::unique_ptr<char[]> buffer(new char[total_mem]);

  const size_t count = phrase_ids.size();

  // prepare objects for async IO
  std::vector<aiocb> aio_read_objs(count);
  std::vector<aiocb*> aio_read_ptrs(count);
  std::memset(aio_read_objs.data(), 0, count * sizeof(aiocb));

  size_t buffer_pos = 0;
  for (size_t i = 0; i < count; ++i) {
    const auto id = phrase_ids[i];
    const auto size = entry_size(id.length());
    const auto fd_it = fd_map.find(id.length());
    util::check(fd_it != fd_map.end(), __func__);

    aio_read_objs[i].aio_buf = &(buffer[buffer_pos]);
    aio_read_objs[i].aio_nbytes = size;
    aio_read_objs[i].aio_fildes = fd_it->second;
    aio_read_objs[i].aio_offset = id.local() * size;
    aio_read_objs[i].aio_lio_opcode = LIO_READ;
    aio_read_ptrs[i] = &aio_read_objs[i];

    buffer_pos += size;
  }

  util::check(::lio_listio(LIO_WAIT, aio_read_ptrs.data(), count, NULL) != -1,
              __func__, "lio_listio failed");

  std::vector<Phrase> phrases;
  phrases.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    const auto aio_ptr = aio_read_ptrs[i];
    util::check(
        ::aio_return(aio_ptr) == static_cast<ssize_t>(aio_ptr->aio_nbytes),
        __func__, "aio_read64 failed");

    phrases.push_back(decode_((const char*)aio_ptr->aio_buf, phrase_ids[i]));
  }
  return phrases;
}

using namespace value;

Phrase PhraseCorpus::decode_(const char* buffer, Phrase::Id id) const {
  Phrase::Frequency freq;
  buffer = value_traits<Phrase::Frequency>::copy_from(freq, buffer);

  Phrase phrase(id, freq);

  const auto end = buffer + sizeof(WordId) * (size_t)id.length();
  WordId word_id;
  while (buffer != end) {
    buffer = value_traits<WordId>::copy_from(word_id, buffer);
    std::string word(id_map->get_c_str(*(id_map->find_for_id(word_id))));
    phrase.words().push_back(std::move(word));
  }

  return phrase;
}

void PhraseCorpus::init_vocabulary_(const bfs::path& vocab_file) {
  bfs::ifstream ifs(vocab_file);
  util::check(ifs.is_open(), error_message::cannot_open, vocab_file);

  std::string word;
  WordId word_id;
  util::StringIdMap<WordId>::Builder builder;
  while (ifs >> word >> word_id) {
    builder.append(word, word_id);
  }

  id_map.emplace(std::move(builder));
}

boost::optional<Phrase::Id::Length> parse_phrase_filename(
    const std::string& name) {
  // we only want to open files of the form `phrases.{n}`
  if (boost::starts_with(name, PhraseCorpus::phrase_file)) {
    std::string::size_type dotpos = name.rfind('.');
    if (dotpos != std::string::npos) {
      // TODO: check that the dot is at the right position
      return boost::lexical_cast<Phrase::Id::Length>(name.substr(++dotpos));
    }
  }

  // incorrect format
  return boost::optional<Phrase::Id::Length>{};
}

void PhraseCorpus::open_phrase_files_(const bfs::path& phrase_dir) {
  bfs::directory_iterator end;
  Phrase::Id::Length max = 0;

  for (bfs::directory_iterator it(phrase_dir); it != end; ++it) {
    const auto& path = it->path();
    const auto filename = path.filename().string();
    const auto phrase_len = parse_phrase_filename(filename);

    if (phrase_len) {
      util::FileDescriptor fd(::open(path.string().c_str(), O_RDONLY));
      util::check(fd != -1, error_message::cannot_open, path);

      fd_map.insert(std::make_pair(*phrase_len, fd));
      max = std::max(max, *phrase_len);
    }
  }

  max_length_ = max;
}

} // namespace netspeak
