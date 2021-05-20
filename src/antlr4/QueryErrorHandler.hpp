#ifndef ANTLR4_QUERY_ERROR_HANDLER_HPP
#define ANTLR4_QUERY_ERROR_HANDLER_HPP


#include <sstream>
#include <string>

#include "antlr4-runtime.h"


namespace antlr4 {

/**
 * @brief The QueryErrorHandler class
 *
 * Is called whenever a error in the parser occurs.
 */
class QueryErrorHandler : public antlr4::BaseErrorListener {
public:
  QueryErrorHandler() = default;

  /**
   * @brief Is called when a syntax error occurs e.g. an unexpected token is
   * read.
   */
  void syntaxError(Recognizer*, Token* offendingSymbol, size_t line,
                   size_t charPositionInLine, const std::string& msg,
                   std::exception_ptr) override;
};

} // namespace antlr4


#endif
