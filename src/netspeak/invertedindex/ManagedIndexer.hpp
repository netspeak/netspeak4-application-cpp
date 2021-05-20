// ManagedIndexer.hpp -*- C++ -*-
// Copyright (C) 2011-2013 Martin Trenkmann
#ifndef NETSPEAK_INVERTEDINDEX_MANAGED_INDEXER_HPP
#define NETSPEAK_INVERTEDINDEX_MANAGED_INDEXER_HPP

#include <boost/filesystem/fstream.hpp>

#include "netspeak/invertedindex/Indexer.hpp"
#include "netspeak/invertedindex/InvertedFileReader.hpp"
#include "netspeak/invertedindex/Properties.hpp"
#include "netspeak/invertedindex/Record.hpp"
#include "netspeak/util/exception.hpp"

namespace netspeak {
namespace invertedindex {

namespace bfs = boost::filesystem;

/**
 * A class to create an inverted index from input files.
 */
template <typename T, typename Reader = InvertedFileReader<T> >
class ManagedIndexer {
public:
  typedef Record<T> RecordType;
  typedef Indexer<T> IndexerType;

  static const Properties index(const Configuration& config) {
    uint64_t total_input_size = 0;
    uint64_t current_input_size = 0;
    uint64_t current_record_count = 0;

    const auto read_file = [&](const bfs::path& file, IndexerType& indexer) {
      if (bfs::is_regular_file(file) && !util::is_hidden_file(file)) {
        bfs::ifstream ifs(file);
        if (!ifs) {
          util::throw_invalid_argument("Cannot open", file);
        }
        RecordType record;
        Reader reader(ifs);
        util::log("Processing", file);
        while (reader.read(record)) {
          try {
            indexer.insert(record);
            if (!(++current_record_count % 10000000)) {
              // Estimate the total number of records.
              const double avg_record_size =
                  (current_input_size + reader.tell()) /
                  static_cast<double>(current_record_count);
              const uint64_t total_record_count =
                  total_input_size / avg_record_size;
              indexer.set_expected_record_count(total_record_count);
            }
          } catch (std::exception& e) {
            util::log("Indexer::insert failed", e.what());
          }
        }
        current_input_size += reader.tell();
      }
    };

    IndexerType indexer(config);
    if (!config.input_file().empty() && !config.input_directory().empty()) {
      util::throw_invalid_argument(
          "Specify either an input file or a directory, not both");
    }
    if (!config.input_file().empty()) {
      const bfs::path input_file = config.input_file();
      if (!bfs::exists(input_file)) {
        util::throw_runtime_error("Does not exist", input_file);
      }
      if (!bfs::is_regular_file(input_file)) {
        util::throw_runtime_error("No regular file", input_file);
      }
      total_input_size = bfs::file_size(input_file);
      read_file(input_file, indexer);
    } else if (!config.input_directory().empty()) {
      const bfs::path input_dir = config.input_directory();
      if (!bfs::exists(input_dir)) {
        util::throw_runtime_error("Does not exist", input_dir);
      }
      if (!bfs::is_directory(input_dir)) {
        util::throw_runtime_error("Not a directory", input_dir);
      }
      total_input_size = util::directory_size(input_dir);
      const bfs::directory_iterator end;
      for (bfs::directory_iterator it(input_dir); it != end; ++it) {
        read_file(it->path(), indexer);
      }
    } else {
      util::throw_invalid_argument("No input file or directory specified");
    }
    return indexer.index();
  }
};

} // namespace invertedindex
} // namespace netspeak

#endif // NETSPEAK_INVERTEDINDEX_MANAGED_INDEXER_HPP
