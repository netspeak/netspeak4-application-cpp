#include "cli/StressCommand.hpp"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>

#include "boost/filesystem.hpp"

#include "cli/util.hpp"

#include "netspeak/service/tracking.hpp"
#include "netspeak/util/glob.hpp"


namespace cli {

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
using namespace netspeak;

#define SOURCE_KEY "source"
#define QUERIES_KEY "queries"
#define CONCURRENCY_KEY "concurrency"
#define MAX_QUERIES_KEY "max-queries"
#define TRACKING_ID_KEY "tracking-id"

std::string StressCommand::desc() {
  return "Runs a stress test against a Netspeak gRPC server and displays "
         "performance metrics.\n"
         "\n"
         "Performance metrics will be printed every second. A typical output "
         "will look like this:\n"
         "\n"
         "    Average: 3208.34req/s   Current: 930req\n"
         "      web-de    Succ: 99.92%  Err: 0.08%  gRPC Err: 0.00%\n"
         "                avg: 2.57ms  std: 14.85ms  min: 0.23ms  max: "
         "440.84ms\n"
         "\n"
         "Let's go through this line by line: The first line the average "
         "number of requests per second that have been sent thus far (here: "
         "`3208.34req/s`) and the number of requests that have been sent in "
         "the last second (here: `930req`).\n"
         "\n"
         "The following lines always come in pair and describe network "
         "statistics and performance metrics on a per-corpus basis. The first "
         "line tells you the corpus key (here: `web-de`), the perentage of "
         "successful queries for which we got a result back (here: `Succ: "
         "99.92%`), the perentage of queries for which we got an error back "
         "(here: `Err: 0.08%`), and the perentage of queries that failed but "
         "didn't return an error response (e.g. due to network errors) (here: "
         "`gRPC Err: 0.00%`). The second line contains aggregated metrics on "
         "how long quests (successful or not) took for this corpus.\n"
         "\n"
         "Only use this on servers you own. Bombarding Netspeak servers with "
         "thousands of queries without permission is an illegal attack.";
};

void StressCommand::add_options(
    boost::program_options::options_description_easy_init& easy_init) {
  easy_init(SOURCE_KEY ",s", bpo::value<std::string>()->required(),
            "The address of a Netspeak gRPC server.");
  easy_init(QUERIES_KEY ",q", bpo::value<std::string>()->required(),
            "A file with test queries.\n"
            "\n"
            "The file is expected to have one query per line.");
  easy_init(CONCURRENCY_KEY ",c", bpo::value<uint32_t>()->default_value(1),
            "The number of concurrent requests sent to the server.");
  easy_init(MAX_QUERIES_KEY ",m", bpo::value<uint64_t>(),
            "The maximum number of queries sent to the server.\n"
            "\n"
            "If this number isn't given, the stress test will never stop "
            "sending queries.");
  easy_init(TRACKING_ID_KEY ",t", bpo::value<std::string>(),
            "A tracking id sent along every request.");
}


class Tester {
private:
  const std::vector<std::string>& queries_;
  const std::string address_;
  const std::string tracking_id_;

  std::vector<std::string> get_corpus_keys() {
    auto channel =
        grpc::CreateChannel(address_, grpc::InsecureChannelCredentials());
    auto stub = service::NetspeakService::NewStub(channel);

    std::vector<std::string> keys;
    const auto resp = getCorpora(*stub, address_);
    for (const auto& c : resp.corpora()) {
      keys.push_back(c.key());
    }

    return keys;
  }

  static grpc::Status safe_search(
      netspeak::service::NetspeakService::Stub& stub,
      grpc::ClientContext* context,
      const netspeak::service::SearchRequest& request,
      netspeak::service::SearchResponse* response) {
    try {
      return stub.Search(context, request, response);
    } catch (...) {
      return grpc::Status(grpc::StatusCode::UNKNOWN, "unknown");
    }
  }

