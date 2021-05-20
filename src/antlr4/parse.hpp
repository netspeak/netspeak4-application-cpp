#ifndef ANTLR4_PARSE_HPP
#define ANTLR4_PARSE_HPP

#include <memory>

#include "netspeak/model/Query.hpp"


namespace antlr4 {

std::shared_ptr<netspeak::model::Query> parse_query(const std::string& query);

} // namespace antlr4

#endif
