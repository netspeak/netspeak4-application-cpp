#include "cli/ShellCommand.hpp"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <chrono>
#include <cstdio>
#include <iomanip>

#include "boost/filesystem.hpp"

#include "cli/util.hpp"

#include "netspeak/Configuration.hpp"
#include "netspeak/Netspeak.hpp"
#include "netspeak/service/NetspeakService.grpc.pb.h"
#include "netspeak/util/service.hpp"

namespace cli {

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
using namespace netspeak;

std::string ShellCommand::desc() {
  return "Load an interactive shell for a Netspeak index or server.\n"
         "\n"
         "This supports multiple input methods. You load an index "
         "configuration file using `--config`, load an index directly using "
         "`--in`, or connect to a Netspeak server using `--source`.\n"
         "\n"
         "You have to provide exactly one of `--config`, `--in`, or "
         "`--source`. Providing none or multiple of this arguments is "
         "invalid.\n"
         "\n"
         "Examples:\n"
         "\n"
         "Loading a local Netspeak index (without configuration):\n"
         "    netspeak4 shell --in \"/path/to/index\"\n"
         "\n"
         "Loading an index configuration file:\n"
         "    netspeak4 shell --config \"/path/to/config.properties\"\n"
         "\n"
         "Connect to a Netspeak server:\n"
         "    netspeak4 shell --source web-en.api.netspeak.org:7883\n"
         "    netspeak4 shell --source localhost:1234\n"
         "    netspeak4 shell --source localhost:1234 --corpus web-en\n";
};

void ShellCommand::add_options(
    boost::program_options::options_description_easy_init& easy_init) {
  easy_init("in,i", bpo::value<std::string>(),
            "Directory containing a Netspeak index.");
  easy_init(
      "config,c", bpo::value<std::string>(),
      "A .properties file describing the configuration of a Netspeak index.");
  easy_init("source,s", bpo::value<std::string>(),
            "The address of a Netspeak gRPC server.");
  easy_init("corpus", bpo::value<std::string>(),
            "The corpus key used for all requests to a Netspeak server.\n"
            "\n"
            "This can only be used in combination with the `--source` option.");
}

typedef std::function<void(const service::SearchRequest&,
                           service::SearchResponse&)>
    Searcher;

void RunShell(Searcher& searcher) {
  std::cout << "Enter a query (type 'q' to exit).\n";

  const uint32_t max_phrases_request = 200;
  const uint32_t max_phrases_display = 50;

  // Set up a Netspeak request.
  service::SearchRequest request;
  request.set_max_phrases(max_phrases_request);

  while (true) {
    std::string query;
    std::cout << "\n" << FG_BRIGHT_BLACK << ">>>" << RESET << " ";
    std::getline(std::cin, query);
    if (query == "q") {
      std::cout << "Shutting down. This might take a while." << std::endl;
      break;
    }

    request.set_query(query);

    // Search Netspeak. This will never throw an exception.
    service::SearchResponse response;
    const auto time_start = std::chrono::steady_clock::now();
    try {
      searcher(request, response);
    } catch (std::exception& e) {
      std::cerr << "Error during search: " << e.what() << "\n";
      continue;
    }
    const auto duration = std::chrono::steady_clock::now() - time_start;

    if (response.has_result()) {
      const auto& result = response.result();

      // print some meta data about the response
      std::cout << FG_BRIGHT_BLACK;
      std::cout << result.phrases().size()
                << " phrase(s) (limit=" << max_phrases_request << ") took "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       duration)
                       .count()
                << "ms";
      if (!result.unknown_words().empty()) {
        std::cout << " (" << result.unknown_words().size()
                  << " unknown word(s):";
        for (const auto& unknown_word : result.unknown_words()) {
          std::cout << " \"" << unknown_word << "\"";
        }
        std::cout << ")";
      }
      std::cout << RESET << std::endl;
      std::cout << std::endl;

      // print all retrieved phrases
      size_t i = 1;
      std::cout << "Rank: Frequency    Text" << std::endl;

      uint32_t phrase_counter = 0;
      for (const auto& phrase : result.phrases()) {
        if (phrase_counter >= max_phrases_display) {
          break;
        }
        phrase_counter++;

        std::cout << std::right << std::setw(4) << i++ << ": " << std::left
                  << std::setw(12) << phrase.frequency();
        for (const auto& word : phrase.words()) {
          std::cout << " ";

          // add some color
          const auto tag = word.tag();
          switch (tag) {
            case service::Phrase::Word::WORD_FOR_QMARK:
            case service::Phrase::Word::WORD_FOR_STAR:
            case service::Phrase::Word::WORD_FOR_PLUS:
            case service::Phrase::Word::WORD_FOR_REGEX:
              std::cout << FG_RED;
              break;
            case service::Phrase::Word::WORD_IN_DICTSET:
            case service::Phrase::Word::WORD_IN_OPTIONSET:
            case service::Phrase::Word::WORD_IN_ORDERSET:
              std::cout << FG_BLUE;
              break;
            default:
              break;
          }

          // print word
          std::cout << word.text();

          // reset colors
          std::cout << RESET;
        }
        std::cout << std::endl;
      }

      if ((int)phrase_counter != result.phrases().size()) {
        std::cout << "and " << (result.phrases().size() - phrase_counter)
                  << " more phrase(s)\n";
      }
    } else {
      const auto& error = response.error();

      // print error
      std::cout << "Error: "
                << service::SearchResponse::Error::Kind_Name(error.kind())
                << ": " << error.message() << std::endl;
    }
    std::cout.flush();
  }
}