  struct corpus_measurements {
    std::atomic<uint64_t> req_counter_;
    std::atomic<uint64_t> grpc_error_counter_;
    std::atomic<uint64_t> resp_error_counter_;
    std::atomic<uint64_t> success_counter_;

    std::vector<std::chrono::nanoseconds> durations_;
    std::mutex mutex_;

    corpus_measurements()
        : req_counter_(0),
          grpc_error_counter_(0),
          resp_error_counter_(0),
          success_counter_(0),
          durations_(),
          mutex_() {}

    void add_duration(const std::chrono::nanoseconds& dur) {
      std::lock_guard<std::mutex> lockGuard(mutex_);
      durations_.push_back(dur);
    }
  };

  struct time_analysis {
    typedef std::chrono::duration<double> sec;
    sec avg_;
    sec std_;
    sec max_;
    sec min_;

    time_analysis(sec avg, sec std, sec max, sec min)
        : avg_(avg), std_(std), max_(max), min_(min) {}
  };

  static time_analysis analyse(
      const std::vector<std::chrono::nanoseconds>& durations) {
    std::chrono::nanoseconds min(INT64_MAX);
    std::chrono::nanoseconds max(0);
    double avg = 0;

    for (const auto& d : durations) {
      min = std::min(min, d);
      max = std::max(max, d);
      avg += d.count();
    }
    avg /= durations.size();

    double var = 0;

    for (const auto& d : durations) {
      auto diff = d.count() - avg;
      var += diff * diff;
    }
    var /= durations.size();

    return time_analysis(time_analysis::sec(avg * 1e-9),
                         time_analysis::sec(std::sqrt(var) * 1e-9),
                         std::chrono::duration_cast<time_analysis::sec>(max),
                         std::chrono::duration_cast<time_analysis::sec>(min));
  }

  static double get_ms(const std::chrono::duration<double>& d) {
    return d.count() * 1000.;
  }


public:
  Tester(const std::vector<std::string>& queries, const std::string& address,
         const std::string& tracking_id)
      : queries_(queries), address_(address), tracking_id_(tracking_id) {}

