#include "netspeak/service/RequestLogger.hpp"

#include <chrono>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "google/protobuf/util/json_util.h"

#include "netspeak/error.hpp"
#include "netspeak/service/tracking.hpp"
#include "netspeak/util/JsonWriter.hpp"
#include "netspeak/util/systemio.hpp"


namespace netspeak {
namespace service {

namespace bfs = boost::filesystem;


std::string utc_timestamp() {
  auto t = boost::posix_time::microsec_clock::universal_time();
  std::string result = boost::posix_time::to_iso_extended_string(t);
  result.push_back('Z');
  return result;
}

std::string get_log_file_prefix() {
  const auto time = utc_timestamp();
  boost::regex re("[^a-zA-Z0-9\\-_]");
  return boost::regex_replace(time, re, "-");
}

RequestLogger::RequestLogger(std::unique_ptr<NetspeakService::Service> service,
                             bfs::path log_dir)
    : service_(std::move(service)), req_counter_(0) {
  if (!bfs::is_directory(log_dir)) {
    bfs::create_directories(log_dir);
  }

  const auto prefix = (log_dir / get_log_file_prefix()).string();

  f_search_req_.lock().value().open(prefix + "_search_req.jsonl");
  f_search_error.lock().value().open(prefix + "_search_error.jsonl");
  f_get_corpora_req_.lock().value().open(prefix + "_get_corpora_req.jsonl");
  f_get_corpora_error.lock().value().open(prefix + "_get_corpora_error.jsonl");
}

template <class S, class R>
util::JsonWriter::ObjectContinue<S, R> log_boilerplate(
    util::JsonWriter::Value<S, R> writer, uint64_t req_id,
    const std::string& user, const google::protobuf::Message& message) {
  return writer.object()
      .prop("timestamp")
      .string(utc_timestamp())
      .prop("req_id")
      .number(req_id)
      .prop("user")
      .string(user)
      .prop("message")
      .protobuf_message(message);
}
template <class S, class R>
util::JsonWriter::ObjectContinue<S, R> log_error_status(
    util::JsonWriter::ObjectContinue<S, R> writer, const grpc::Status& status) {
  return writer.prop("status")
      .object()
      .prop("code")
      .number(status.error_code())
      .prop("message")
      .string(status.error_message())
      .prop("details")
      .string(status.error_details())
      .endObject();
}

grpc::Status RequestLogger::Search(grpc::ServerContext* context,
                                   const SearchRequest* request,
                                   SearchResponse* response) {
  const auto user = get_tracking_id(*context);
  const auto req_id = req_counter_++;

  {
    std::string log_line;
    log_boilerplate(util::JsonWriter::create(log_line), req_id, user, *request)
        .endObject()
        .done();

    {
      auto lock = f_search_req_.lock();
      lock.value() << log_line << std::endl;
    }
  }

  const auto status = service_->Search(context, request, response);

  if (!status.ok() || response->has_error()) {
    std::string log_line;
    auto writer = log_boilerplate(util::JsonWriter::create(log_line), req_id,
                                  user, *request);

    if (!status.ok()) {
      writer = log_error_status(std::move(writer), status);
    }
    if (response->has_error()) {
      writer = writer.prop("error_message").protobuf_message(*response);
    }

    writer.endObject().done();

    {
      auto lock = f_search_error.lock();
      lock.value() << log_line << std::endl;
    }
  }

  return status;
}

grpc::Status RequestLogger::GetCorpora(grpc::ServerContext* context,
                                       const CorporaRequest* request,
                                       CorporaResponse* response) {
  const auto user = get_tracking_id(*context);
  const auto req_id = req_counter_++;

  {
    std::string log_line;
    log_boilerplate(util::JsonWriter::create(log_line), req_id, user, *request)
        .endObject()
        .done();

    {
      auto lock = f_get_corpora_req_.lock();
      lock.value() << log_line << std::endl;
    }
  }

  const auto status = service_->GetCorpora(context, request, response);

  if (!status.ok()) {
    std::string log_line;
    log_error_status(log_boilerplate(util::JsonWriter::create(log_line), req_id,
                                     user, *request),
                     status)
        .endObject()
        .done();

    {
      auto lock = f_get_corpora_error.lock();
      lock.value() << log_line << std::endl;
    }
  }

  return status;
}


} // namespace service
} // namespace netspeak
