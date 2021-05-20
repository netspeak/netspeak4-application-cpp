#include "netspeak/error.hpp"

#include <sstream>
#include <string>

namespace netspeak {

const std::string query_error_message::too_many_words(
    "There are too many words in this section");
const std::string query_error_message::too_many_regex_in_orderset(
    "There are too many regexwords in this orderset");
const std::string query_error_message::invalid_regexword("invalid regexword");
const std::string query_error_message::too_deeply_nested(
    "The query is too deeply nested");
const std::string query_error_message::too_complex(
    uint32_t complexity, uint32_t max_worst_case_complexity) {
  std::stringstream error;
  error << "This query is too complex. Your complexity is ";
  error << complexity;
  error << " but the maximum complexity is ";
  error << max_worst_case_complexity;
  error << ".";

  return error.str();
}
const std::string query_error_message::too_long_query(int queryLength,
                                                      int queryMaxLength) {
  std::stringstream error;
  error << "This query is too long. query length : ";
  error << queryLength;
  error << " max query length : ";
  error << queryMaxLength;
  return error.str();
}

const std::string error_message::no_error("no error");

const std::string error_message::invalid_query("invalid query");

const std::string error_message::server_error("server error");

const std::string error_message::unknown_error("unknown error");

const std::string error_message::cannot_create("cannot create");

const std::string error_message::cannot_open("cannot open");

const std::string error_message::does_not_exist("does not exist");

const std::string error_message::is_not_empty("is not empty");


} // namespace netspeak