/**
 * Runs an interactive Netspeak with the default retrieval strategy.
 */
void RunNetspeakShell(const Configuration& config) {
  Netspeak netspeak;
  try {
    netspeak.initialize(config);
  } catch (const std::exception& error) {
    netspeak::util::log(error.what());
    return;
  }

  Searcher searcher = [&](const service::SearchRequest& request,
                          service::SearchResponse& response) {
    netspeak.search(request, response);
  };
  RunShell(searcher);
}

void handle_corpus_key(std::string& corpus_key,
                       netspeak::service::NetspeakService::Stub& stub) {
  service::CorporaResponse corporaRes = getCorpora(stub);
  const auto& corpora = corporaRes.corpora();
  if (corpus_key.empty()) {
    // select key

    if (corpora.empty()) {
      throw std::logic_error("The server doesn't support any corpora.");
    } else if (corpora.size() == 1) {
      const auto& corpus = *corpora.begin();
      std::cout << "The corpus " << corpus
                << " has been selected because it's the only corpus.\n";
      corpus_key = corpus.key();
    } else {
      std::cout << "The server supports multiple corpora. Please choose one by "
                   "entering its number:\n";
      int i = 0;
      for (const auto& corpus : corpora) {
        std::cout << "\t" << (i++) << ": " << corpus << "\n";
      }
      int index;
      std::cin >> index;
      if (index < 0 || index >= corpora.size()) {
        throw std::logic_error("Number out of range.");
      }

      const auto& corpus = corporaRes.corpora(index);
      std::cout << "The corpus " << corpus << " has been selected.\n";
      corpus_key = corpus.key();
    }
  } else {
    // verify key

    if (std::find_if(corpora.begin(), corpora.end(),
                     [&](const service::Corpus& corpus) {
                       return corpus.key() == corpus_key;
                     }) == corpora.end()) {
      std::stringstream what;
      what << "Cannot find the given corpus key \"" << corpus_key
           << "\" in the list of corpora support by the server.\n"
           << "Supported are:";
      for (const auto& corpus : corpora) {
        what << "\n\t" << corpus;
      }
      throw std::logic_error(what.str());
    }
  }
}
void RunLocalServerShell(const std::string& address, std::string corpus_key) {
  auto channel =
      grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
  auto stub = service::NetspeakService::NewStub(channel);

  handle_corpus_key(corpus_key, *stub);

  Searcher searcher = [&](const service::SearchRequest& request,
                          service::SearchResponse& response) {
    service::SearchRequest req(request);
    req.set_corpus(corpus_key);

    grpc::ClientContext context;
    auto status = stub->Search(&context, req, &response);
    if (!status.ok()) {
      std::stringstream what;
      what << status;
      throw std::logic_error(what.str());
    }
  };
  RunShell(searcher);
}

int ShellCommand::run(boost::program_options::variables_map variables) {
  Configuration config;

  size_t input_options = 0;
  if (variables.count("in") != 0)
    input_options++;
  if (variables.count("config") != 0)
    input_options++;
  if (variables.count("source") != 0)
    input_options++;

  if (input_options != 1) {
    std::stringstream msg;
    msg << "You have to provide exactly one of `--config`, `--in`, or "
           "`--source`.\n"
        << "You provided " << input_options << " options.";
    throw std::runtime_error(msg.str());
  }

  if (variables.count("in") != 0) {
    config = {
      { Configuration::PATH_TO_HOME, variables["in"].as<std::string>() },
      { Configuration::CACHE_CAPACITY, "1000" },
    };
  } else if (variables.count("config") != 0) {
    config = Configuration(variables["config"].as<std::string>());
  } else /* if (variables.count("source") != 0) */ {
    const auto& address = variables["source"].as<std::string>();
    std::string corpus_key;
    if (variables.count("corpus") != 0) {
      corpus_key = variables["corpus"].as<std::string>();
    }

    RunLocalServerShell(address, corpus_key);
    return EXIT_SUCCESS;
  }

  RunNetspeakShell(config);
  return EXIT_SUCCESS;
}

} // namespace cli
