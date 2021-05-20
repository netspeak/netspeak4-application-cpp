#include "cli/BuildCommand.hpp"

#include "boost/filesystem.hpp"

#include "netspeak/indexing.hpp"

namespace cli {

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

std::string BuildCommand::desc() {
  return "Builds a new Netspeak index from an n-gram collection.\n"
         "\n"
         "If the n-gram collection is not unique (= there are duplicate "
         "phrases), then those duplicates have to be merged with the "
         "`--merge-duplicates` option.";
};

void BuildCommand::add_options(
    boost::program_options::options_description_easy_init& easy_init) {
  easy_init("in,i", bpo::value<std::string>()->required(), "Input directory");
  easy_init("out,o", bpo::value<std::string>()->required(),
            "The output directory (must be empty). If the directory doesn't "
            "exist, it will be created.");
  easy_init("merge-duplicates,m",
            "If the n-gram collection contains duplicates, they have to be "
            "merged or else the build will fail.\n"
            "\n"
            "This will create a complete copy of the n-gram collection during "
            "the merging process and then in-place merge the copy. The "
            "location of the copy can be set using the `--merge-dir` option.");
  easy_init("merge-dir", bpo::value<std::string>(),
            "The directory used to store the merged (aka unique) n-gram "
            "collection.\n\n"
            "This defaults to a temporary directory in the `--out` direcotry.");
}

// -----------------------------------------------------------------------------
// Builds netspeak index with given command line options.
// An n-gram collection consists of a set of n-gram files, whose lines must be
// formatted as follows (other formats are not supported):
//
// <n-gram> TAB <n-gram-frequency>
//
// The separator between the n-gram and its frequency is a single tab ('\t').
// The separator between the n-grams words is a single whitespace (' ').
// Example:
//
// hello world 123
// -----------------------------------------------------------------------------
int BuildCommand::run(boost::program_options::variables_map variables) {
  const std::string org_input_dir = variables["in"].as<std::string>();
  const std::string org_output_dir = variables["out"].as<std::string>();

  bfs::path input_dir(org_input_dir);
  bfs::path output_dir(org_output_dir);

  if (variables.count("merge-duplicates")) {
    // if keys have to be merged and no merge dir was given. Create 'tmp'
    // dir in dst_folder
    bfs::path tmp_dir =
        variables.count("merge-dir")
            ? bfs::path(variables["merge-dir"].as<std::string>())
            : output_dir / bfs::path("tmp");

    netspeak::MergeDuplicates(input_dir, tmp_dir);
    input_dir = tmp_dir;
    output_dir /= "tmp-index";
  }

  netspeak::BuildNetspeak(input_dir, output_dir);
  if (org_input_dir != input_dir.c_str()) {
    if (input_dir.parent_path() == org_input_dir) {
      bfs::remove_all(input_dir);
    }
    bfs::rename(output_dir, bfs::path(org_output_dir));
  }

  return EXIT_SUCCESS;
}

} // namespace cli
