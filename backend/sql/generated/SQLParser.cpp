
// Generated from SQL.g4 by ANTLR 4.13.2


#include "SQLVisitor.h"

#include "SQLParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SQLParserStaticData final {
  SQLParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SQLParserStaticData(const SQLParserStaticData&) = delete;
  SQLParserStaticData(SQLParserStaticData&&) = delete;
  SQLParserStaticData& operator=(const SQLParserStaticData&) = delete;
  SQLParserStaticData& operator=(SQLParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sqlParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<SQLParserStaticData> sqlParserStaticData = nullptr;

void sqlParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (sqlParserStaticData != nullptr) {
    return;
  }
#else
  assert(sqlParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SQLParserStaticData>(
    std::vector<std::string>{
      "script", "statement", "createTableStatement", "tableElement", "columnDefinition", 
      "dataType", "columnConstraint", "tableConstraint", "dropTableStatement", 
      "createIndexStatement", "dropIndexStatement", "insertStatement", "valuesRow", 
      "updateStatement", "assignment", "deleteStatement", "selectStatement", 
      "selectItem", "fromClause", "tableReference", "joinClause", "whereClause", 
      "groupByClause", "havingClause", "orderByClause", "orderItem", "limitClause", 
      "expression", "primary", "aggregateFunction", "columnReference", "literal", 
      "identifier"
    },
    std::vector<std::string>{
      "", "'CREATE'", "'TABLE'", "'DROP'", "'INDEX'", "'ON'", "'IF'", "'EXISTS'", 
      "'INSERT'", "'INTO'", "'VALUES'", "'UPDATE'", "'SET'", "'DELETE'", 
      "'SELECT'", "'DISTINCT'", "'FROM'", "'WHERE'", "'AND'", "'OR'", "'NOT'", 
      "'IN'", "'IS'", "'NULL'", "'INNER'", "'JOIN'", "'AS'", "'ORDER'", 
      "'GROUP'", "'BY'", "'ASC'", "'DESC'", "'HAVING'", "'LIMIT'", "'COUNT'", 
      "'SUM'", "'AVG'", "'MIN'", "'MAX'", "'PRIMARY'", "'KEY'", "'FOREIGN'", 
      "'REFERENCES'", "'UNIQUE'", "", "'FLOAT'", "'VARCHAR'", "'BOOLEAN'", 
      "'DATE'", "'TRUE'", "'FALSE'", "", "'<='", "'>='", "'='", "'<'", "'>'", 
      "'+'", "'-'", "'*'", "'/'", "'%'", "','", "';'", "'.'", "'('", "')'"
    },
    std::vector<std::string>{
      "", "K_CREATE", "K_TABLE", "K_DROP", "K_INDEX", "K_ON", "K_IF", "K_EXISTS", 
      "K_INSERT", "K_INTO", "K_VALUES", "K_UPDATE", "K_SET", "K_DELETE", 
      "K_SELECT", "K_DISTINCT", "K_FROM", "K_WHERE", "K_AND", "K_OR", "K_NOT", 
      "K_IN", "K_IS", "K_NULL", "K_INNER", "K_JOIN", "K_AS", "K_ORDER", 
      "K_GROUP", "K_BY", "K_ASC", "K_DESC", "K_HAVING", "K_LIMIT", "K_COUNT", 
      "K_SUM", "K_AVG", "K_MIN", "K_MAX", "K_PRIMARY", "K_KEY", "K_FOREIGN", 
      "K_REFERENCES", "K_UNIQUE", "K_INT", "K_FLOAT", "K_VARCHAR", "K_BOOLEAN", 
      "K_DATE", "K_TRUE", "K_FALSE", "NEQ", "LTE", "GTE", "EQ", "LT", "GT", 
      "PLUS", "MINUS", "STAR", "SLASH", "PERCENT", "COMMA", "SEMI", "DOT", 
      "LPAREN", "RPAREN", "FLOAT_LITERAL", "INT_LITERAL", "STRING_LITERAL", 
      "IDENTIFIER", "LINE_COMMENT", "BLOCK_COMMENT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,73,480,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,1,0,5,0,68,8,0,10,0,12,0,71,
  	9,0,1,0,1,0,4,0,75,8,0,11,0,12,0,76,1,0,5,0,80,8,0,10,0,12,0,83,9,0,1,
  	0,5,0,86,8,0,10,0,12,0,89,9,0,3,0,91,8,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,3,1,103,8,1,1,2,1,2,1,2,1,2,1,2,3,2,110,8,2,1,2,1,2,1,2,1,
  	2,1,2,5,2,117,8,2,10,2,12,2,120,9,2,1,2,1,2,1,3,1,3,3,3,126,8,3,1,4,1,
  	4,1,4,5,4,131,8,4,10,4,12,4,134,9,4,1,5,1,5,1,5,1,5,1,5,1,5,1,5,1,5,3,
  	5,144,8,5,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,1,6,3,6,158,8,6,
  	1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,7,1,
  	7,1,7,1,7,1,7,1,7,3,7,182,8,7,1,8,1,8,1,8,1,8,3,8,188,8,8,1,8,1,8,1,9,
  	1,9,3,9,194,8,9,1,9,1,9,1,9,1,9,1,9,1,9,1,9,1,9,1,10,1,10,1,10,1,10,3,
  	10,208,8,10,1,10,1,10,1,11,1,11,1,11,1,11,1,11,1,11,1,11,5,11,219,8,11,
  	10,11,12,11,222,9,11,1,11,1,11,3,11,226,8,11,1,11,1,11,1,11,1,11,5,11,
  	232,8,11,10,11,12,11,235,9,11,1,12,1,12,1,12,1,12,5,12,241,8,12,10,12,
  	12,12,244,9,12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,5,13,254,8,13,
  	10,13,12,13,257,9,13,1,13,3,13,260,8,13,1,14,1,14,1,14,1,14,1,15,1,15,
  	1,15,1,15,3,15,270,8,15,1,16,1,16,3,16,274,8,16,1,16,1,16,1,16,5,16,279,
  	8,16,10,16,12,16,282,9,16,1,16,3,16,285,8,16,1,16,3,16,288,8,16,1,16,
  	3,16,291,8,16,1,16,3,16,294,8,16,1,16,3,16,297,8,16,1,16,3,16,300,8,16,
  	1,17,1,17,1,17,1,17,1,17,1,17,1,17,3,17,309,8,17,1,17,3,17,312,8,17,3,
  	17,314,8,17,1,18,1,18,1,18,5,18,319,8,18,10,18,12,18,322,9,18,1,19,1,
  	19,3,19,326,8,19,1,19,3,19,329,8,19,1,20,3,20,332,8,20,1,20,1,20,1,20,
  	1,20,1,20,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,5,22,347,8,22,10,22,
  	12,22,350,9,22,1,23,1,23,1,23,1,24,1,24,1,24,1,24,1,24,5,24,360,8,24,
  	10,24,12,24,363,9,24,1,25,1,25,3,25,367,8,25,1,26,1,26,1,26,1,27,1,27,
  	1,27,1,27,1,27,1,27,3,27,378,8,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,
  	1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,3,27,398,8,27,
  	1,27,1,27,1,27,3,27,403,8,27,1,27,1,27,1,27,1,27,1,27,1,27,1,27,3,27,
  	412,8,27,1,27,1,27,1,27,1,27,1,27,5,27,419,8,27,10,27,12,27,422,9,27,
  	1,27,1,27,5,27,426,8,27,10,27,12,27,429,9,27,1,28,1,28,1,28,1,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,3,28,447,8,28,
  	1,29,1,29,1,29,1,29,1,29,1,29,1,29,3,29,456,8,29,1,29,1,29,1,29,3,29,
  	461,8,29,1,30,1,30,1,30,3,30,466,8,30,1,31,1,31,1,31,1,31,1,31,1,31,1,
  	31,1,31,3,31,476,8,31,1,32,1,32,1,32,0,1,54,33,0,2,4,6,8,10,12,14,16,
  	18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,
  	64,0,5,1,0,30,31,1,0,57,58,1,0,59,61,1,0,51,56,1,0,34,38,529,0,69,1,0,
  	0,0,2,102,1,0,0,0,4,104,1,0,0,0,6,125,1,0,0,0,8,127,1,0,0,0,10,143,1,
  	0,0,0,12,157,1,0,0,0,14,181,1,0,0,0,16,183,1,0,0,0,18,191,1,0,0,0,20,
  	203,1,0,0,0,22,211,1,0,0,0,24,236,1,0,0,0,26,247,1,0,0,0,28,261,1,0,0,
  	0,30,265,1,0,0,0,32,271,1,0,0,0,34,313,1,0,0,0,36,315,1,0,0,0,38,323,
  	1,0,0,0,40,331,1,0,0,0,42,338,1,0,0,0,44,341,1,0,0,0,46,351,1,0,0,0,48,
  	354,1,0,0,0,50,364,1,0,0,0,52,368,1,0,0,0,54,377,1,0,0,0,56,446,1,0,0,
  	0,58,460,1,0,0,0,60,462,1,0,0,0,62,475,1,0,0,0,64,477,1,0,0,0,66,68,5,
  	63,0,0,67,66,1,0,0,0,68,71,1,0,0,0,69,67,1,0,0,0,69,70,1,0,0,0,70,90,
  	1,0,0,0,71,69,1,0,0,0,72,81,3,2,1,0,73,75,5,63,0,0,74,73,1,0,0,0,75,76,
  	1,0,0,0,76,74,1,0,0,0,76,77,1,0,0,0,77,78,1,0,0,0,78,80,3,2,1,0,79,74,
  	1,0,0,0,80,83,1,0,0,0,81,79,1,0,0,0,81,82,1,0,0,0,82,87,1,0,0,0,83,81,
  	1,0,0,0,84,86,5,63,0,0,85,84,1,0,0,0,86,89,1,0,0,0,87,85,1,0,0,0,87,88,
  	1,0,0,0,88,91,1,0,0,0,89,87,1,0,0,0,90,72,1,0,0,0,90,91,1,0,0,0,91,92,
  	1,0,0,0,92,93,5,0,0,1,93,1,1,0,0,0,94,103,3,4,2,0,95,103,3,16,8,0,96,
  	103,3,18,9,0,97,103,3,20,10,0,98,103,3,22,11,0,99,103,3,26,13,0,100,103,
  	3,30,15,0,101,103,3,32,16,0,102,94,1,0,0,0,102,95,1,0,0,0,102,96,1,0,
  	0,0,102,97,1,0,0,0,102,98,1,0,0,0,102,99,1,0,0,0,102,100,1,0,0,0,102,
  	101,1,0,0,0,103,3,1,0,0,0,104,105,5,1,0,0,105,109,5,2,0,0,106,107,5,6,
  	0,0,107,108,5,20,0,0,108,110,5,7,0,0,109,106,1,0,0,0,109,110,1,0,0,0,
  	110,111,1,0,0,0,111,112,3,64,32,0,112,113,5,65,0,0,113,118,3,6,3,0,114,
  	115,5,62,0,0,115,117,3,6,3,0,116,114,1,0,0,0,117,120,1,0,0,0,118,116,
  	1,0,0,0,118,119,1,0,0,0,119,121,1,0,0,0,120,118,1,0,0,0,121,122,5,66,
  	0,0,122,5,1,0,0,0,123,126,3,8,4,0,124,126,3,14,7,0,125,123,1,0,0,0,125,
  	124,1,0,0,0,126,7,1,0,0,0,127,128,3,64,32,0,128,132,3,10,5,0,129,131,
  	3,12,6,0,130,129,1,0,0,0,131,134,1,0,0,0,132,130,1,0,0,0,132,133,1,0,
  	0,0,133,9,1,0,0,0,134,132,1,0,0,0,135,144,5,44,0,0,136,144,5,45,0,0,137,
  	138,5,46,0,0,138,139,5,65,0,0,139,140,5,68,0,0,140,144,5,66,0,0,141,144,
  	5,47,0,0,142,144,5,48,0,0,143,135,1,0,0,0,143,136,1,0,0,0,143,137,1,0,
  	0,0,143,141,1,0,0,0,143,142,1,0,0,0,144,11,1,0,0,0,145,146,5,39,0,0,146,
  	158,5,40,0,0,147,148,5,20,0,0,148,158,5,23,0,0,149,158,5,23,0,0,150,158,
  	5,43,0,0,151,152,5,42,0,0,152,153,3,64,32,0,153,154,5,65,0,0,154,155,
  	3,64,32,0,155,156,5,66,0,0,156,158,1,0,0,0,157,145,1,0,0,0,157,147,1,
  	0,0,0,157,149,1,0,0,0,157,150,1,0,0,0,157,151,1,0,0,0,158,13,1,0,0,0,
  	159,160,5,39,0,0,160,161,5,40,0,0,161,162,5,65,0,0,162,163,3,64,32,0,
  	163,164,5,66,0,0,164,182,1,0,0,0,165,166,5,43,0,0,166,167,5,65,0,0,167,
  	168,3,64,32,0,168,169,5,66,0,0,169,182,1,0,0,0,170,171,5,41,0,0,171,172,
  	5,40,0,0,172,173,5,65,0,0,173,174,3,64,32,0,174,175,5,66,0,0,175,176,
  	5,42,0,0,176,177,3,64,32,0,177,178,5,65,0,0,178,179,3,64,32,0,179,180,
  	5,66,0,0,180,182,1,0,0,0,181,159,1,0,0,0,181,165,1,0,0,0,181,170,1,0,
  	0,0,182,15,1,0,0,0,183,184,5,3,0,0,184,187,5,2,0,0,185,186,5,6,0,0,186,
  	188,5,7,0,0,187,185,1,0,0,0,187,188,1,0,0,0,188,189,1,0,0,0,189,190,3,
  	64,32,0,190,17,1,0,0,0,191,193,5,1,0,0,192,194,5,43,0,0,193,192,1,0,0,
  	0,193,194,1,0,0,0,194,195,1,0,0,0,195,196,5,4,0,0,196,197,3,64,32,0,197,
  	198,5,5,0,0,198,199,3,64,32,0,199,200,5,65,0,0,200,201,3,64,32,0,201,
  	202,5,66,0,0,202,19,1,0,0,0,203,204,5,3,0,0,204,207,5,4,0,0,205,206,5,
  	6,0,0,206,208,5,7,0,0,207,205,1,0,0,0,207,208,1,0,0,0,208,209,1,0,0,0,
  	209,210,3,64,32,0,210,21,1,0,0,0,211,212,5,8,0,0,212,213,5,9,0,0,213,
  	225,3,64,32,0,214,215,5,65,0,0,215,220,3,64,32,0,216,217,5,62,0,0,217,
  	219,3,64,32,0,218,216,1,0,0,0,219,222,1,0,0,0,220,218,1,0,0,0,220,221,
  	1,0,0,0,221,223,1,0,0,0,222,220,1,0,0,0,223,224,5,66,0,0,224,226,1,0,
  	0,0,225,214,1,0,0,0,225,226,1,0,0,0,226,227,1,0,0,0,227,228,5,10,0,0,
  	228,233,3,24,12,0,229,230,5,62,0,0,230,232,3,24,12,0,231,229,1,0,0,0,
  	232,235,1,0,0,0,233,231,1,0,0,0,233,234,1,0,0,0,234,23,1,0,0,0,235,233,
  	1,0,0,0,236,237,5,65,0,0,237,242,3,54,27,0,238,239,5,62,0,0,239,241,3,
  	54,27,0,240,238,1,0,0,0,241,244,1,0,0,0,242,240,1,0,0,0,242,243,1,0,0,
  	0,243,245,1,0,0,0,244,242,1,0,0,0,245,246,5,66,0,0,246,25,1,0,0,0,247,
  	248,5,11,0,0,248,249,3,64,32,0,249,250,5,12,0,0,250,255,3,28,14,0,251,
  	252,5,62,0,0,252,254,3,28,14,0,253,251,1,0,0,0,254,257,1,0,0,0,255,253,
  	1,0,0,0,255,256,1,0,0,0,256,259,1,0,0,0,257,255,1,0,0,0,258,260,3,42,
  	21,0,259,258,1,0,0,0,259,260,1,0,0,0,260,27,1,0,0,0,261,262,3,64,32,0,
  	262,263,5,54,0,0,263,264,3,54,27,0,264,29,1,0,0,0,265,266,5,13,0,0,266,
  	267,5,16,0,0,267,269,3,64,32,0,268,270,3,42,21,0,269,268,1,0,0,0,269,
  	270,1,0,0,0,270,31,1,0,0,0,271,273,5,14,0,0,272,274,5,15,0,0,273,272,
  	1,0,0,0,273,274,1,0,0,0,274,275,1,0,0,0,275,280,3,34,17,0,276,277,5,62,
  	0,0,277,279,3,34,17,0,278,276,1,0,0,0,279,282,1,0,0,0,280,278,1,0,0,0,
  	280,281,1,0,0,0,281,284,1,0,0,0,282,280,1,0,0,0,283,285,3,36,18,0,284,
  	283,1,0,0,0,284,285,1,0,0,0,285,287,1,0,0,0,286,288,3,42,21,0,287,286,
  	1,0,0,0,287,288,1,0,0,0,288,290,1,0,0,0,289,291,3,44,22,0,290,289,1,0,
  	0,0,290,291,1,0,0,0,291,293,1,0,0,0,292,294,3,46,23,0,293,292,1,0,0,0,
  	293,294,1,0,0,0,294,296,1,0,0,0,295,297,3,48,24,0,296,295,1,0,0,0,296,
  	297,1,0,0,0,297,299,1,0,0,0,298,300,3,52,26,0,299,298,1,0,0,0,299,300,
  	1,0,0,0,300,33,1,0,0,0,301,314,5,59,0,0,302,303,3,64,32,0,303,304,5,64,
  	0,0,304,305,5,59,0,0,305,314,1,0,0,0,306,311,3,54,27,0,307,309,5,26,0,
  	0,308,307,1,0,0,0,308,309,1,0,0,0,309,310,1,0,0,0,310,312,3,64,32,0,311,
  	308,1,0,0,0,311,312,1,0,0,0,312,314,1,0,0,0,313,301,1,0,0,0,313,302,1,
  	0,0,0,313,306,1,0,0,0,314,35,1,0,0,0,315,316,5,16,0,0,316,320,3,38,19,
  	0,317,319,3,40,20,0,318,317,1,0,0,0,319,322,1,0,0,0,320,318,1,0,0,0,320,
  	321,1,0,0,0,321,37,1,0,0,0,322,320,1,0,0,0,323,328,3,64,32,0,324,326,
  	5,26,0,0,325,324,1,0,0,0,325,326,1,0,0,0,326,327,1,0,0,0,327,329,3,64,
  	32,0,328,325,1,0,0,0,328,329,1,0,0,0,329,39,1,0,0,0,330,332,5,24,0,0,
  	331,330,1,0,0,0,331,332,1,0,0,0,332,333,1,0,0,0,333,334,5,25,0,0,334,
  	335,3,38,19,0,335,336,5,5,0,0,336,337,3,54,27,0,337,41,1,0,0,0,338,339,
  	5,17,0,0,339,340,3,54,27,0,340,43,1,0,0,0,341,342,5,28,0,0,342,343,5,
  	29,0,0,343,348,3,54,27,0,344,345,5,62,0,0,345,347,3,54,27,0,346,344,1,
  	0,0,0,347,350,1,0,0,0,348,346,1,0,0,0,348,349,1,0,0,0,349,45,1,0,0,0,
  	350,348,1,0,0,0,351,352,5,32,0,0,352,353,3,54,27,0,353,47,1,0,0,0,354,
  	355,5,27,0,0,355,356,5,29,0,0,356,361,3,50,25,0,357,358,5,62,0,0,358,
  	360,3,50,25,0,359,357,1,0,0,0,360,363,1,0,0,0,361,359,1,0,0,0,361,362,
  	1,0,0,0,362,49,1,0,0,0,363,361,1,0,0,0,364,366,3,54,27,0,365,367,7,0,
  	0,0,366,365,1,0,0,0,366,367,1,0,0,0,367,51,1,0,0,0,368,369,5,33,0,0,369,
  	370,5,68,0,0,370,53,1,0,0,0,371,372,6,27,-1,0,372,378,3,56,28,0,373,374,
  	7,1,0,0,374,378,3,54,27,10,375,376,5,20,0,0,376,378,3,54,27,3,377,371,
  	1,0,0,0,377,373,1,0,0,0,377,375,1,0,0,0,378,427,1,0,0,0,379,380,10,9,
  	0,0,380,381,7,2,0,0,381,426,3,54,27,10,382,383,10,8,0,0,383,384,7,1,0,
  	0,384,426,3,54,27,9,385,386,10,7,0,0,386,387,7,3,0,0,387,426,3,54,27,
  	8,388,389,10,2,0,0,389,390,5,18,0,0,390,426,3,54,27,3,391,392,10,1,0,
  	0,392,393,5,19,0,0,393,426,3,54,27,2,394,395,10,6,0,0,395,397,5,22,0,
  	0,396,398,5,20,0,0,397,396,1,0,0,0,397,398,1,0,0,0,398,399,1,0,0,0,399,
  	426,5,23,0,0,400,402,10,5,0,0,401,403,5,20,0,0,402,401,1,0,0,0,402,403,
  	1,0,0,0,403,404,1,0,0,0,404,405,5,21,0,0,405,406,5,65,0,0,406,407,3,32,
  	16,0,407,408,5,66,0,0,408,426,1,0,0,0,409,411,10,4,0,0,410,412,5,20,0,
  	0,411,410,1,0,0,0,411,412,1,0,0,0,412,413,1,0,0,0,413,414,5,21,0,0,414,
  	415,5,65,0,0,415,420,3,54,27,0,416,417,5,62,0,0,417,419,3,54,27,0,418,
  	416,1,0,0,0,419,422,1,0,0,0,420,418,1,0,0,0,420,421,1,0,0,0,421,423,1,
  	0,0,0,422,420,1,0,0,0,423,424,5,66,0,0,424,426,1,0,0,0,425,379,1,0,0,
  	0,425,382,1,0,0,0,425,385,1,0,0,0,425,388,1,0,0,0,425,391,1,0,0,0,425,
  	394,1,0,0,0,425,400,1,0,0,0,425,409,1,0,0,0,426,429,1,0,0,0,427,425,1,
  	0,0,0,427,428,1,0,0,0,428,55,1,0,0,0,429,427,1,0,0,0,430,447,3,62,31,
  	0,431,447,3,58,29,0,432,447,3,60,30,0,433,434,5,7,0,0,434,435,5,65,0,
  	0,435,436,3,32,16,0,436,437,5,66,0,0,437,447,1,0,0,0,438,439,5,65,0,0,
  	439,440,3,32,16,0,440,441,5,66,0,0,441,447,1,0,0,0,442,443,5,65,0,0,443,
  	444,3,54,27,0,444,445,5,66,0,0,445,447,1,0,0,0,446,430,1,0,0,0,446,431,
  	1,0,0,0,446,432,1,0,0,0,446,433,1,0,0,0,446,438,1,0,0,0,446,442,1,0,0,
  	0,447,57,1,0,0,0,448,449,5,34,0,0,449,450,5,65,0,0,450,451,5,59,0,0,451,
  	461,5,66,0,0,452,453,7,4,0,0,453,455,5,65,0,0,454,456,5,15,0,0,455,454,
  	1,0,0,0,455,456,1,0,0,0,456,457,1,0,0,0,457,458,3,54,27,0,458,459,5,66,
  	0,0,459,461,1,0,0,0,460,448,1,0,0,0,460,452,1,0,0,0,461,59,1,0,0,0,462,
  	465,3,64,32,0,463,464,5,64,0,0,464,466,3,64,32,0,465,463,1,0,0,0,465,
  	466,1,0,0,0,466,61,1,0,0,0,467,476,5,68,0,0,468,476,5,67,0,0,469,476,
  	5,69,0,0,470,476,5,49,0,0,471,476,5,50,0,0,472,476,5,23,0,0,473,474,5,
  	48,0,0,474,476,5,69,0,0,475,467,1,0,0,0,475,468,1,0,0,0,475,469,1,0,0,
  	0,475,470,1,0,0,0,475,471,1,0,0,0,475,472,1,0,0,0,475,473,1,0,0,0,476,
  	63,1,0,0,0,477,478,5,70,0,0,478,65,1,0,0,0,53,69,76,81,87,90,102,109,
  	118,125,132,143,157,181,187,193,207,220,225,233,242,255,259,269,273,280,
  	284,287,290,293,296,299,308,311,313,320,325,328,331,348,361,366,377,397,
  	402,411,420,425,427,446,455,460,465,475
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sqlParserStaticData = std::move(staticData);
}

}

SQLParser::SQLParser(TokenStream *input) : SQLParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SQLParser::SQLParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SQLParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sqlParserStaticData->atn, sqlParserStaticData->decisionToDFA, sqlParserStaticData->sharedContextCache, options);
}