  void start(const uint32_t concurrency, const uint64_t max_queries) {
    const auto corpus_keys = get_corpus_keys();
    if (corpus_keys.empty()) {
      throw std::logic_error(
          "The Netspeak server has to have at least one corpus.");
    }

    std::vector<std::unique_ptr<corpus_measurements>> measurements_list;
    for (auto i = corpus_keys.size(); i != 0; i--) {
      measurements_list.push_back(std::make_unique<corpus_measurements>());
    }

    std::atomic<uint64_t> req_counter(0);

    std::vector<std::unique_ptr<std::thread>> threads;
    for (auto i = concurrency; i != 0; i--) {
      // worker threads
      threads.push_back(std::make_unique<std::thread>([&]() {
        // setup the stub
        auto channel =
            grpc::CreateChannel(address_, grpc::InsecureChannelCredentials());
        auto stub = service::NetspeakService::NewStub(channel);

        // setup random
        std::default_random_engine rnd;
        std::uniform_int_distribution<int> keys_d(0, corpus_keys.size() - 1);
        std::uniform_int_distribution<int> queries_d(0, queries_.size() - 1);

        while (++req_counter <= max_queries) {
          grpc::ClientContext context;
          service::SearchRequest req;
          service::SearchResponse resp;

          service::set_tracking_id(context, tracking_id_);

          const auto query_index = queries_d(rnd);
          const auto corpus_keys_index = keys_d(rnd);
          auto& measurements = *measurements_list[corpus_keys_index];

          req.set_query(queries_[query_index]);
          req.set_corpus(corpus_keys[corpus_keys_index]);

          const auto start = std::chrono::steady_clock::now();
          const auto status = safe_search(*stub, &context, req, &resp);

          measurements.add_duration(std::chrono::steady_clock::now() - start);
          measurements.req_counter_++;

          if (!status.ok()) {
            measurements.grpc_error_counter_++;
          } else if (resp.has_error()) {
            measurements.resp_error_counter_++;
          } else {
            measurements.success_counter_++;
          }
        }
      }));
    }

    // display output
    uint64_t last_req_counter = 0;
    const auto display_start = std::chrono::steady_clock::now();
    while (req_counter <= max_queries) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      uint64_t curr_req_counter = req_counter;
      uint64_t diff_req_counter = curr_req_counter - last_req_counter;
      last_req_counter = curr_req_counter;

      // lock all measurements
      std::vector<std::unique_ptr<std::lock_guard<std::mutex>>> locks;
      for (auto& m : measurements_list) {
        locks.push_back(
            std::make_unique<std::lock_guard<std::mutex>>(m->mutex_));
      }

      auto seconds_since_start =
          std::chrono::duration_cast<std::chrono::duration<double>>(
              std::chrono::steady_clock::now() - display_start)
              .count();
      auto avg_req_per_s = curr_req_counter / seconds_since_start;
      auto curr_req_per_s = diff_req_counter;

      std::cout << std::fixed << std::setprecision(2)
                << "Average: " << avg_req_per_s << "req/s"
                << "\tCurrent: " << curr_req_per_s << "req" << std::endl;

      size_t key_pad = 0;
      for (const auto& key : corpus_keys) {
        key_pad = std::max(key_pad, key.size());
      }
      key_pad += 2;

      for (auto i = 0; i < corpus_keys.size(); i++) {
        const auto& key = corpus_keys[i];
        const auto& m = *measurements_list[i];

        auto total_req = static_cast<double>(m.req_counter_);
        auto success_percentage = m.success_counter_ / total_req * 100.;
        auto grpc_err_percentage = m.grpc_error_counter_ / total_req * 100.;
        auto resp_err_percentage = m.resp_error_counter_ / total_req * 100.;

        std::cout << "  " << key << std::string(key_pad - key.size(), ' ')
                  << "  Succ: " << success_percentage << "%"
                  << "  Err: " << resp_err_percentage << "%"
                  << "  gRPC Err: " << grpc_err_percentage << "%" << std::endl;

        auto dur_analysis = analyse(m.durations_);

        std::cout << "  " << std::string(key_pad, ' ')
                  << "  avg: " << get_ms(dur_analysis.avg_) << "ms"
                  << "  std: " << get_ms(dur_analysis.std_) << "ms"
                  << "  min: " << get_ms(dur_analysis.min_) << "ms"
                  << "  max: " << get_ms(dur_analysis.max_) << "ms"
                  << std::endl;
      }
    }

    // wait for all threads to die
    for (const auto& t : threads) {
      t->join();
    }
  }
};

void read_all_lines(const std::string& file_path,
                    std::vector<std::string>& lines) {
  std::ifstream file(file_path);
  std::string str;
  while (std::getline(file, str)) {
    // remove trailing \r
    if (!str.empty() && str.back() == '\r') {
      str.pop_back();
    }
    lines.push_back(str);
  }
}

int StressCommand::run(boost::program_options::variables_map variables) {
  std::vector<std::string> queries;
  read_all_lines(variables[QUERIES_KEY].as<std::string>(), queries);
  const auto concurrency = variables[CONCURRENCY_KEY].as<uint32_t>();
  const auto source = variables[SOURCE_KEY].as<std::string>();
  const auto max = variables.count(MAX_QUERIES_KEY) == 0
                       ? UINT64_MAX
                       : variables[MAX_QUERIES_KEY].as<uint64_t>();
  const auto tracking_id = variables.count(TRACKING_ID_KEY) == 0
                               ? ""
                               : variables[TRACKING_ID_KEY].as<std::string>();

  Tester tester(queries, source, tracking_id);
  tester.start(concurrency, max);

  return EXIT_SUCCESS;
}

} // namespace cli
