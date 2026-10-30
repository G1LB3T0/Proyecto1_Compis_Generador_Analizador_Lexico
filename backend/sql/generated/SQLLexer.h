
// Generated from SQL.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  SQLLexer : public antlr4::Lexer {
public:
  enum {
    K_CREATE = 1, K_TABLE = 2, K_DROP = 3, K_INDEX = 4, K_ON = 5, K_IF = 6, 
    K_EXISTS = 7, K_INSERT = 8, K_INTO = 9, K_VALUES = 10, K_UPDATE = 11, 
    K_SET = 12, K_DELETE = 13, K_SELECT = 14, K_DISTINCT = 15, K_FROM = 16, 
    K_WHERE = 17, K_AND = 18, K_OR = 19, K_NOT = 20, K_IN = 21, K_IS = 22, 
    K_NULL = 23, K_INNER = 24, K_JOIN = 25, K_AS = 26, K_ORDER = 27, K_GROUP = 28, 
    K_BY = 29, K_ASC = 30, K_DESC = 31, K_HAVING = 32, K_LIMIT = 33, K_COUNT = 34, 
    K_SUM = 35, K_AVG = 36, K_MIN = 37, K_MAX = 38, K_PRIMARY = 39, K_KEY = 40, 
    K_FOREIGN = 41, K_REFERENCES = 42, K_UNIQUE = 43, K_INT = 44, K_FLOAT = 45, 
    K_VARCHAR = 46, K_BOOLEAN = 47, K_DATE = 48, K_TRUE = 49, K_FALSE = 50, 
    NEQ = 51, LTE = 52, GTE = 53, EQ = 54, LT = 55, GT = 56, PLUS = 57, 
    MINUS = 58, STAR = 59, SLASH = 60, PERCENT = 61, COMMA = 62, SEMI = 63, 
    DOT = 64, LPAREN = 65, RPAREN = 66, FLOAT_LITERAL = 67, INT_LITERAL = 68, 
    STRING_LITERAL = 69, IDENTIFIER = 70, LINE_COMMENT = 71, BLOCK_COMMENT = 72, 
    WS = 73
  };

  explicit SQLLexer(antlr4::CharStream *input);

  ~SQLLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