SQLParser::~SQLParser() {
  delete _interpreter;
}

const atn::ATN& SQLParser::getATN() const {
  return *sqlParserStaticData->atn;
}

std::string SQLParser::getGrammarFileName() const {
  return "SQL.g4";
}

const std::vector<std::string>& SQLParser::getRuleNames() const {
  return sqlParserStaticData->ruleNames;
}

const dfa::Vocabulary& SQLParser::getVocabulary() const {
  return sqlParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SQLParser::getSerializedATN() const {
  return sqlParserStaticData->serializedATN;
}


//----------------- ScriptContext ------------------------------------------------------------------

SQLParser::ScriptContext::ScriptContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::ScriptContext::EOF() {
  return getToken(SQLParser::EOF, 0);
}

std::vector<tree::TerminalNode *> SQLParser::ScriptContext::SEMI() {
  return getTokens(SQLParser::SEMI);
}

tree::TerminalNode* SQLParser::ScriptContext::SEMI(size_t i) {
  return getToken(SQLParser::SEMI, i);
}

std::vector<SQLParser::StatementContext *> SQLParser::ScriptContext::statement() {
  return getRuleContexts<SQLParser::StatementContext>();
}

SQLParser::StatementContext* SQLParser::ScriptContext::statement(size_t i) {
  return getRuleContext<SQLParser::StatementContext>(i);
}


size_t SQLParser::ScriptContext::getRuleIndex() const {
  return SQLParser::RuleScript;
}


std::any SQLParser::ScriptContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitScript(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ScriptContext* SQLParser::script() {
  ScriptContext *_localctx = _tracker.createInstance<ScriptContext>(_ctx, getState());
  enterRule(_localctx, 0, SQLParser::RuleScript);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(69);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::SEMI) {
      setState(66);
      match(SQLParser::SEMI);
      setState(71);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(90);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 26890) != 0)) {
      setState(72);
      statement();
      setState(81);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(74); 
          _errHandler->sync(this);
          _la = _input->LA(1);
          do {
            setState(73);
            match(SQLParser::SEMI);
            setState(76); 
            _errHandler->sync(this);
            _la = _input->LA(1);
          } while (_la == SQLParser::SEMI);
          setState(78);
          statement(); 
        }
        setState(83);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 2, _ctx);
      }
      setState(87);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SQLParser::SEMI) {
        setState(84);
        match(SQLParser::SEMI);
        setState(89);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(92);
    match(SQLParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

SQLParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SQLParser::CreateTableStatementContext* SQLParser::StatementContext::createTableStatement() {
  return getRuleContext<SQLParser::CreateTableStatementContext>(0);
}

SQLParser::DropTableStatementContext* SQLParser::StatementContext::dropTableStatement() {
  return getRuleContext<SQLParser::DropTableStatementContext>(0);
}

SQLParser::CreateIndexStatementContext* SQLParser::StatementContext::createIndexStatement() {
  return getRuleContext<SQLParser::CreateIndexStatementContext>(0);
}

SQLParser::DropIndexStatementContext* SQLParser::StatementContext::dropIndexStatement() {
  return getRuleContext<SQLParser::DropIndexStatementContext>(0);
}

SQLParser::InsertStatementContext* SQLParser::StatementContext::insertStatement() {
  return getRuleContext<SQLParser::InsertStatementContext>(0);
}

SQLParser::UpdateStatementContext* SQLParser::StatementContext::updateStatement() {
  return getRuleContext<SQLParser::UpdateStatementContext>(0);
}

SQLParser::DeleteStatementContext* SQLParser::StatementContext::deleteStatement() {
  return getRuleContext<SQLParser::DeleteStatementContext>(0);
}

SQLParser::SelectStatementContext* SQLParser::StatementContext::selectStatement() {
  return getRuleContext<SQLParser::SelectStatementContext>(0);
}


size_t SQLParser::StatementContext::getRuleIndex() const {
  return SQLParser::RuleStatement;
}


std::any SQLParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::StatementContext* SQLParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 2, SQLParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(102);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(94);
      createTableStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(95);
      dropTableStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(96);
      createIndexStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(97);
      dropIndexStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(98);
      insertStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(99);
      updateStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(100);
      deleteStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(101);
      selectStatement();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CreateTableStatementContext ------------------------------------------------------------------

SQLParser::CreateTableStatementContext::CreateTableStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::K_CREATE() {
  return getToken(SQLParser::K_CREATE, 0);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::K_TABLE() {
  return getToken(SQLParser::K_TABLE, 0);
}

SQLParser::IdentifierContext* SQLParser::CreateTableStatementContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

std::vector<SQLParser::TableElementContext *> SQLParser::CreateTableStatementContext::tableElement() {
  return getRuleContexts<SQLParser::TableElementContext>();
}

SQLParser::TableElementContext* SQLParser::CreateTableStatementContext::tableElement(size_t i) {
  return getRuleContext<SQLParser::TableElementContext>(i);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::K_IF() {
  return getToken(SQLParser::K_IF, 0);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::K_EXISTS() {
  return getToken(SQLParser::K_EXISTS, 0);
}

std::vector<tree::TerminalNode *> SQLParser::CreateTableStatementContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::CreateTableStatementContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}


size_t SQLParser::CreateTableStatementContext::getRuleIndex() const {
  return SQLParser::RuleCreateTableStatement;
}


std::any SQLParser::CreateTableStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitCreateTableStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::CreateTableStatementContext* SQLParser::createTableStatement() {
  CreateTableStatementContext *_localctx = _tracker.createInstance<CreateTableStatementContext>(_ctx, getState());
  enterRule(_localctx, 4, SQLParser::RuleCreateTableStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(104);
    match(SQLParser::K_CREATE);
    setState(105);
    match(SQLParser::K_TABLE);
    setState(109);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_IF) {
      setState(106);
      match(SQLParser::K_IF);
      setState(107);
      match(SQLParser::K_NOT);
      setState(108);
      match(SQLParser::K_EXISTS);
    }
    setState(111);
    identifier();
    setState(112);
    match(SQLParser::LPAREN);
    setState(113);
    tableElement();
    setState(118);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(114);
      match(SQLParser::COMMA);
      setState(115);
      tableElement();
      setState(120);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(121);
    match(SQLParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TableElementContext ------------------------------------------------------------------

SQLParser::TableElementContext::TableElementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SQLParser::ColumnDefinitionContext* SQLParser::TableElementContext::columnDefinition() {
  return getRuleContext<SQLParser::ColumnDefinitionContext>(0);
}

SQLParser::TableConstraintContext* SQLParser::TableElementContext::tableConstraint() {
  return getRuleContext<SQLParser::TableConstraintContext>(0);
}


size_t SQLParser::TableElementContext::getRuleIndex() const {
  return SQLParser::RuleTableElement;
}


std::any SQLParser::TableElementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitTableElement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::TableElementContext* SQLParser::tableElement() {
  TableElementContext *_localctx = _tracker.createInstance<TableElementContext>(_ctx, getState());
  enterRule(_localctx, 6, SQLParser::RuleTableElement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(125);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(123);
        columnDefinition();
        break;
      }

      case SQLParser::K_PRIMARY:
      case SQLParser::K_FOREIGN:
      case SQLParser::K_UNIQUE: {
        enterOuterAlt(_localctx, 2);
        setState(124);
        tableConstraint();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ColumnDefinitionContext ------------------------------------------------------------------

SQLParser::ColumnDefinitionContext::ColumnDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SQLParser::IdentifierContext* SQLParser::ColumnDefinitionContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

SQLParser::DataTypeContext* SQLParser::ColumnDefinitionContext::dataType() {
  return getRuleContext<SQLParser::DataTypeContext>(0);
}

std::vector<SQLParser::ColumnConstraintContext *> SQLParser::ColumnDefinitionContext::columnConstraint() {
  return getRuleContexts<SQLParser::ColumnConstraintContext>();
}

SQLParser::ColumnConstraintContext* SQLParser::ColumnDefinitionContext::columnConstraint(size_t i) {
  return getRuleContext<SQLParser::ColumnConstraintContext>(i);
}


size_t SQLParser::ColumnDefinitionContext::getRuleIndex() const {
  return SQLParser::RuleColumnDefinition;
}


std::any SQLParser::ColumnDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitColumnDefinition(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ColumnDefinitionContext* SQLParser::columnDefinition() {
  ColumnDefinitionContext *_localctx = _tracker.createInstance<ColumnDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 8, SQLParser::RuleColumnDefinition);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(127);
    identifier();
    setState(128);
    dataType();
    setState(132);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 13743904784384) != 0)) {
      setState(129);
      columnConstraint();
      setState(134);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DataTypeContext ------------------------------------------------------------------

SQLParser::DataTypeContext::DataTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::DataTypeContext::K_INT() {
  return getToken(SQLParser::K_INT, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::K_FLOAT() {
  return getToken(SQLParser::K_FLOAT, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::K_VARCHAR() {
  return getToken(SQLParser::K_VARCHAR, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::INT_LITERAL() {
  return getToken(SQLParser::INT_LITERAL, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::K_BOOLEAN() {
  return getToken(SQLParser::K_BOOLEAN, 0);
}

tree::TerminalNode* SQLParser::DataTypeContext::K_DATE() {
  return getToken(SQLParser::K_DATE, 0);
}


size_t SQLParser::DataTypeContext::getRuleIndex() const {
  return SQLParser::RuleDataType;
}


std::any SQLParser::DataTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitDataType(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::DataTypeContext* SQLParser::dataType() {
  DataTypeContext *_localctx = _tracker.createInstance<DataTypeContext>(_ctx, getState());
  enterRule(_localctx, 10, SQLParser::RuleDataType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(143);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::K_INT: {
        enterOuterAlt(_localctx, 1);
        setState(135);
        match(SQLParser::K_INT);
        break;
      }

      case SQLParser::K_FLOAT: {
        enterOuterAlt(_localctx, 2);
        setState(136);
        match(SQLParser::K_FLOAT);
        break;
      }

      case SQLParser::K_VARCHAR: {
        enterOuterAlt(_localctx, 3);
        setState(137);
        match(SQLParser::K_VARCHAR);
        setState(138);
        match(SQLParser::LPAREN);
        setState(139);
        match(SQLParser::INT_LITERAL);
        setState(140);
        match(SQLParser::RPAREN);
        break;
      }

      case SQLParser::K_BOOLEAN: {
        enterOuterAlt(_localctx, 4);
        setState(141);
        match(SQLParser::K_BOOLEAN);
        break;
      }

      case SQLParser::K_DATE: {
        enterOuterAlt(_localctx, 5);
        setState(142);
        match(SQLParser::K_DATE);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ColumnConstraintContext ------------------------------------------------------------------

SQLParser::ColumnConstraintContext::ColumnConstraintContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_PRIMARY() {
  return getToken(SQLParser::K_PRIMARY, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_KEY() {
  return getToken(SQLParser::K_KEY, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_NULL() {
  return getToken(SQLParser::K_NULL, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_UNIQUE() {
  return getToken(SQLParser::K_UNIQUE, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::K_REFERENCES() {
  return getToken(SQLParser::K_REFERENCES, 0);
}

std::vector<SQLParser::IdentifierContext *> SQLParser::ColumnConstraintContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::ColumnConstraintContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::ColumnConstraintContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}


size_t SQLParser::ColumnConstraintContext::getRuleIndex() const {
  return SQLParser::RuleColumnConstraint;
}


std::any SQLParser::ColumnConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitColumnConstraint(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ColumnConstraintContext* SQLParser::columnConstraint() {
  ColumnConstraintContext *_localctx = _tracker.createInstance<ColumnConstraintContext>(_ctx, getState());
  enterRule(_localctx, 12, SQLParser::RuleColumnConstraint);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(157);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::K_PRIMARY: {
        enterOuterAlt(_localctx, 1);
        setState(145);
        match(SQLParser::K_PRIMARY);
        setState(146);
        match(SQLParser::K_KEY);
        break;
      }

      case SQLParser::K_NOT: {
        enterOuterAlt(_localctx, 2);
        setState(147);
        match(SQLParser::K_NOT);
        setState(148);
        match(SQLParser::K_NULL);
        break;
      }

      case SQLParser::K_NULL: {
        enterOuterAlt(_localctx, 3);
        setState(149);
        match(SQLParser::K_NULL);
        break;
      }

      case SQLParser::K_UNIQUE: {
        enterOuterAlt(_localctx, 4);
        setState(150);
        match(SQLParser::K_UNIQUE);
        break;
      }

      case SQLParser::K_REFERENCES: {
        enterOuterAlt(_localctx, 5);
        setState(151);
        match(SQLParser::K_REFERENCES);
        setState(152);
        identifier();
        setState(153);
        match(SQLParser::LPAREN);
        setState(154);
        identifier();
        setState(155);
        match(SQLParser::RPAREN);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TableConstraintContext ------------------------------------------------------------------

SQLParser::TableConstraintContext::TableConstraintContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::TableConstraintContext::K_PRIMARY() {
  return getToken(SQLParser::K_PRIMARY, 0);
}

tree::TerminalNode* SQLParser::TableConstraintContext::K_KEY() {
  return getToken(SQLParser::K_KEY, 0);
}

std::vector<tree::TerminalNode *> SQLParser::TableConstraintContext::LPAREN() {
  return getTokens(SQLParser::LPAREN);
}

tree::TerminalNode* SQLParser::TableConstraintContext::LPAREN(size_t i) {
  return getToken(SQLParser::LPAREN, i);
}

std::vector<SQLParser::IdentifierContext *> SQLParser::TableConstraintContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::TableConstraintContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

std::vector<tree::TerminalNode *> SQLParser::TableConstraintContext::RPAREN() {
  return getTokens(SQLParser::RPAREN);
}

tree::TerminalNode* SQLParser::TableConstraintContext::RPAREN(size_t i) {
  return getToken(SQLParser::RPAREN, i);
}

tree::TerminalNode* SQLParser::TableConstraintContext::K_UNIQUE() {
  return getToken(SQLParser::K_UNIQUE, 0);
}

tree::TerminalNode* SQLParser::TableConstraintContext::K_FOREIGN() {
  return getToken(SQLParser::K_FOREIGN, 0);
}

tree::TerminalNode* SQLParser::TableConstraintContext::K_REFERENCES() {
  return getToken(SQLParser::K_REFERENCES, 0);
}


size_t SQLParser::TableConstraintContext::getRuleIndex() const {
  return SQLParser::RuleTableConstraint;
}


std::any SQLParser::TableConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitTableConstraint(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::TableConstraintContext* SQLParser::tableConstraint() {
  TableConstraintContext *_localctx = _tracker.createInstance<TableConstraintContext>(_ctx, getState());
  enterRule(_localctx, 14, SQLParser::RuleTableConstraint);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(181);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::K_PRIMARY: {
        enterOuterAlt(_localctx, 1);
        setState(159);
        match(SQLParser::K_PRIMARY);
        setState(160);
        match(SQLParser::K_KEY);
        setState(161);
        match(SQLParser::LPAREN);
        setState(162);
        identifier();
        setState(163);
        match(SQLParser::RPAREN);
        break;
      }

      case SQLParser::K_UNIQUE: {
        enterOuterAlt(_localctx, 2);
        setState(165);
        match(SQLParser::K_UNIQUE);
        setState(166);
        match(SQLParser::LPAREN);
        setState(167);
        identifier();
        setState(168);
        match(SQLParser::RPAREN);
        break;
      }

      case SQLParser::K_FOREIGN: {
        enterOuterAlt(_localctx, 3);
        setState(170);
        match(SQLParser::K_FOREIGN);
        setState(171);
        match(SQLParser::K_KEY);
        setState(172);
        match(SQLParser::LPAREN);
        setState(173);
        identifier();
        setState(174);
        match(SQLParser::RPAREN);
        setState(175);
        match(SQLParser::K_REFERENCES);
        setState(176);
        identifier();
        setState(177);
        match(SQLParser::LPAREN);
        setState(178);
        identifier();
        setState(179);
        match(SQLParser::RPAREN);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DropTableStatementContext ------------------------------------------------------------------

SQLParser::DropTableStatementContext::DropTableStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::DropTableStatementContext::K_DROP() {
  return getToken(SQLParser::K_DROP, 0);
}

tree::TerminalNode* SQLParser::DropTableStatementContext::K_TABLE() {
  return getToken(SQLParser::K_TABLE, 0);
}

SQLParser::IdentifierContext* SQLParser::DropTableStatementContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::DropTableStatementContext::K_IF() {
  return getToken(SQLParser::K_IF, 0);
}

tree::TerminalNode* SQLParser::DropTableStatementContext::K_EXISTS() {
  return getToken(SQLParser::K_EXISTS, 0);
}


size_t SQLParser::DropTableStatementContext::getRuleIndex() const {
  return SQLParser::RuleDropTableStatement;
}


std::any SQLParser::DropTableStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitDropTableStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::DropTableStatementContext* SQLParser::dropTableStatement() {
  DropTableStatementContext *_localctx = _tracker.createInstance<DropTableStatementContext>(_ctx, getState());
  enterRule(_localctx, 16, SQLParser::RuleDropTableStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(183);
    match(SQLParser::K_DROP);
    setState(184);
    match(SQLParser::K_TABLE);
    setState(187);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_IF) {
      setState(185);
      match(SQLParser::K_IF);
      setState(186);
      match(SQLParser::K_EXISTS);
    }
    setState(189);
    identifier();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CreateIndexStatementContext ------------------------------------------------------------------

SQLParser::CreateIndexStatementContext::CreateIndexStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::K_CREATE() {
  return getToken(SQLParser::K_CREATE, 0);
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::K_INDEX() {
  return getToken(SQLParser::K_INDEX, 0);
}

std::vector<SQLParser::IdentifierContext *> SQLParser::CreateIndexStatementContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::CreateIndexStatementContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::K_ON() {
  return getToken(SQLParser::K_ON, 0);
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

tree::TerminalNode* SQLParser::CreateIndexStatementContext::K_UNIQUE() {
  return getToken(SQLParser::K_UNIQUE, 0);
}


size_t SQLParser::CreateIndexStatementContext::getRuleIndex() const {
  return SQLParser::RuleCreateIndexStatement;
}


std::any SQLParser::CreateIndexStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitCreateIndexStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::CreateIndexStatementContext* SQLParser::createIndexStatement() {
  CreateIndexStatementContext *_localctx = _tracker.createInstance<CreateIndexStatementContext>(_ctx, getState());
  enterRule(_localctx, 18, SQLParser::RuleCreateIndexStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(191);
    match(SQLParser::K_CREATE);
    setState(193);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_UNIQUE) {
      setState(192);
      match(SQLParser::K_UNIQUE);
    }
    setState(195);
    match(SQLParser::K_INDEX);
    setState(196);
    identifier();
    setState(197);
    match(SQLParser::K_ON);
    setState(198);
    identifier();
    setState(199);
    match(SQLParser::LPAREN);
    setState(200);
    identifier();
    setState(201);
    match(SQLParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DropIndexStatementContext ------------------------------------------------------------------

SQLParser::DropIndexStatementContext::DropIndexStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::DropIndexStatementContext::K_DROP() {
  return getToken(SQLParser::K_DROP, 0);
}

tree::TerminalNode* SQLParser::DropIndexStatementContext::K_INDEX() {
  return getToken(SQLParser::K_INDEX, 0);
}

SQLParser::IdentifierContext* SQLParser::DropIndexStatementContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::DropIndexStatementContext::K_IF() {
  return getToken(SQLParser::K_IF, 0);
}

tree::TerminalNode* SQLParser::DropIndexStatementContext::K_EXISTS() {
  return getToken(SQLParser::K_EXISTS, 0);
}


size_t SQLParser::DropIndexStatementContext::getRuleIndex() const {
  return SQLParser::RuleDropIndexStatement;
}


std::any SQLParser::DropIndexStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitDropIndexStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::DropIndexStatementContext* SQLParser::dropIndexStatement() {
  DropIndexStatementContext *_localctx = _tracker.createInstance<DropIndexStatementContext>(_ctx, getState());
  enterRule(_localctx, 20, SQLParser::RuleDropIndexStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(203);
    match(SQLParser::K_DROP);
    setState(204);
    match(SQLParser::K_INDEX);
    setState(207);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_IF) {
      setState(205);
      match(SQLParser::K_IF);
      setState(206);
      match(SQLParser::K_EXISTS);
    }
    setState(209);
    identifier();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InsertStatementContext ------------------------------------------------------------------

SQLParser::InsertStatementContext::InsertStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::InsertStatementContext::K_INSERT() {
  return getToken(SQLParser::K_INSERT, 0);
}

tree::TerminalNode* SQLParser::InsertStatementContext::K_INTO() {
  return getToken(SQLParser::K_INTO, 0);
}

std::vector<SQLParser::IdentifierContext *> SQLParser::InsertStatementContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::InsertStatementContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

tree::TerminalNode* SQLParser::InsertStatementContext::K_VALUES() {
  return getToken(SQLParser::K_VALUES, 0);
}

std::vector<SQLParser::ValuesRowContext *> SQLParser::InsertStatementContext::valuesRow() {
  return getRuleContexts<SQLParser::ValuesRowContext>();
}

SQLParser::ValuesRowContext* SQLParser::InsertStatementContext::valuesRow(size_t i) {
  return getRuleContext<SQLParser::ValuesRowContext>(i);
}

tree::TerminalNode* SQLParser::InsertStatementContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::InsertStatementContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

std::vector<tree::TerminalNode *> SQLParser::InsertStatementContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::InsertStatementContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}


size_t SQLParser::InsertStatementContext::getRuleIndex() const {
  return SQLParser::RuleInsertStatement;
}


std::any SQLParser::InsertStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitInsertStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::InsertStatementContext* SQLParser::insertStatement() {
  InsertStatementContext *_localctx = _tracker.createInstance<InsertStatementContext>(_ctx, getState());
  enterRule(_localctx, 22, SQLParser::RuleInsertStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(211);
    match(SQLParser::K_INSERT);
    setState(212);
    match(SQLParser::K_INTO);
    setState(213);
    identifier();
    setState(225);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::LPAREN) {
      setState(214);
      match(SQLParser::LPAREN);
      setState(215);
      identifier();
      setState(220);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SQLParser::COMMA) {
        setState(216);
        match(SQLParser::COMMA);
        setState(217);
        identifier();
        setState(222);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(223);
      match(SQLParser::RPAREN);
    }
    setState(227);
    match(SQLParser::K_VALUES);
    setState(228);
    valuesRow();
    setState(233);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(229);
      match(SQLParser::COMMA);
      setState(230);
      valuesRow();
      setState(235);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValuesRowContext ------------------------------------------------------------------

SQLParser::ValuesRowContext::ValuesRowContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::ValuesRowContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

std::vector<SQLParser::ExpressionContext *> SQLParser::ValuesRowContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::ValuesRowContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::ValuesRowContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

std::vector<tree::TerminalNode *> SQLParser::ValuesRowContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::ValuesRowContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}


size_t SQLParser::ValuesRowContext::getRuleIndex() const {
  return SQLParser::RuleValuesRow;
}


std::any SQLParser::ValuesRowContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitValuesRow(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ValuesRowContext* SQLParser::valuesRow() {
  ValuesRowContext *_localctx = _tracker.createInstance<ValuesRowContext>(_ctx, getState());
  enterRule(_localctx, 24, SQLParser::RuleValuesRow);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(236);
    match(SQLParser::LPAREN);
    setState(237);
    expression(0);
    setState(242);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(238);
      match(SQLParser::COMMA);
      setState(239);
      expression(0);
      setState(244);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(245);
    match(SQLParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UpdateStatementContext ------------------------------------------------------------------

SQLParser::UpdateStatementContext::UpdateStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::UpdateStatementContext::K_UPDATE() {
  return getToken(SQLParser::K_UPDATE, 0);
}

SQLParser::IdentifierContext* SQLParser::UpdateStatementContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::UpdateStatementContext::K_SET() {
  return getToken(SQLParser::K_SET, 0);
}

std::vector<SQLParser::AssignmentContext *> SQLParser::UpdateStatementContext::assignment() {
  return getRuleContexts<SQLParser::AssignmentContext>();
}

SQLParser::AssignmentContext* SQLParser::UpdateStatementContext::assignment(size_t i) {
  return getRuleContext<SQLParser::AssignmentContext>(i);
}

std::vector<tree::TerminalNode *> SQLParser::UpdateStatementContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::UpdateStatementContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}

SQLParser::WhereClauseContext* SQLParser::UpdateStatementContext::whereClause() {
  return getRuleContext<SQLParser::WhereClauseContext>(0);
}


size_t SQLParser::UpdateStatementContext::getRuleIndex() const {
  return SQLParser::RuleUpdateStatement;
}


std::any SQLParser::UpdateStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitUpdateStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::UpdateStatementContext* SQLParser::updateStatement() {
  UpdateStatementContext *_localctx = _tracker.createInstance<UpdateStatementContext>(_ctx, getState());
  enterRule(_localctx, 26, SQLParser::RuleUpdateStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(247);
    match(SQLParser::K_UPDATE);
    setState(248);
    identifier();
    setState(249);
    match(SQLParser::K_SET);
    setState(250);
    assignment();
    setState(255);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(251);
      match(SQLParser::COMMA);
      setState(252);
      assignment();
      setState(257);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(259);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_WHERE) {
      setState(258);
      whereClause();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentContext ------------------------------------------------------------------

SQLParser::AssignmentContext::AssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SQLParser::IdentifierContext* SQLParser::AssignmentContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::AssignmentContext::EQ() {
  return getToken(SQLParser::EQ, 0);
}

SQLParser::ExpressionContext* SQLParser::AssignmentContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}


size_t SQLParser::AssignmentContext::getRuleIndex() const {
  return SQLParser::RuleAssignment;
}


std::any SQLParser::AssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitAssignment(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::AssignmentContext* SQLParser::assignment() {
  AssignmentContext *_localctx = _tracker.createInstance<AssignmentContext>(_ctx, getState());
  enterRule(_localctx, 28, SQLParser::RuleAssignment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(261);
    identifier();
    setState(262);
    match(SQLParser::EQ);
    setState(263);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeleteStatementContext ------------------------------------------------------------------

SQLParser::DeleteStatementContext::DeleteStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::DeleteStatementContext::K_DELETE() {
  return getToken(SQLParser::K_DELETE, 0);
}

tree::TerminalNode* SQLParser::DeleteStatementContext::K_FROM() {
  return getToken(SQLParser::K_FROM, 0);
}

SQLParser::IdentifierContext* SQLParser::DeleteStatementContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

SQLParser::WhereClauseContext* SQLParser::DeleteStatementContext::whereClause() {
  return getRuleContext<SQLParser::WhereClauseContext>(0);
}


size_t SQLParser::DeleteStatementContext::getRuleIndex() const {
  return SQLParser::RuleDeleteStatement;
}


std::any SQLParser::DeleteStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitDeleteStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::DeleteStatementContext* SQLParser::deleteStatement() {
  DeleteStatementContext *_localctx = _tracker.createInstance<DeleteStatementContext>(_ctx, getState());
  enterRule(_localctx, 30, SQLParser::RuleDeleteStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(265);
    match(SQLParser::K_DELETE);
    setState(266);
    match(SQLParser::K_FROM);
    setState(267);
    identifier();
    setState(269);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_WHERE) {
      setState(268);
      whereClause();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SelectStatementContext ------------------------------------------------------------------

SQLParser::SelectStatementContext::SelectStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::SelectStatementContext::K_SELECT() {
  return getToken(SQLParser::K_SELECT, 0);
}

std::vector<SQLParser::SelectItemContext *> SQLParser::SelectStatementContext::selectItem() {
  return getRuleContexts<SQLParser::SelectItemContext>();
}

SQLParser::SelectItemContext* SQLParser::SelectStatementContext::selectItem(size_t i) {
  return getRuleContext<SQLParser::SelectItemContext>(i);
}

tree::TerminalNode* SQLParser::SelectStatementContext::K_DISTINCT() {
  return getToken(SQLParser::K_DISTINCT, 0);
}

std::vector<tree::TerminalNode *> SQLParser::SelectStatementContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::SelectStatementContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}

SQLParser::FromClauseContext* SQLParser::SelectStatementContext::fromClause() {
  return getRuleContext<SQLParser::FromClauseContext>(0);
}

SQLParser::WhereClauseContext* SQLParser::SelectStatementContext::whereClause() {
  return getRuleContext<SQLParser::WhereClauseContext>(0);
}

SQLParser::GroupByClauseContext* SQLParser::SelectStatementContext::groupByClause() {
  return getRuleContext<SQLParser::GroupByClauseContext>(0);
}

SQLParser::HavingClauseContext* SQLParser::SelectStatementContext::havingClause() {
  return getRuleContext<SQLParser::HavingClauseContext>(0);
}

SQLParser::OrderByClauseContext* SQLParser::SelectStatementContext::orderByClause() {
  return getRuleContext<SQLParser::OrderByClauseContext>(0);
}

SQLParser::LimitClauseContext* SQLParser::SelectStatementContext::limitClause() {
  return getRuleContext<SQLParser::LimitClauseContext>(0);
}


size_t SQLParser::SelectStatementContext::getRuleIndex() const {
  return SQLParser::RuleSelectStatement;
}


std::any SQLParser::SelectStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitSelectStatement(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::SelectStatementContext* SQLParser::selectStatement() {
  SelectStatementContext *_localctx = _tracker.createInstance<SelectStatementContext>(_ctx, getState());
  enterRule(_localctx, 32, SQLParser::RuleSelectStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(271);
    match(SQLParser::K_SELECT);
    setState(273);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_DISTINCT) {
      setState(272);
      match(SQLParser::K_DISTINCT);
    }
    setState(275);
    selectItem();
    setState(280);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(276);
      match(SQLParser::COMMA);
      setState(277);
      selectItem();
      setState(282);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(284);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_FROM) {
      setState(283);
      fromClause();
    }
    setState(287);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_WHERE) {
      setState(286);
      whereClause();
    }
    setState(290);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_GROUP) {
      setState(289);
      groupByClause();
    }
    setState(293);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_HAVING) {
      setState(292);
      havingClause();
    }
    setState(296);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_ORDER) {
      setState(295);
      orderByClause();
    }
    setState(299);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_LIMIT) {
      setState(298);
      limitClause();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SelectItemContext ------------------------------------------------------------------

SQLParser::SelectItemContext::SelectItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::SelectItemContext::STAR() {
  return getToken(SQLParser::STAR, 0);
}

SQLParser::IdentifierContext* SQLParser::SelectItemContext::identifier() {
  return getRuleContext<SQLParser::IdentifierContext>(0);
}

tree::TerminalNode* SQLParser::SelectItemContext::DOT() {
  return getToken(SQLParser::DOT, 0);
}

SQLParser::ExpressionContext* SQLParser::SelectItemContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::SelectItemContext::K_AS() {
  return getToken(SQLParser::K_AS, 0);
}


size_t SQLParser::SelectItemContext::getRuleIndex() const {
  return SQLParser::RuleSelectItem;
}


std::any SQLParser::SelectItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitSelectItem(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::SelectItemContext* SQLParser::selectItem() {
  SelectItemContext *_localctx = _tracker.createInstance<SelectItemContext>(_ctx, getState());
  enterRule(_localctx, 34, SQLParser::RuleSelectItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(313);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(301);
      match(SQLParser::STAR);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(302);
      identifier();
      setState(303);
      match(SQLParser::DOT);
      setState(304);
      match(SQLParser::STAR);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(306);
      expression(0);
      setState(311);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SQLParser::K_AS

      || _la == SQLParser::IDENTIFIER) {
        setState(308);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SQLParser::K_AS) {
          setState(307);
          match(SQLParser::K_AS);
        }
        setState(310);
        identifier();
      }
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FromClauseContext ------------------------------------------------------------------

SQLParser::FromClauseContext::FromClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::FromClauseContext::K_FROM() {
  return getToken(SQLParser::K_FROM, 0);
}

SQLParser::TableReferenceContext* SQLParser::FromClauseContext::tableReference() {
  return getRuleContext<SQLParser::TableReferenceContext>(0);
}

std::vector<SQLParser::JoinClauseContext *> SQLParser::FromClauseContext::joinClause() {
  return getRuleContexts<SQLParser::JoinClauseContext>();
}

SQLParser::JoinClauseContext* SQLParser::FromClauseContext::joinClause(size_t i) {
  return getRuleContext<SQLParser::JoinClauseContext>(i);
}


size_t SQLParser::FromClauseContext::getRuleIndex() const {
  return SQLParser::RuleFromClause;
}


std::any SQLParser::FromClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitFromClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::FromClauseContext* SQLParser::fromClause() {
  FromClauseContext *_localctx = _tracker.createInstance<FromClauseContext>(_ctx, getState());
  enterRule(_localctx, 36, SQLParser::RuleFromClause);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(315);
    match(SQLParser::K_FROM);
    setState(316);
    tableReference();
    setState(320);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::K_INNER

    || _la == SQLParser::K_JOIN) {
      setState(317);
      joinClause();
      setState(322);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TableReferenceContext ------------------------------------------------------------------

SQLParser::TableReferenceContext::TableReferenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SQLParser::IdentifierContext *> SQLParser::TableReferenceContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::TableReferenceContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

tree::TerminalNode* SQLParser::TableReferenceContext::K_AS() {
  return getToken(SQLParser::K_AS, 0);
}


size_t SQLParser::TableReferenceContext::getRuleIndex() const {
  return SQLParser::RuleTableReference;
}


std::any SQLParser::TableReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitTableReference(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::TableReferenceContext* SQLParser::tableReference() {
  TableReferenceContext *_localctx = _tracker.createInstance<TableReferenceContext>(_ctx, getState());
  enterRule(_localctx, 38, SQLParser::RuleTableReference);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(323);
    identifier();
    setState(328);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_AS

    || _la == SQLParser::IDENTIFIER) {
      setState(325);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SQLParser::K_AS) {
        setState(324);
        match(SQLParser::K_AS);
      }
      setState(327);
      identifier();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- JoinClauseContext ------------------------------------------------------------------

SQLParser::JoinClauseContext::JoinClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::JoinClauseContext::K_JOIN() {
  return getToken(SQLParser::K_JOIN, 0);
}

SQLParser::TableReferenceContext* SQLParser::JoinClauseContext::tableReference() {
  return getRuleContext<SQLParser::TableReferenceContext>(0);
}

tree::TerminalNode* SQLParser::JoinClauseContext::K_ON() {
  return getToken(SQLParser::K_ON, 0);
}

SQLParser::ExpressionContext* SQLParser::JoinClauseContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::JoinClauseContext::K_INNER() {
  return getToken(SQLParser::K_INNER, 0);
}


size_t SQLParser::JoinClauseContext::getRuleIndex() const {
  return SQLParser::RuleJoinClause;
}


std::any SQLParser::JoinClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitJoinClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::JoinClauseContext* SQLParser::joinClause() {
  JoinClauseContext *_localctx = _tracker.createInstance<JoinClauseContext>(_ctx, getState());
  enterRule(_localctx, 40, SQLParser::RuleJoinClause);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(331);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_INNER) {
      setState(330);
      match(SQLParser::K_INNER);
    }
    setState(333);
    match(SQLParser::K_JOIN);
    setState(334);
    tableReference();
    setState(335);
    match(SQLParser::K_ON);
    setState(336);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WhereClauseContext ------------------------------------------------------------------

SQLParser::WhereClauseContext::WhereClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::WhereClauseContext::K_WHERE() {
  return getToken(SQLParser::K_WHERE, 0);
}

SQLParser::ExpressionContext* SQLParser::WhereClauseContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}


size_t SQLParser::WhereClauseContext::getRuleIndex() const {
  return SQLParser::RuleWhereClause;
}


std::any SQLParser::WhereClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitWhereClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::WhereClauseContext* SQLParser::whereClause() {
  WhereClauseContext *_localctx = _tracker.createInstance<WhereClauseContext>(_ctx, getState());
  enterRule(_localctx, 42, SQLParser::RuleWhereClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(338);
    match(SQLParser::K_WHERE);
    setState(339);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GroupByClauseContext ------------------------------------------------------------------

SQLParser::GroupByClauseContext::GroupByClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::GroupByClauseContext::K_GROUP() {
  return getToken(SQLParser::K_GROUP, 0);
}

tree::TerminalNode* SQLParser::GroupByClauseContext::K_BY() {
  return getToken(SQLParser::K_BY, 0);
}

std::vector<SQLParser::ExpressionContext *> SQLParser::GroupByClauseContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::GroupByClauseContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> SQLParser::GroupByClauseContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::GroupByClauseContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}


size_t SQLParser::GroupByClauseContext::getRuleIndex() const {
  return SQLParser::RuleGroupByClause;
}


std::any SQLParser::GroupByClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitGroupByClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::GroupByClauseContext* SQLParser::groupByClause() {
  GroupByClauseContext *_localctx = _tracker.createInstance<GroupByClauseContext>(_ctx, getState());
  enterRule(_localctx, 44, SQLParser::RuleGroupByClause);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(341);
    match(SQLParser::K_GROUP);
    setState(342);
    match(SQLParser::K_BY);
    setState(343);
    expression(0);
    setState(348);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(344);
      match(SQLParser::COMMA);
      setState(345);
      expression(0);
      setState(350);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- HavingClauseContext ------------------------------------------------------------------

SQLParser::HavingClauseContext::HavingClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::HavingClauseContext::K_HAVING() {
  return getToken(SQLParser::K_HAVING, 0);
}

SQLParser::ExpressionContext* SQLParser::HavingClauseContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}


size_t SQLParser::HavingClauseContext::getRuleIndex() const {
  return SQLParser::RuleHavingClause;
}


std::any SQLParser::HavingClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitHavingClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::HavingClauseContext* SQLParser::havingClause() {
  HavingClauseContext *_localctx = _tracker.createInstance<HavingClauseContext>(_ctx, getState());
  enterRule(_localctx, 46, SQLParser::RuleHavingClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(351);
    match(SQLParser::K_HAVING);
    setState(352);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OrderByClauseContext ------------------------------------------------------------------

SQLParser::OrderByClauseContext::OrderByClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::OrderByClauseContext::K_ORDER() {
  return getToken(SQLParser::K_ORDER, 0);
}

tree::TerminalNode* SQLParser::OrderByClauseContext::K_BY() {
  return getToken(SQLParser::K_BY, 0);
}

std::vector<SQLParser::OrderItemContext *> SQLParser::OrderByClauseContext::orderItem() {
  return getRuleContexts<SQLParser::OrderItemContext>();
}

SQLParser::OrderItemContext* SQLParser::OrderByClauseContext::orderItem(size_t i) {
  return getRuleContext<SQLParser::OrderItemContext>(i);
}

std::vector<tree::TerminalNode *> SQLParser::OrderByClauseContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::OrderByClauseContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}


size_t SQLParser::OrderByClauseContext::getRuleIndex() const {
  return SQLParser::RuleOrderByClause;
}


std::any SQLParser::OrderByClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitOrderByClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::OrderByClauseContext* SQLParser::orderByClause() {
  OrderByClauseContext *_localctx = _tracker.createInstance<OrderByClauseContext>(_ctx, getState());
  enterRule(_localctx, 48, SQLParser::RuleOrderByClause);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(354);
    match(SQLParser::K_ORDER);
    setState(355);
    match(SQLParser::K_BY);
    setState(356);
    orderItem();
    setState(361);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SQLParser::COMMA) {
      setState(357);
      match(SQLParser::COMMA);
      setState(358);
      orderItem();
      setState(363);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OrderItemContext ------------------------------------------------------------------

SQLParser::OrderItemContext::OrderItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SQLParser::ExpressionContext* SQLParser::OrderItemContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::OrderItemContext::K_ASC() {
  return getToken(SQLParser::K_ASC, 0);
}

tree::TerminalNode* SQLParser::OrderItemContext::K_DESC() {
  return getToken(SQLParser::K_DESC, 0);
}


size_t SQLParser::OrderItemContext::getRuleIndex() const {
  return SQLParser::RuleOrderItem;
}


std::any SQLParser::OrderItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitOrderItem(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::OrderItemContext* SQLParser::orderItem() {
  OrderItemContext *_localctx = _tracker.createInstance<OrderItemContext>(_ctx, getState());
  enterRule(_localctx, 50, SQLParser::RuleOrderItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(364);
    expression(0);
    setState(366);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SQLParser::K_ASC

    || _la == SQLParser::K_DESC) {
      setState(365);
      _la = _input->LA(1);
      if (!(_la == SQLParser::K_ASC

      || _la == SQLParser::K_DESC)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LimitClauseContext ------------------------------------------------------------------

SQLParser::LimitClauseContext::LimitClauseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::LimitClauseContext::K_LIMIT() {
  return getToken(SQLParser::K_LIMIT, 0);
}

tree::TerminalNode* SQLParser::LimitClauseContext::INT_LITERAL() {
  return getToken(SQLParser::INT_LITERAL, 0);
}


size_t SQLParser::LimitClauseContext::getRuleIndex() const {
  return SQLParser::RuleLimitClause;
}


std::any SQLParser::LimitClauseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitLimitClause(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::LimitClauseContext* SQLParser::limitClause() {
  LimitClauseContext *_localctx = _tracker.createInstance<LimitClauseContext>(_ctx, getState());
  enterRule(_localctx, 52, SQLParser::RuleLimitClause);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(368);
    match(SQLParser::K_LIMIT);
    setState(369);
    match(SQLParser::INT_LITERAL);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

SQLParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SQLParser::ExpressionContext::getRuleIndex() const {
  return SQLParser::RuleExpression;
}

void SQLParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- PrimaryExpressionContext ------------------------------------------------------------------

SQLParser::PrimaryContext* SQLParser::PrimaryExpressionContext::primary() {
  return getRuleContext<SQLParser::PrimaryContext>(0);
}

SQLParser::PrimaryExpressionContext::PrimaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::PrimaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::OrExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::OrExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::OrExpressionContext::K_OR() {
  return getToken(SQLParser::K_OR, 0);
}

SQLParser::OrExpressionContext::OrExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::OrExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitOrExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::AndExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::AndExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::AndExpressionContext::K_AND() {
  return getToken(SQLParser::K_AND, 0);
}

SQLParser::AndExpressionContext::AndExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::AndExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitAndExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InSubqueryExpressionContext ------------------------------------------------------------------

SQLParser::ExpressionContext* SQLParser::InSubqueryExpressionContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::InSubqueryExpressionContext::K_IN() {
  return getToken(SQLParser::K_IN, 0);
}

tree::TerminalNode* SQLParser::InSubqueryExpressionContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

SQLParser::SelectStatementContext* SQLParser::InSubqueryExpressionContext::selectStatement() {
  return getRuleContext<SQLParser::SelectStatementContext>(0);
}

tree::TerminalNode* SQLParser::InSubqueryExpressionContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

tree::TerminalNode* SQLParser::InSubqueryExpressionContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

SQLParser::InSubqueryExpressionContext::InSubqueryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::InSubqueryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitInSubqueryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AdditiveExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::AdditiveExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::AdditiveExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::AdditiveExpressionContext::PLUS() {
  return getToken(SQLParser::PLUS, 0);
}

tree::TerminalNode* SQLParser::AdditiveExpressionContext::MINUS() {
  return getToken(SQLParser::MINUS, 0);
}

SQLParser::AdditiveExpressionContext::AdditiveExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- InListExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::InListExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::InListExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::InListExpressionContext::K_IN() {
  return getToken(SQLParser::K_IN, 0);
}

tree::TerminalNode* SQLParser::InListExpressionContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::InListExpressionContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

tree::TerminalNode* SQLParser::InListExpressionContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

std::vector<tree::TerminalNode *> SQLParser::InListExpressionContext::COMMA() {
  return getTokens(SQLParser::COMMA);
}

tree::TerminalNode* SQLParser::InListExpressionContext::COMMA(size_t i) {
  return getToken(SQLParser::COMMA, i);
}

SQLParser::InListExpressionContext::InListExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::InListExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitInListExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotExpressionContext ------------------------------------------------------------------

tree::TerminalNode* SQLParser::NotExpressionContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

SQLParser::ExpressionContext* SQLParser::NotExpressionContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

SQLParser::NotExpressionContext::NotExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::NotExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitNotExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ComparisonExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::ComparisonExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::ComparisonExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::EQ() {
  return getToken(SQLParser::EQ, 0);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::NEQ() {
  return getToken(SQLParser::NEQ, 0);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::LT() {
  return getToken(SQLParser::LT, 0);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::LTE() {
  return getToken(SQLParser::LTE, 0);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::GT() {
  return getToken(SQLParser::GT, 0);
}

tree::TerminalNode* SQLParser::ComparisonExpressionContext::GTE() {
  return getToken(SQLParser::GTE, 0);
}

SQLParser::ComparisonExpressionContext::ComparisonExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::ComparisonExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitComparisonExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MultiplicativeExpressionContext ------------------------------------------------------------------

std::vector<SQLParser::ExpressionContext *> SQLParser::MultiplicativeExpressionContext::expression() {
  return getRuleContexts<SQLParser::ExpressionContext>();
}

SQLParser::ExpressionContext* SQLParser::MultiplicativeExpressionContext::expression(size_t i) {
  return getRuleContext<SQLParser::ExpressionContext>(i);
}

tree::TerminalNode* SQLParser::MultiplicativeExpressionContext::STAR() {
  return getToken(SQLParser::STAR, 0);
}

tree::TerminalNode* SQLParser::MultiplicativeExpressionContext::SLASH() {
  return getToken(SQLParser::SLASH, 0);
}

tree::TerminalNode* SQLParser::MultiplicativeExpressionContext::PERCENT() {
  return getToken(SQLParser::PERCENT, 0);
}

SQLParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- UnaryExpressionContext ------------------------------------------------------------------

SQLParser::ExpressionContext* SQLParser::UnaryExpressionContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::UnaryExpressionContext::MINUS() {
  return getToken(SQLParser::MINUS, 0);
}

tree::TerminalNode* SQLParser::UnaryExpressionContext::PLUS() {
  return getToken(SQLParser::PLUS, 0);
}

SQLParser::UnaryExpressionContext::UnaryExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::UnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IsNullExpressionContext ------------------------------------------------------------------

SQLParser::ExpressionContext* SQLParser::IsNullExpressionContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::IsNullExpressionContext::K_IS() {
  return getToken(SQLParser::K_IS, 0);
}

tree::TerminalNode* SQLParser::IsNullExpressionContext::K_NULL() {
  return getToken(SQLParser::K_NULL, 0);
}

tree::TerminalNode* SQLParser::IsNullExpressionContext::K_NOT() {
  return getToken(SQLParser::K_NOT, 0);
}

SQLParser::IsNullExpressionContext::IsNullExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }


std::any SQLParser::IsNullExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitIsNullExpression(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ExpressionContext* SQLParser::expression() {
   return expression(0);
}

SQLParser::ExpressionContext* SQLParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SQLParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  SQLParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 54;
  enterRecursionRule(_localctx, 54, SQLParser::RuleExpression, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(377);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::K_EXISTS:
      case SQLParser::K_NULL:
      case SQLParser::K_COUNT:
      case SQLParser::K_SUM:
      case SQLParser::K_AVG:
      case SQLParser::K_MIN:
      case SQLParser::K_MAX:
      case SQLParser::K_DATE:
      case SQLParser::K_TRUE:
      case SQLParser::K_FALSE:
      case SQLParser::LPAREN:
      case SQLParser::FLOAT_LITERAL:
      case SQLParser::INT_LITERAL:
      case SQLParser::STRING_LITERAL:
      case SQLParser::IDENTIFIER: {
        _localctx = _tracker.createInstance<PrimaryExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(372);
        primary();
        break;
      }

      case SQLParser::PLUS:
      case SQLParser::MINUS: {
        _localctx = _tracker.createInstance<UnaryExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(373);
        antlrcpp::downCast<UnaryExpressionContext *>(_localctx)->op = _input->LT(1);
        _la = _input->LA(1);
        if (!(_la == SQLParser::PLUS

        || _la == SQLParser::MINUS)) {
          antlrcpp::downCast<UnaryExpressionContext *>(_localctx)->op = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(374);
        expression(10);
        break;
      }

      case SQLParser::K_NOT: {
        _localctx = _tracker.createInstance<NotExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(375);
        match(SQLParser::K_NOT);
        setState(376);
        expression(3);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(427);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(425);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<MultiplicativeExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(379);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(380);
          antlrcpp::downCast<MultiplicativeExpressionContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 4035225266123964416) != 0))) {
            antlrcpp::downCast<MultiplicativeExpressionContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(381);
          expression(10);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<AdditiveExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(382);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(383);
          antlrcpp::downCast<AdditiveExpressionContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == SQLParser::PLUS

          || _la == SQLParser::MINUS)) {
            antlrcpp::downCast<AdditiveExpressionContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(384);
          expression(9);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<ComparisonExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(385);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(386);
          antlrcpp::downCast<ComparisonExpressionContext *>(_localctx)->op = _input->LT(1);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 141863388262170624) != 0))) {
            antlrcpp::downCast<ComparisonExpressionContext *>(_localctx)->op = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(387);
          expression(8);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<AndExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(388);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(389);
          match(SQLParser::K_AND);
          setState(390);
          expression(3);
          break;
        }

        case 5: {
          auto newContext = _tracker.createInstance<OrExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(391);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(392);
          match(SQLParser::K_OR);
          setState(393);
          expression(2);
          break;
        }

        case 6: {
          auto newContext = _tracker.createInstance<IsNullExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(394);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(395);
          match(SQLParser::K_IS);
          setState(397);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SQLParser::K_NOT) {
            setState(396);
            match(SQLParser::K_NOT);
          }
          setState(399);
          match(SQLParser::K_NULL);
          break;
        }

        case 7: {
          auto newContext = _tracker.createInstance<InSubqueryExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(400);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(402);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SQLParser::K_NOT) {
            setState(401);
            match(SQLParser::K_NOT);
          }
          setState(404);
          match(SQLParser::K_IN);
          setState(405);
          match(SQLParser::LPAREN);
          setState(406);
          selectStatement();
          setState(407);
          match(SQLParser::RPAREN);
          break;
        }

        case 8: {
          auto newContext = _tracker.createInstance<InListExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(409);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(411);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SQLParser::K_NOT) {
            setState(410);
            match(SQLParser::K_NOT);
          }
          setState(413);
          match(SQLParser::K_IN);
          setState(414);
          match(SQLParser::LPAREN);
          setState(415);
          expression(0);
          setState(420);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SQLParser::COMMA) {
            setState(416);
            match(SQLParser::COMMA);
            setState(417);
            expression(0);
            setState(422);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
          setState(423);
          match(SQLParser::RPAREN);
          break;
        }

        default:
          break;
        } 
      }
      setState(429);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 47, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PrimaryContext ------------------------------------------------------------------

SQLParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t SQLParser::PrimaryContext::getRuleIndex() const {
  return SQLParser::RulePrimary;
}

void SQLParser::PrimaryContext::copyFrom(PrimaryContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ColumnPrimaryContext ------------------------------------------------------------------

SQLParser::ColumnReferenceContext* SQLParser::ColumnPrimaryContext::columnReference() {
  return getRuleContext<SQLParser::ColumnReferenceContext>(0);
}

SQLParser::ColumnPrimaryContext::ColumnPrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::ColumnPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitColumnPrimary(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenthesizedPrimaryContext ------------------------------------------------------------------

tree::TerminalNode* SQLParser::ParenthesizedPrimaryContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

SQLParser::ExpressionContext* SQLParser::ParenthesizedPrimaryContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::ParenthesizedPrimaryContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

SQLParser::ParenthesizedPrimaryContext::ParenthesizedPrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::ParenthesizedPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitParenthesizedPrimary(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SubqueryPrimaryContext ------------------------------------------------------------------

tree::TerminalNode* SQLParser::SubqueryPrimaryContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

SQLParser::SelectStatementContext* SQLParser::SubqueryPrimaryContext::selectStatement() {
  return getRuleContext<SQLParser::SelectStatementContext>(0);
}

tree::TerminalNode* SQLParser::SubqueryPrimaryContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

SQLParser::SubqueryPrimaryContext::SubqueryPrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::SubqueryPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitSubqueryPrimary(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExistsPrimaryContext ------------------------------------------------------------------

tree::TerminalNode* SQLParser::ExistsPrimaryContext::K_EXISTS() {
  return getToken(SQLParser::K_EXISTS, 0);
}

tree::TerminalNode* SQLParser::ExistsPrimaryContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

SQLParser::SelectStatementContext* SQLParser::ExistsPrimaryContext::selectStatement() {
  return getRuleContext<SQLParser::SelectStatementContext>(0);
}

tree::TerminalNode* SQLParser::ExistsPrimaryContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

SQLParser::ExistsPrimaryContext::ExistsPrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::ExistsPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitExistsPrimary(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LiteralPrimaryContext ------------------------------------------------------------------

SQLParser::LiteralContext* SQLParser::LiteralPrimaryContext::literal() {
  return getRuleContext<SQLParser::LiteralContext>(0);
}

SQLParser::LiteralPrimaryContext::LiteralPrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::LiteralPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitLiteralPrimary(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AggregatePrimaryContext ------------------------------------------------------------------

SQLParser::AggregateFunctionContext* SQLParser::AggregatePrimaryContext::aggregateFunction() {
  return getRuleContext<SQLParser::AggregateFunctionContext>(0);
}

SQLParser::AggregatePrimaryContext::AggregatePrimaryContext(PrimaryContext *ctx) { copyFrom(ctx); }


std::any SQLParser::AggregatePrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitAggregatePrimary(this);
  else
    return visitor->visitChildren(this);
}
SQLParser::PrimaryContext* SQLParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 56, SQLParser::RulePrimary);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(446);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<SQLParser::LiteralPrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(430);
      literal();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<SQLParser::AggregatePrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(431);
      aggregateFunction();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SQLParser::ColumnPrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(432);
      columnReference();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<SQLParser::ExistsPrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(433);
      match(SQLParser::K_EXISTS);
      setState(434);
      match(SQLParser::LPAREN);
      setState(435);
      selectStatement();
      setState(436);
      match(SQLParser::RPAREN);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<SQLParser::SubqueryPrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(438);
      match(SQLParser::LPAREN);
      setState(439);
      selectStatement();
      setState(440);
      match(SQLParser::RPAREN);
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<SQLParser::ParenthesizedPrimaryContext>(_localctx);
      enterOuterAlt(_localctx, 6);
      setState(442);
      match(SQLParser::LPAREN);
      setState(443);
      expression(0);
      setState(444);
      match(SQLParser::RPAREN);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AggregateFunctionContext ------------------------------------------------------------------

SQLParser::AggregateFunctionContext::AggregateFunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_COUNT() {
  return getToken(SQLParser::K_COUNT, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::LPAREN() {
  return getToken(SQLParser::LPAREN, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::STAR() {
  return getToken(SQLParser::STAR, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::RPAREN() {
  return getToken(SQLParser::RPAREN, 0);
}

SQLParser::ExpressionContext* SQLParser::AggregateFunctionContext::expression() {
  return getRuleContext<SQLParser::ExpressionContext>(0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_SUM() {
  return getToken(SQLParser::K_SUM, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_AVG() {
  return getToken(SQLParser::K_AVG, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_MIN() {
  return getToken(SQLParser::K_MIN, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_MAX() {
  return getToken(SQLParser::K_MAX, 0);
}

tree::TerminalNode* SQLParser::AggregateFunctionContext::K_DISTINCT() {
  return getToken(SQLParser::K_DISTINCT, 0);
}


size_t SQLParser::AggregateFunctionContext::getRuleIndex() const {
  return SQLParser::RuleAggregateFunction;
}


std::any SQLParser::AggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::AggregateFunctionContext* SQLParser::aggregateFunction() {
  AggregateFunctionContext *_localctx = _tracker.createInstance<AggregateFunctionContext>(_ctx, getState());
  enterRule(_localctx, 58, SQLParser::RuleAggregateFunction);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(460);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 50, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(448);
      match(SQLParser::K_COUNT);
      setState(449);
      match(SQLParser::LPAREN);
      setState(450);
      match(SQLParser::STAR);
      setState(451);
      match(SQLParser::RPAREN);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(452);
      antlrcpp::downCast<AggregateFunctionContext *>(_localctx)->name = _input->LT(1);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 532575944704) != 0))) {
        antlrcpp::downCast<AggregateFunctionContext *>(_localctx)->name = _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(453);
      match(SQLParser::LPAREN);
      setState(455);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == SQLParser::K_DISTINCT) {
        setState(454);
        match(SQLParser::K_DISTINCT);
      }
      setState(457);
      expression(0);
      setState(458);
      match(SQLParser::RPAREN);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ColumnReferenceContext ------------------------------------------------------------------

SQLParser::ColumnReferenceContext::ColumnReferenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SQLParser::IdentifierContext *> SQLParser::ColumnReferenceContext::identifier() {
  return getRuleContexts<SQLParser::IdentifierContext>();
}

SQLParser::IdentifierContext* SQLParser::ColumnReferenceContext::identifier(size_t i) {
  return getRuleContext<SQLParser::IdentifierContext>(i);
}

tree::TerminalNode* SQLParser::ColumnReferenceContext::DOT() {
  return getToken(SQLParser::DOT, 0);
}


size_t SQLParser::ColumnReferenceContext::getRuleIndex() const {
  return SQLParser::RuleColumnReference;
}


std::any SQLParser::ColumnReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitColumnReference(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::ColumnReferenceContext* SQLParser::columnReference() {
  ColumnReferenceContext *_localctx = _tracker.createInstance<ColumnReferenceContext>(_ctx, getState());
  enterRule(_localctx, 60, SQLParser::RuleColumnReference);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(462);
    identifier();
    setState(465);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      setState(463);
      match(SQLParser::DOT);
      setState(464);
      identifier();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LiteralContext ------------------------------------------------------------------

SQLParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::LiteralContext::INT_LITERAL() {
  return getToken(SQLParser::INT_LITERAL, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::FLOAT_LITERAL() {
  return getToken(SQLParser::FLOAT_LITERAL, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::STRING_LITERAL() {
  return getToken(SQLParser::STRING_LITERAL, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::K_TRUE() {
  return getToken(SQLParser::K_TRUE, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::K_FALSE() {
  return getToken(SQLParser::K_FALSE, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::K_NULL() {
  return getToken(SQLParser::K_NULL, 0);
}

tree::TerminalNode* SQLParser::LiteralContext::K_DATE() {
  return getToken(SQLParser::K_DATE, 0);
}


size_t SQLParser::LiteralContext::getRuleIndex() const {
  return SQLParser::RuleLiteral;
}


std::any SQLParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::LiteralContext* SQLParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 62, SQLParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(475);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SQLParser::INT_LITERAL: {
        enterOuterAlt(_localctx, 1);
        setState(467);
        match(SQLParser::INT_LITERAL);
        break;
      }

      case SQLParser::FLOAT_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(468);
        match(SQLParser::FLOAT_LITERAL);
        break;
      }

      case SQLParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 3);
        setState(469);
        match(SQLParser::STRING_LITERAL);
        break;
      }

      case SQLParser::K_TRUE: {
        enterOuterAlt(_localctx, 4);
        setState(470);
        match(SQLParser::K_TRUE);
        break;
      }

      case SQLParser::K_FALSE: {
        enterOuterAlt(_localctx, 5);
        setState(471);
        match(SQLParser::K_FALSE);
        break;
      }

      case SQLParser::K_NULL: {
        enterOuterAlt(_localctx, 6);
        setState(472);
        match(SQLParser::K_NULL);
        break;
      }

      case SQLParser::K_DATE: {
        enterOuterAlt(_localctx, 7);
        setState(473);
        match(SQLParser::K_DATE);
        setState(474);
        match(SQLParser::STRING_LITERAL);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdentifierContext ------------------------------------------------------------------

SQLParser::IdentifierContext::IdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SQLParser::IdentifierContext::IDENTIFIER() {
  return getToken(SQLParser::IDENTIFIER, 0);
}


size_t SQLParser::IdentifierContext::getRuleIndex() const {
  return SQLParser::RuleIdentifier;
}


std::any SQLParser::IdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SQLVisitor*>(visitor))
    return parserVisitor->visitIdentifier(this);
  else
    return visitor->visitChildren(this);
}

SQLParser::IdentifierContext* SQLParser::identifier() {
  IdentifierContext *_localctx = _tracker.createInstance<IdentifierContext>(_ctx, getState());
  enterRule(_localctx, 64, SQLParser::RuleIdentifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(477);
    match(SQLParser::IDENTIFIER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SQLParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 27: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SQLParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 9);
    case 1: return precpred(_ctx, 8);
    case 2: return precpred(_ctx, 7);
    case 3: return precpred(_ctx, 2);
    case 4: return precpred(_ctx, 1);
    case 5: return precpred(_ctx, 6);
    case 6: return precpred(_ctx, 5);
    case 7: return precpred(_ctx, 4);

  default:
    break;
  }
  return true;
}

void SQLParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  sqlParserInitialize();
#else
  ::antlr4::internal::call_once(sqlParserOnceFlag, sqlParserInitialize);
#endif
}
