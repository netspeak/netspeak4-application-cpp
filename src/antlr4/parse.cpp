#include "antlr4/parse.hpp"

#include <sstream>
#include <vector>

#include "antlr4-runtime.h"
#include "antlr4/QueryErrorHandler.hpp"
#include "antlr4/generated/QueryBaseListener.h"
#include "antlr4/generated/QueryLexer.h"
#include "antlr4/generated/QueryParser.h"

#include "netspeak/error.hpp"

namespace antlr4 {
using namespace netspeak::model;

typedef Query::Unit Unit;
typedef Query::Unit::Tag Tag;

/**
 * @brief Unescapes backslashes in the given word.
 */
void unescape_word(std::string& word) {
  size_t write_i = 0;

  for (size_t i = 0; i < word.size(); i++) {
    char c = word[i];
    if (c == '\\') {
      size_t next = i + 1;
      if (next < word.size()) {
        c = word[next];
        i = next;
      }
    }
    word[write_i] = c;
    write_i++;
  }

  word.erase(write_i);
}

/**
 * @brief A query listener that will construct a \c Query while visiting all
 * ANTLR4 grammar rules.
 *
 * The understand the implementation of this class, it's helpful to have the
 * grammar (not necessarily the lexer) open on the side.
 */
class QueryConstructor : public QueryBaseListener {
private:
  std::vector<std::shared_ptr<Unit>> stack_;
  std::shared_ptr<Query> query_;
  bool in_dictset_ = false;

  static const size_t MAX_STACK_SIZE = 30;

  void push_stack(std::shared_ptr<Unit> unit) {
    auto& last = stack_[stack_.size() - 1];
    last->add_child(unit);
    stack_.push_back(unit);

    // Right now, the maximum expected stack size is 6 or 7, but was the
    // grammar changed to be recursive, the stack size has to be limited. The
    // reason for the limit is not that this stack might grow too much. Almost
    // all query opertions (e.g. min/max length, conversion to norm queries) are
    // implemented using recursion, so the program stack might overflow for too
    // deeply nested query ASTs. So to be on the safe side, the size of this
    // stack is limited.
    if (stack_.size() > MAX_STACK_SIZE) {
      throw netspeak::invalid_query_error("The query is too deeply nested");
    }
  }

  void pop_stack() {
    if (stack_.size() <= 1) {
      throw netspeak::tracable_logic_error(
          "Some operation either pushed too little or popped too much.");
    }
    stack_.pop_back();
  }

public:
  QueryConstructor() {}
  QueryConstructor(const QueryConstructor&) = delete;

  std::shared_ptr<Query> query() {
    return query_;
  }

public:
  virtual void enterQuery(QueryParser::QueryContext*) override {
    query_ = std::make_shared<Query>();
    stack_.clear();
    stack_.push_back(query_->alternatives());
  }
  virtual void exitQuery(QueryParser::QueryContext*) override {
    if (stack_.size() != 1) {
      throw netspeak::tracable_logic_error(
          "Some operation either pushed too much or popped too little.");
    }
  }

  virtual void enterUnits(QueryParser::UnitsContext*) override {
    push_stack(Unit::non_terminal(Tag::CONCAT));
  }
  virtual void exitUnits(QueryParser::UnitsContext*) override {
    pop_stack();
  }

  // A unit is just a wrapper around all of the below units (see grammar).
  // Since, it's not a "physical" unit, we can ignore it.
  virtual void enterUnit(QueryParser::UnitContext*) override {}
  virtual void exitUnit(QueryParser::UnitContext*) override {}

  virtual void enterOptionset(QueryParser::OptionsetContext*) override {
    push_stack(Unit::non_terminal(Tag::OPTIONSET));
  }
  virtual void exitOptionset(QueryParser::OptionsetContext*) override {
    pop_stack();
  }

  virtual void enterOrderset(QueryParser::OrdersetContext*) override {
    push_stack(Unit::non_terminal(Tag::ORDERSET));
  }
  virtual void exitOrderset(QueryParser::OrdersetContext*) override {
    pop_stack();
  }

  virtual void enterDictset(QueryParser::DictsetContext*) override {
    in_dictset_ = true;
  }
  virtual void exitDictset(QueryParser::DictsetContext*) override {
    in_dictset_ = false;
  }

  virtual void enterPhrase(QueryParser::PhraseContext*) override {
    push_stack(Unit::non_terminal(Tag::CONCAT));
  }
  virtual void exitPhrase(QueryParser::PhraseContext*) override {
    pop_stack();
  }

  virtual void enterAsterisk(QueryParser::AsteriskContext* context) override {
    push_stack(Unit::terminal(Tag::STAR, context->getText()));
  }
  virtual void exitAsterisk(QueryParser::AsteriskContext*) override {
    pop_stack();
  }

  virtual void enterRegexword(QueryParser::RegexwordContext* context) override {
    push_stack(Unit::terminal(Tag::REGEX, context->getText()));
  }
  virtual void exitRegexword(QueryParser::RegexwordContext*) override {
    pop_stack();
  }

  virtual void enterWord(QueryParser::WordContext* context) override {
    std::string text = context->getText();
    unescape_word(text);

    if (in_dictset_) {
      push_stack(Unit::terminal(Tag::DICTSET, text));
    } else {
      push_stack(Unit::terminal(Tag::WORD, text));
    }
  }
  virtual void exitWord(QueryParser::WordContext*) override {
    pop_stack();
  }

  virtual void enterQmark(QueryParser::QmarkContext* context) override {
    push_stack(Unit::terminal(Tag::QMARK, context->getText()));
  }
  virtual void exitQmark(QueryParser::QmarkContext*) override {
    pop_stack();
  }

  virtual void enterPlus(QueryParser::PlusContext* context) override {
    push_stack(Unit::terminal(Tag::PLUS, context->getText()));
  }
  virtual void exitPlus(QueryParser::PlusContext*) override {
    pop_stack();
  }

  // Association tokens are just the '|' character for some reason
  // TODO: Make a the '|' a token and not a rule in the grammar
  virtual void enterAssociation(QueryParser::AssociationContext*) override {}
  virtual void exitAssociation(QueryParser::AssociationContext*) override {}
};


std::shared_ptr<Query> parse_query(const std::string& query) {
  if (query.size() > 2000) {
    throw netspeak::invalid_query_error("Query too long");
  }

  ANTLRInputStream input(query);
  QueryLexer lexer(&input);
  CommonTokenStream tokens(&lexer);
  QueryParser parser(&tokens);

  QueryErrorHandler error_listener;
  parser.removeErrorListeners();
  parser.addErrorListener(&error_listener);

  QueryConstructor construct;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&construct, parser.query());
  return construct.query();
}


} // namespace antlr4
