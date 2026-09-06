
// Generated from Compiscript.g4 by ANTLR 4.13.2


#include "CompiscriptVisitor.h"

#include "CompiscriptParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct CompiscriptParserStaticData final {
  CompiscriptParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  CompiscriptParserStaticData(const CompiscriptParserStaticData&) = delete;
  CompiscriptParserStaticData(CompiscriptParserStaticData&&) = delete;
  CompiscriptParserStaticData& operator=(const CompiscriptParserStaticData&) = delete;
  CompiscriptParserStaticData& operator=(CompiscriptParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag compiscriptParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<CompiscriptParserStaticData> compiscriptParserStaticData = nullptr;

void compiscriptParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (compiscriptParserStaticData != nullptr) {
    return;
  }
#else
  assert(compiscriptParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<CompiscriptParserStaticData>(
    std::vector<std::string>{
      "program", "statement", "block", "controlBody", "variableDeclaration", 
      "constantDeclaration", "typeAnnotation", "initializer", "assignmentStatement", 
      "expressionStatement", "printStatement", "ifStatement", "whileStatement", 
      "doWhileStatement", "forStatement", "foreachStatement", "breakStatement", 
      "continueStatement", "returnStatement", "tryCatchStatement", "switchStatement", 
      "switchCase", "defaultCase", "functionDeclaration", "parameters", 
      "parameter", "classDeclaration", "classMember", "expression", "assignmentExpr", 
      "conditionalExpr", "logicalOrExpr", "logicalAndExpr", "equalityExpr", 
      "relationalExpr", "additiveExpr", "multiplicativeExpr", "unaryExpr", 
      "primaryExpr", "literalExpr", "leftHandSide", "primaryAtom", "suffixOp", 
      "arguments", "arrayLiteral", "type", "baseType"
    },
    std::vector<std::string>{
      "", "'{'", "'}'", "'let'", "'var'", "';'", "'const'", "'='", "':'", 
      "'print'", "'('", "')'", "'if'", "'else'", "'while'", "'do'", "'for'", 
      "'foreach'", "'in'", "'break'", "'continue'", "'return'", "'try'", 
      "'catch'", "'switch'", "'case'", "'default'", "'function'", "','", 
      "'class'", "'\\u003F'", "'||'", "'&&'", "'=='", "'!='", "'<'", "'<='", 
      "'>'", "'>='", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "'null'", 
      "'true'", "'false'", "'new'", "'this'", "'['", "']'", "'.'", "'boolean'", 
      "'integer'", "'float'", "'string'", "'void'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
      "", "", "", "", "", "", "", "FloatLiteral", "IntegerLiteral", "StringLiteral", 
      "Identifier", "WS", "COMMENT", "MULTILINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,64,471,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,1,0,5,0,96,8,0,10,0,12,0,99,
  	9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,3,1,121,8,1,1,2,1,2,5,2,125,8,2,10,2,12,2,128,9,2,1,2,1,
  	2,1,3,1,3,3,3,134,8,3,1,4,1,4,1,4,3,4,139,8,4,1,4,3,4,142,8,4,1,4,1,4,
  	1,5,1,5,1,5,3,5,149,8,5,1,5,1,5,1,5,1,5,1,6,1,6,1,6,1,7,1,7,1,7,1,8,1,
  	8,1,8,1,8,1,8,1,9,1,9,1,9,1,10,1,10,1,10,1,10,1,10,1,10,1,11,1,11,1,11,
  	1,11,1,11,1,11,1,11,3,11,182,8,11,1,12,1,12,1,12,1,12,1,12,1,12,1,13,
  	1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,14,1,14,1,14,1,14,1,14,3,14,203,
  	8,14,1,14,3,14,206,8,14,1,14,1,14,3,14,210,8,14,1,14,1,14,1,14,1,15,1,
  	15,1,15,1,15,1,15,1,15,1,15,1,15,1,16,1,16,1,16,1,17,1,17,1,17,1,18,1,
  	18,3,18,231,8,18,1,18,1,18,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,19,1,
  	20,1,20,1,20,1,20,1,20,1,20,5,20,249,8,20,10,20,12,20,252,9,20,1,20,3,
  	20,255,8,20,1,20,1,20,1,21,1,21,1,21,1,21,5,21,263,8,21,10,21,12,21,266,
  	9,21,1,22,1,22,1,22,5,22,271,8,22,10,22,12,22,274,9,22,1,23,1,23,1,23,
  	1,23,3,23,280,8,23,1,23,1,23,1,23,3,23,285,8,23,1,23,1,23,1,24,1,24,1,
  	24,5,24,292,8,24,10,24,12,24,295,9,24,1,25,1,25,1,25,3,25,300,8,25,1,
  	26,1,26,1,26,1,26,3,26,306,8,26,1,26,1,26,5,26,310,8,26,10,26,12,26,313,
  	9,26,1,26,1,26,1,27,1,27,1,27,3,27,320,8,27,1,28,1,28,1,29,1,29,1,29,
  	1,29,1,29,3,29,329,8,29,1,30,1,30,1,30,1,30,1,30,1,30,3,30,337,8,30,1,
  	31,1,31,1,31,5,31,342,8,31,10,31,12,31,345,9,31,1,32,1,32,1,32,5,32,350,
  	8,32,10,32,12,32,353,9,32,1,33,1,33,1,33,5,33,358,8,33,10,33,12,33,361,
  	9,33,1,34,1,34,1,34,5,34,366,8,34,10,34,12,34,369,9,34,1,35,1,35,1,35,
  	5,35,374,8,35,10,35,12,35,377,9,35,1,36,1,36,1,36,5,36,382,8,36,10,36,
  	12,36,385,9,36,1,37,1,37,1,37,3,37,390,8,37,1,38,1,38,1,38,1,38,1,38,
  	1,38,3,38,398,8,38,1,39,1,39,1,39,1,39,1,39,1,39,1,39,3,39,407,8,39,1,
  	40,1,40,5,40,411,8,40,10,40,12,40,414,9,40,1,41,1,41,1,41,1,41,1,41,3,
  	41,421,8,41,1,41,1,41,3,41,425,8,41,1,42,1,42,3,42,429,8,42,1,42,1,42,
  	1,42,1,42,1,42,1,42,1,42,3,42,438,8,42,1,43,1,43,1,43,5,43,443,8,43,10,
  	43,12,43,446,9,43,1,44,1,44,1,44,1,44,5,44,452,8,44,10,44,12,44,455,9,
  	44,3,44,457,8,44,1,44,1,44,1,45,1,45,1,45,5,45,464,8,45,10,45,12,45,467,
  	9,45,1,46,1,46,1,46,0,0,47,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
  	32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,
  	78,80,82,84,86,88,90,92,0,7,1,0,3,4,1,0,33,34,1,0,35,38,1,0,39,40,1,0,
  	41,43,2,0,40,40,44,44,2,0,53,57,61,61,492,0,97,1,0,0,0,2,120,1,0,0,0,
  	4,122,1,0,0,0,6,133,1,0,0,0,8,135,1,0,0,0,10,145,1,0,0,0,12,154,1,0,0,
  	0,14,157,1,0,0,0,16,160,1,0,0,0,18,165,1,0,0,0,20,168,1,0,0,0,22,174,
  	1,0,0,0,24,183,1,0,0,0,26,189,1,0,0,0,28,197,1,0,0,0,30,214,1,0,0,0,32,
  	222,1,0,0,0,34,225,1,0,0,0,36,228,1,0,0,0,38,234,1,0,0,0,40,242,1,0,0,
  	0,42,258,1,0,0,0,44,267,1,0,0,0,46,275,1,0,0,0,48,288,1,0,0,0,50,296,
  	1,0,0,0,52,301,1,0,0,0,54,319,1,0,0,0,56,321,1,0,0,0,58,328,1,0,0,0,60,
  	330,1,0,0,0,62,338,1,0,0,0,64,346,1,0,0,0,66,354,1,0,0,0,68,362,1,0,0,
  	0,70,370,1,0,0,0,72,378,1,0,0,0,74,389,1,0,0,0,76,397,1,0,0,0,78,406,
  	1,0,0,0,80,408,1,0,0,0,82,424,1,0,0,0,84,437,1,0,0,0,86,439,1,0,0,0,88,
  	447,1,0,0,0,90,460,1,0,0,0,92,468,1,0,0,0,94,96,3,2,1,0,95,94,1,0,0,0,
  	96,99,1,0,0,0,97,95,1,0,0,0,97,98,1,0,0,0,98,100,1,0,0,0,99,97,1,0,0,
  	0,100,101,5,0,0,1,101,1,1,0,0,0,102,121,3,8,4,0,103,121,3,10,5,0,104,
  	121,3,16,8,0,105,121,3,46,23,0,106,121,3,52,26,0,107,121,3,18,9,0,108,
  	121,3,20,10,0,109,121,3,4,2,0,110,121,3,22,11,0,111,121,3,24,12,0,112,
  	121,3,26,13,0,113,121,3,28,14,0,114,121,3,30,15,0,115,121,3,38,19,0,116,
  	121,3,40,20,0,117,121,3,32,16,0,118,121,3,34,17,0,119,121,3,36,18,0,120,
  	102,1,0,0,0,120,103,1,0,0,0,120,104,1,0,0,0,120,105,1,0,0,0,120,106,1,
  	0,0,0,120,107,1,0,0,0,120,108,1,0,0,0,120,109,1,0,0,0,120,110,1,0,0,0,
  	120,111,1,0,0,0,120,112,1,0,0,0,120,113,1,0,0,0,120,114,1,0,0,0,120,115,
  	1,0,0,0,120,116,1,0,0,0,120,117,1,0,0,0,120,118,1,0,0,0,120,119,1,0,0,
  	0,121,3,1,0,0,0,122,126,5,1,0,0,123,125,3,2,1,0,124,123,1,0,0,0,125,128,
  	1,0,0,0,126,124,1,0,0,0,126,127,1,0,0,0,127,129,1,0,0,0,128,126,1,0,0,
  	0,129,130,5,2,0,0,130,5,1,0,0,0,131,134,3,4,2,0,132,134,3,2,1,0,133,131,
  	1,0,0,0,133,132,1,0,0,0,134,7,1,0,0,0,135,136,7,0,0,0,136,138,5,61,0,
  	0,137,139,3,12,6,0,138,137,1,0,0,0,138,139,1,0,0,0,139,141,1,0,0,0,140,
  	142,3,14,7,0,141,140,1,0,0,0,141,142,1,0,0,0,142,143,1,0,0,0,143,144,
  	5,5,0,0,144,9,1,0,0,0,145,146,5,6,0,0,146,148,5,61,0,0,147,149,3,12,6,
  	0,148,147,1,0,0,0,148,149,1,0,0,0,149,150,1,0,0,0,150,151,5,7,0,0,151,
  	152,3,56,28,0,152,153,5,5,0,0,153,11,1,0,0,0,154,155,5,8,0,0,155,156,
  	3,90,45,0,156,13,1,0,0,0,157,158,5,7,0,0,158,159,3,56,28,0,159,15,1,0,
  	0,0,160,161,3,80,40,0,161,162,5,7,0,0,162,163,3,56,28,0,163,164,5,5,0,
  	0,164,17,1,0,0,0,165,166,3,56,28,0,166,167,5,5,0,0,167,19,1,0,0,0,168,
  	169,5,9,0,0,169,170,5,10,0,0,170,171,3,56,28,0,171,172,5,11,0,0,172,173,
  	5,5,0,0,173,21,1,0,0,0,174,175,5,12,0,0,175,176,5,10,0,0,176,177,3,56,
  	28,0,177,178,5,11,0,0,178,181,3,6,3,0,179,180,5,13,0,0,180,182,3,6,3,
  	0,181,179,1,0,0,0,181,182,1,0,0,0,182,23,1,0,0,0,183,184,5,14,0,0,184,
  	185,5,10,0,0,185,186,3,56,28,0,186,187,5,11,0,0,187,188,3,6,3,0,188,25,
  	1,0,0,0,189,190,5,15,0,0,190,191,3,6,3,0,191,192,5,14,0,0,192,193,5,10,
  	0,0,193,194,3,56,28,0,194,195,5,11,0,0,195,196,5,5,0,0,196,27,1,0,0,0,
  	197,198,5,16,0,0,198,202,5,10,0,0,199,203,3,8,4,0,200,203,3,18,9,0,201,
  	203,5,5,0,0,202,199,1,0,0,0,202,200,1,0,0,0,202,201,1,0,0,0,203,205,1,
  	0,0,0,204,206,3,56,28,0,205,204,1,0,0,0,205,206,1,0,0,0,206,207,1,0,0,
  	0,207,209,5,5,0,0,208,210,3,56,28,0,209,208,1,0,0,0,209,210,1,0,0,0,210,
  	211,1,0,0,0,211,212,5,11,0,0,212,213,3,6,3,0,213,29,1,0,0,0,214,215,5,
  	17,0,0,215,216,5,10,0,0,216,217,5,61,0,0,217,218,5,18,0,0,218,219,3,56,
  	28,0,219,220,5,11,0,0,220,221,3,6,3,0,221,31,1,0,0,0,222,223,5,19,0,0,
  	223,224,5,5,0,0,224,33,1,0,0,0,225,226,5,20,0,0,226,227,5,5,0,0,227,35,
  	1,0,0,0,228,230,5,21,0,0,229,231,3,56,28,0,230,229,1,0,0,0,230,231,1,
  	0,0,0,231,232,1,0,0,0,232,233,5,5,0,0,233,37,1,0,0,0,234,235,5,22,0,0,
  	235,236,3,4,2,0,236,237,5,23,0,0,237,238,5,10,0,0,238,239,5,61,0,0,239,
  	240,5,11,0,0,240,241,3,4,2,0,241,39,1,0,0,0,242,243,5,24,0,0,243,244,
  	5,10,0,0,244,245,3,56,28,0,245,246,5,11,0,0,246,250,5,1,0,0,247,249,3,
  	42,21,0,248,247,1,0,0,0,249,252,1,0,0,0,250,248,1,0,0,0,250,251,1,0,0,
  	0,251,254,1,0,0,0,252,250,1,0,0,0,253,255,3,44,22,0,254,253,1,0,0,0,254,
  	255,1,0,0,0,255,256,1,0,0,0,256,257,5,2,0,0,257,41,1,0,0,0,258,259,5,
  	25,0,0,259,260,3,56,28,0,260,264,5,8,0,0,261,263,3,2,1,0,262,261,1,0,
  	0,0,263,266,1,0,0,0,264,262,1,0,0,0,264,265,1,0,0,0,265,43,1,0,0,0,266,
  	264,1,0,0,0,267,268,5,26,0,0,268,272,5,8,0,0,269,271,3,2,1,0,270,269,
  	1,0,0,0,271,274,1,0,0,0,272,270,1,0,0,0,272,273,1,0,0,0,273,45,1,0,0,
  	0,274,272,1,0,0,0,275,276,5,27,0,0,276,277,5,61,0,0,277,279,5,10,0,0,
  	278,280,3,48,24,0,279,278,1,0,0,0,279,280,1,0,0,0,280,281,1,0,0,0,281,
  	284,5,11,0,0,282,283,5,8,0,0,283,285,3,90,45,0,284,282,1,0,0,0,284,285,
  	1,0,0,0,285,286,1,0,0,0,286,287,3,4,2,0,287,47,1,0,0,0,288,293,3,50,25,
  	0,289,290,5,28,0,0,290,292,3,50,25,0,291,289,1,0,0,0,292,295,1,0,0,0,
  	293,291,1,0,0,0,293,294,1,0,0,0,294,49,1,0,0,0,295,293,1,0,0,0,296,299,
  	5,61,0,0,297,298,5,8,0,0,298,300,3,90,45,0,299,297,1,0,0,0,299,300,1,
  	0,0,0,300,51,1,0,0,0,301,302,5,29,0,0,302,305,5,61,0,0,303,304,5,8,0,
  	0,304,306,5,61,0,0,305,303,1,0,0,0,305,306,1,0,0,0,306,307,1,0,0,0,307,
  	311,5,1,0,0,308,310,3,54,27,0,309,308,1,0,0,0,310,313,1,0,0,0,311,309,
  	1,0,0,0,311,312,1,0,0,0,312,314,1,0,0,0,313,311,1,0,0,0,314,315,5,2,0,
  	0,315,53,1,0,0,0,316,320,3,46,23,0,317,320,3,8,4,0,318,320,3,10,5,0,319,
  	316,1,0,0,0,319,317,1,0,0,0,319,318,1,0,0,0,320,55,1,0,0,0,321,322,3,
  	58,29,0,322,57,1,0,0,0,323,324,3,80,40,0,324,325,5,7,0,0,325,326,3,58,
  	29,0,326,329,1,0,0,0,327,329,3,60,30,0,328,323,1,0,0,0,328,327,1,0,0,
  	0,329,59,1,0,0,0,330,336,3,62,31,0,331,332,5,30,0,0,332,333,3,56,28,0,
  	333,334,5,8,0,0,334,335,3,56,28,0,335,337,1,0,0,0,336,331,1,0,0,0,336,
  	337,1,0,0,0,337,61,1,0,0,0,338,343,3,64,32,0,339,340,5,31,0,0,340,342,
  	3,64,32,0,341,339,1,0,0,0,342,345,1,0,0,0,343,341,1,0,0,0,343,344,1,0,
  	0,0,344,63,1,0,0,0,345,343,1,0,0,0,346,351,3,66,33,0,347,348,5,32,0,0,
  	348,350,3,66,33,0,349,347,1,0,0,0,350,353,1,0,0,0,351,349,1,0,0,0,351,
  	352,1,0,0,0,352,65,1,0,0,0,353,351,1,0,0,0,354,359,3,68,34,0,355,356,
  	7,1,0,0,356,358,3,68,34,0,357,355,1,0,0,0,358,361,1,0,0,0,359,357,1,0,
  	0,0,359,360,1,0,0,0,360,67,1,0,0,0,361,359,1,0,0,0,362,367,3,70,35,0,
  	363,364,7,2,0,0,364,366,3,70,35,0,365,363,1,0,0,0,366,369,1,0,0,0,367,
  	365,1,0,0,0,367,368,1,0,0,0,368,69,1,0,0,0,369,367,1,0,0,0,370,375,3,
  	72,36,0,371,372,7,3,0,0,372,374,3,72,36,0,373,371,1,0,0,0,374,377,1,0,
  	0,0,375,373,1,0,0,0,375,376,1,0,0,0,376,71,1,0,0,0,377,375,1,0,0,0,378,
  	383,3,74,37,0,379,380,7,4,0,0,380,382,3,74,37,0,381,379,1,0,0,0,382,385,
  	1,0,0,0,383,381,1,0,0,0,383,384,1,0,0,0,384,73,1,0,0,0,385,383,1,0,0,
  	0,386,387,7,5,0,0,387,390,3,74,37,0,388,390,3,76,38,0,389,386,1,0,0,0,
  	389,388,1,0,0,0,390,75,1,0,0,0,391,398,3,78,39,0,392,398,3,80,40,0,393,
  	394,5,10,0,0,394,395,3,56,28,0,395,396,5,11,0,0,396,398,1,0,0,0,397,391,
  	1,0,0,0,397,392,1,0,0,0,397,393,1,0,0,0,398,77,1,0,0,0,399,407,5,59,0,
  	0,400,407,5,58,0,0,401,407,5,60,0,0,402,407,3,88,44,0,403,407,5,45,0,
  	0,404,407,5,46,0,0,405,407,5,47,0,0,406,399,1,0,0,0,406,400,1,0,0,0,406,
  	401,1,0,0,0,406,402,1,0,0,0,406,403,1,0,0,0,406,404,1,0,0,0,406,405,1,
  	0,0,0,407,79,1,0,0,0,408,412,3,82,41,0,409,411,3,84,42,0,410,409,1,0,
  	0,0,411,414,1,0,0,0,412,410,1,0,0,0,412,413,1,0,0,0,413,81,1,0,0,0,414,
  	412,1,0,0,0,415,425,5,61,0,0,416,417,5,48,0,0,417,418,5,61,0,0,418,420,
  	5,10,0,0,419,421,3,86,43,0,420,419,1,0,0,0,420,421,1,0,0,0,421,422,1,
  	0,0,0,422,425,5,11,0,0,423,425,5,49,0,0,424,415,1,0,0,0,424,416,1,0,0,
  	0,424,423,1,0,0,0,425,83,1,0,0,0,426,428,5,10,0,0,427,429,3,86,43,0,428,
  	427,1,0,0,0,428,429,1,0,0,0,429,430,1,0,0,0,430,438,5,11,0,0,431,432,
  	5,50,0,0,432,433,3,56,28,0,433,434,5,51,0,0,434,438,1,0,0,0,435,436,5,
  	52,0,0,436,438,5,61,0,0,437,426,1,0,0,0,437,431,1,0,0,0,437,435,1,0,0,
  	0,438,85,1,0,0,0,439,444,3,56,28,0,440,441,5,28,0,0,441,443,3,56,28,0,
  	442,440,1,0,0,0,443,446,1,0,0,0,444,442,1,0,0,0,444,445,1,0,0,0,445,87,
  	1,0,0,0,446,444,1,0,0,0,447,456,5,50,0,0,448,453,3,56,28,0,449,450,5,
  	28,0,0,450,452,3,56,28,0,451,449,1,0,0,0,452,455,1,0,0,0,453,451,1,0,
  	0,0,453,454,1,0,0,0,454,457,1,0,0,0,455,453,1,0,0,0,456,448,1,0,0,0,456,
  	457,1,0,0,0,457,458,1,0,0,0,458,459,5,51,0,0,459,89,1,0,0,0,460,465,3,
  	92,46,0,461,462,5,50,0,0,462,464,5,51,0,0,463,461,1,0,0,0,464,467,1,0,
  	0,0,465,463,1,0,0,0,465,466,1,0,0,0,466,91,1,0,0,0,467,465,1,0,0,0,468,
  	469,7,6,0,0,469,93,1,0,0,0,43,97,120,126,133,138,141,148,181,202,205,
  	209,230,250,254,264,272,279,284,293,299,305,311,319,328,336,343,351,359,
  	367,375,383,389,397,406,412,420,424,428,437,444,453,456,465
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  compiscriptParserStaticData = std::move(staticData);
}

}

CompiscriptParser::CompiscriptParser(TokenStream *input) : CompiscriptParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

CompiscriptParser::CompiscriptParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  CompiscriptParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *compiscriptParserStaticData->atn, compiscriptParserStaticData->decisionToDFA, compiscriptParserStaticData->sharedContextCache, options);
}

CompiscriptParser::~CompiscriptParser() {
  delete _interpreter;
}

const atn::ATN& CompiscriptParser::getATN() const {
  return *compiscriptParserStaticData->atn;
}

std::string CompiscriptParser::getGrammarFileName() const {
  return "Compiscript.g4";
}

const std::vector<std::string>& CompiscriptParser::getRuleNames() const {
  return compiscriptParserStaticData->ruleNames;
}

const dfa::Vocabulary& CompiscriptParser::getVocabulary() const {
  return compiscriptParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView CompiscriptParser::getSerializedATN() const {
  return compiscriptParserStaticData->serializedATN;
}


//----------------- ProgramContext ------------------------------------------------------------------

CompiscriptParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::ProgramContext::EOF() {
  return getToken(CompiscriptParser::EOF, 0);
}

std::vector<CompiscriptParser::StatementContext *> CompiscriptParser::ProgramContext::statement() {
  return getRuleContexts<CompiscriptParser::StatementContext>();
}

CompiscriptParser::StatementContext* CompiscriptParser::ProgramContext::statement(size_t i) {
  return getRuleContext<CompiscriptParser::StatementContext>(i);
}


size_t CompiscriptParser::ProgramContext::getRuleIndex() const {
  return CompiscriptParser::RuleProgram;
}


std::any CompiscriptParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ProgramContext* CompiscriptParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 0, CompiscriptParser::RuleProgram);
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
    setState(97);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690950110926426) != 0)) {
      setState(94);
      statement();
      setState(99);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(100);
    match(CompiscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

CompiscriptParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::VariableDeclarationContext* CompiscriptParser::StatementContext::variableDeclaration() {
  return getRuleContext<CompiscriptParser::VariableDeclarationContext>(0);
}

CompiscriptParser::ConstantDeclarationContext* CompiscriptParser::StatementContext::constantDeclaration() {
  return getRuleContext<CompiscriptParser::ConstantDeclarationContext>(0);
}

CompiscriptParser::AssignmentStatementContext* CompiscriptParser::StatementContext::assignmentStatement() {
  return getRuleContext<CompiscriptParser::AssignmentStatementContext>(0);
}

CompiscriptParser::FunctionDeclarationContext* CompiscriptParser::StatementContext::functionDeclaration() {
  return getRuleContext<CompiscriptParser::FunctionDeclarationContext>(0);
}

CompiscriptParser::ClassDeclarationContext* CompiscriptParser::StatementContext::classDeclaration() {
  return getRuleContext<CompiscriptParser::ClassDeclarationContext>(0);
}

CompiscriptParser::ExpressionStatementContext* CompiscriptParser::StatementContext::expressionStatement() {
  return getRuleContext<CompiscriptParser::ExpressionStatementContext>(0);
}

CompiscriptParser::PrintStatementContext* CompiscriptParser::StatementContext::printStatement() {
  return getRuleContext<CompiscriptParser::PrintStatementContext>(0);
}

CompiscriptParser::BlockContext* CompiscriptParser::StatementContext::block() {
  return getRuleContext<CompiscriptParser::BlockContext>(0);
}

CompiscriptParser::IfStatementContext* CompiscriptParser::StatementContext::ifStatement() {
  return getRuleContext<CompiscriptParser::IfStatementContext>(0);
}

CompiscriptParser::WhileStatementContext* CompiscriptParser::StatementContext::whileStatement() {
  return getRuleContext<CompiscriptParser::WhileStatementContext>(0);
}

CompiscriptParser::DoWhileStatementContext* CompiscriptParser::StatementContext::doWhileStatement() {
  return getRuleContext<CompiscriptParser::DoWhileStatementContext>(0);
}

CompiscriptParser::ForStatementContext* CompiscriptParser::StatementContext::forStatement() {
  return getRuleContext<CompiscriptParser::ForStatementContext>(0);
}

CompiscriptParser::ForeachStatementContext* CompiscriptParser::StatementContext::foreachStatement() {
  return getRuleContext<CompiscriptParser::ForeachStatementContext>(0);
}

CompiscriptParser::TryCatchStatementContext* CompiscriptParser::StatementContext::tryCatchStatement() {
  return getRuleContext<CompiscriptParser::TryCatchStatementContext>(0);
}

CompiscriptParser::SwitchStatementContext* CompiscriptParser::StatementContext::switchStatement() {
  return getRuleContext<CompiscriptParser::SwitchStatementContext>(0);
}

CompiscriptParser::BreakStatementContext* CompiscriptParser::StatementContext::breakStatement() {
  return getRuleContext<CompiscriptParser::BreakStatementContext>(0);
}

CompiscriptParser::ContinueStatementContext* CompiscriptParser::StatementContext::continueStatement() {
  return getRuleContext<CompiscriptParser::ContinueStatementContext>(0);
}

CompiscriptParser::ReturnStatementContext* CompiscriptParser::StatementContext::returnStatement() {
  return getRuleContext<CompiscriptParser::ReturnStatementContext>(0);
}


size_t CompiscriptParser::StatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleStatement;
}


std::any CompiscriptParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::StatementContext* CompiscriptParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 2, CompiscriptParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(120);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(102);
      variableDeclaration();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(103);
      constantDeclaration();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(104);
      assignmentStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(105);
      functionDeclaration();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(106);
      classDeclaration();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(107);
      expressionStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(108);
      printStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(109);
      block();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(110);
      ifStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(111);
      whileStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(112);
      doWhileStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(113);
      forStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(114);
      foreachStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(115);
      tryCatchStatement();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(116);
      switchStatement();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(117);
      breakStatement();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(118);
      continueStatement();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(119);
      returnStatement();
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

//----------------- BlockContext ------------------------------------------------------------------

CompiscriptParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::StatementContext *> CompiscriptParser::BlockContext::statement() {
  return getRuleContexts<CompiscriptParser::StatementContext>();
}

CompiscriptParser::StatementContext* CompiscriptParser::BlockContext::statement(size_t i) {
  return getRuleContext<CompiscriptParser::StatementContext>(i);
}


size_t CompiscriptParser::BlockContext::getRuleIndex() const {
  return CompiscriptParser::RuleBlock;
}


std::any CompiscriptParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::BlockContext* CompiscriptParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 4, CompiscriptParser::RuleBlock);
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
    setState(122);
    match(CompiscriptParser::T__0);
    setState(126);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690950110926426) != 0)) {
      setState(123);
      statement();
      setState(128);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(129);
    match(CompiscriptParser::T__1);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ControlBodyContext ------------------------------------------------------------------

CompiscriptParser::ControlBodyContext::ControlBodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::BlockContext* CompiscriptParser::ControlBodyContext::block() {
  return getRuleContext<CompiscriptParser::BlockContext>(0);
}

CompiscriptParser::StatementContext* CompiscriptParser::ControlBodyContext::statement() {
  return getRuleContext<CompiscriptParser::StatementContext>(0);
}


size_t CompiscriptParser::ControlBodyContext::getRuleIndex() const {
  return CompiscriptParser::RuleControlBody;
}


std::any CompiscriptParser::ControlBodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitControlBody(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::controlBody() {
  ControlBodyContext *_localctx = _tracker.createInstance<ControlBodyContext>(_ctx, getState());
  enterRule(_localctx, 6, CompiscriptParser::RuleControlBody);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(133);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(131);
      block();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(132);
      statement();
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

//----------------- VariableDeclarationContext ------------------------------------------------------------------

CompiscriptParser::VariableDeclarationContext::VariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::VariableDeclarationContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::TypeAnnotationContext* CompiscriptParser::VariableDeclarationContext::typeAnnotation() {
  return getRuleContext<CompiscriptParser::TypeAnnotationContext>(0);
}

CompiscriptParser::InitializerContext* CompiscriptParser::VariableDeclarationContext::initializer() {
  return getRuleContext<CompiscriptParser::InitializerContext>(0);
}


size_t CompiscriptParser::VariableDeclarationContext::getRuleIndex() const {
  return CompiscriptParser::RuleVariableDeclaration;
}


std::any CompiscriptParser::VariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::VariableDeclarationContext* CompiscriptParser::variableDeclaration() {
  VariableDeclarationContext *_localctx = _tracker.createInstance<VariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 8, CompiscriptParser::RuleVariableDeclaration);
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
    setState(135);
    _la = _input->LA(1);
    if (!(_la == CompiscriptParser::T__2

    || _la == CompiscriptParser::T__3)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(136);
    match(CompiscriptParser::Identifier);
    setState(138);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__7) {
      setState(137);
      typeAnnotation();
    }
    setState(141);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__6) {
      setState(140);
      initializer();
    }
    setState(143);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstantDeclarationContext ------------------------------------------------------------------

CompiscriptParser::ConstantDeclarationContext::ConstantDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::ConstantDeclarationContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ConstantDeclarationContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

CompiscriptParser::TypeAnnotationContext* CompiscriptParser::ConstantDeclarationContext::typeAnnotation() {
  return getRuleContext<CompiscriptParser::TypeAnnotationContext>(0);
}


size_t CompiscriptParser::ConstantDeclarationContext::getRuleIndex() const {
  return CompiscriptParser::RuleConstantDeclaration;
}


std::any CompiscriptParser::ConstantDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitConstantDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ConstantDeclarationContext* CompiscriptParser::constantDeclaration() {
  ConstantDeclarationContext *_localctx = _tracker.createInstance<ConstantDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 10, CompiscriptParser::RuleConstantDeclaration);
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
    setState(145);
    match(CompiscriptParser::T__5);
    setState(146);
    match(CompiscriptParser::Identifier);
    setState(148);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__7) {
      setState(147);
      typeAnnotation();
    }
    setState(150);
    match(CompiscriptParser::T__6);
    setState(151);
    expression();
    setState(152);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeAnnotationContext ------------------------------------------------------------------

CompiscriptParser::TypeAnnotationContext::TypeAnnotationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::TypeContext* CompiscriptParser::TypeAnnotationContext::type() {
  return getRuleContext<CompiscriptParser::TypeContext>(0);
}


size_t CompiscriptParser::TypeAnnotationContext::getRuleIndex() const {
  return CompiscriptParser::RuleTypeAnnotation;
}


std::any CompiscriptParser::TypeAnnotationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitTypeAnnotation(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::TypeAnnotationContext* CompiscriptParser::typeAnnotation() {
  TypeAnnotationContext *_localctx = _tracker.createInstance<TypeAnnotationContext>(_ctx, getState());
  enterRule(_localctx, 12, CompiscriptParser::RuleTypeAnnotation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(154);
    match(CompiscriptParser::T__7);
    setState(155);
    type();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitializerContext ------------------------------------------------------------------

CompiscriptParser::InitializerContext::InitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::InitializerContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::InitializerContext::getRuleIndex() const {
  return CompiscriptParser::RuleInitializer;
}


std::any CompiscriptParser::InitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitInitializer(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::InitializerContext* CompiscriptParser::initializer() {
  InitializerContext *_localctx = _tracker.createInstance<InitializerContext>(_ctx, getState());
  enterRule(_localctx, 14, CompiscriptParser::RuleInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(157);
    match(CompiscriptParser::T__6);
    setState(158);
    expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentStatementContext ------------------------------------------------------------------

CompiscriptParser::AssignmentStatementContext::AssignmentStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::LeftHandSideContext* CompiscriptParser::AssignmentStatementContext::leftHandSide() {
  return getRuleContext<CompiscriptParser::LeftHandSideContext>(0);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::AssignmentStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::AssignmentStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleAssignmentStatement;
}


std::any CompiscriptParser::AssignmentStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitAssignmentStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::AssignmentStatementContext* CompiscriptParser::assignmentStatement() {
  AssignmentStatementContext *_localctx = _tracker.createInstance<AssignmentStatementContext>(_ctx, getState());
  enterRule(_localctx, 16, CompiscriptParser::RuleAssignmentStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(160);
    leftHandSide();
    setState(161);
    match(CompiscriptParser::T__6);
    setState(162);
    expression();
    setState(163);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionStatementContext ------------------------------------------------------------------

CompiscriptParser::ExpressionStatementContext::ExpressionStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ExpressionStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::ExpressionStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleExpressionStatement;
}


std::any CompiscriptParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ExpressionStatementContext* CompiscriptParser::expressionStatement() {
  ExpressionStatementContext *_localctx = _tracker.createInstance<ExpressionStatementContext>(_ctx, getState());
  enterRule(_localctx, 18, CompiscriptParser::RuleExpressionStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(165);
    expression();
    setState(166);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrintStatementContext ------------------------------------------------------------------

CompiscriptParser::PrintStatementContext::PrintStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::PrintStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::PrintStatementContext::getRuleIndex() const {
  return CompiscriptParser::RulePrintStatement;
}


std::any CompiscriptParser::PrintStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitPrintStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::PrintStatementContext* CompiscriptParser::printStatement() {
  PrintStatementContext *_localctx = _tracker.createInstance<PrintStatementContext>(_ctx, getState());
  enterRule(_localctx, 20, CompiscriptParser::RulePrintStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(168);
    match(CompiscriptParser::T__8);
    setState(169);
    match(CompiscriptParser::T__9);
    setState(170);
    expression();
    setState(171);
    match(CompiscriptParser::T__10);
    setState(172);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IfStatementContext ------------------------------------------------------------------

CompiscriptParser::IfStatementContext::IfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::IfStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

std::vector<CompiscriptParser::ControlBodyContext *> CompiscriptParser::IfStatementContext::controlBody() {
  return getRuleContexts<CompiscriptParser::ControlBodyContext>();
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::IfStatementContext::controlBody(size_t i) {
  return getRuleContext<CompiscriptParser::ControlBodyContext>(i);
}


size_t CompiscriptParser::IfStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleIfStatement;
}


std::any CompiscriptParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::IfStatementContext* CompiscriptParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 22, CompiscriptParser::RuleIfStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(174);
    match(CompiscriptParser::T__11);
    setState(175);
    match(CompiscriptParser::T__9);
    setState(176);
    expression();
    setState(177);
    match(CompiscriptParser::T__10);
    setState(178);
    controlBody();
    setState(181);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 7, _ctx)) {
    case 1: {
      setState(179);
      match(CompiscriptParser::T__12);
      setState(180);
      controlBody();
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

//----------------- WhileStatementContext ------------------------------------------------------------------

CompiscriptParser::WhileStatementContext::WhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::WhileStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::WhileStatementContext::controlBody() {
  return getRuleContext<CompiscriptParser::ControlBodyContext>(0);
}


size_t CompiscriptParser::WhileStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleWhileStatement;
}


std::any CompiscriptParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::WhileStatementContext* CompiscriptParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 24, CompiscriptParser::RuleWhileStatement);

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
    match(CompiscriptParser::T__13);
    setState(184);
    match(CompiscriptParser::T__9);
    setState(185);
    expression();
    setState(186);
    match(CompiscriptParser::T__10);
    setState(187);
    controlBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DoWhileStatementContext ------------------------------------------------------------------

CompiscriptParser::DoWhileStatementContext::DoWhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::DoWhileStatementContext::controlBody() {
  return getRuleContext<CompiscriptParser::ControlBodyContext>(0);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::DoWhileStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::DoWhileStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleDoWhileStatement;
}


std::any CompiscriptParser::DoWhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitDoWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::DoWhileStatementContext* CompiscriptParser::doWhileStatement() {
  DoWhileStatementContext *_localctx = _tracker.createInstance<DoWhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 26, CompiscriptParser::RuleDoWhileStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(189);
    match(CompiscriptParser::T__14);
    setState(190);
    controlBody();
    setState(191);
    match(CompiscriptParser::T__13);
    setState(192);
    match(CompiscriptParser::T__9);
    setState(193);
    expression();
    setState(194);
    match(CompiscriptParser::T__10);
    setState(195);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForStatementContext ------------------------------------------------------------------

CompiscriptParser::ForStatementContext::ForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::ForStatementContext::controlBody() {
  return getRuleContext<CompiscriptParser::ControlBodyContext>(0);
}

CompiscriptParser::VariableDeclarationContext* CompiscriptParser::ForStatementContext::variableDeclaration() {
  return getRuleContext<CompiscriptParser::VariableDeclarationContext>(0);
}

CompiscriptParser::ExpressionStatementContext* CompiscriptParser::ForStatementContext::expressionStatement() {
  return getRuleContext<CompiscriptParser::ExpressionStatementContext>(0);
}

std::vector<CompiscriptParser::ExpressionContext *> CompiscriptParser::ForStatementContext::expression() {
  return getRuleContexts<CompiscriptParser::ExpressionContext>();
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ForStatementContext::expression(size_t i) {
  return getRuleContext<CompiscriptParser::ExpressionContext>(i);
}


size_t CompiscriptParser::ForStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleForStatement;
}


std::any CompiscriptParser::ForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitForStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ForStatementContext* CompiscriptParser::forStatement() {
  ForStatementContext *_localctx = _tracker.createInstance<ForStatementContext>(_ctx, getState());
  enterRule(_localctx, 28, CompiscriptParser::RuleForStatement);
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
    setState(197);
    match(CompiscriptParser::T__15);
    setState(198);
    match(CompiscriptParser::T__9);
    setState(202);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::T__2:
      case CompiscriptParser::T__3: {
        setState(199);
        variableDeclaration();
        break;
      }

      case CompiscriptParser::T__9:
      case CompiscriptParser::T__39:
      case CompiscriptParser::T__43:
      case CompiscriptParser::T__44:
      case CompiscriptParser::T__45:
      case CompiscriptParser::T__46:
      case CompiscriptParser::T__47:
      case CompiscriptParser::T__48:
      case CompiscriptParser::T__49:
      case CompiscriptParser::FloatLiteral:
      case CompiscriptParser::IntegerLiteral:
      case CompiscriptParser::StringLiteral:
      case CompiscriptParser::Identifier: {
        setState(200);
        expressionStatement();
        break;
      }

      case CompiscriptParser::T__4: {
        setState(201);
        match(CompiscriptParser::T__4);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(205);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690949414945792) != 0)) {
      setState(204);
      expression();
    }
    setState(207);
    match(CompiscriptParser::T__4);
    setState(209);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690949414945792) != 0)) {
      setState(208);
      expression();
    }
    setState(211);
    match(CompiscriptParser::T__10);
    setState(212);
    controlBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForeachStatementContext ------------------------------------------------------------------

CompiscriptParser::ForeachStatementContext::ForeachStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::ForeachStatementContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ForeachStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

CompiscriptParser::ControlBodyContext* CompiscriptParser::ForeachStatementContext::controlBody() {
  return getRuleContext<CompiscriptParser::ControlBodyContext>(0);
}


size_t CompiscriptParser::ForeachStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleForeachStatement;
}


std::any CompiscriptParser::ForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ForeachStatementContext* CompiscriptParser::foreachStatement() {
  ForeachStatementContext *_localctx = _tracker.createInstance<ForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 30, CompiscriptParser::RuleForeachStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(214);
    match(CompiscriptParser::T__16);
    setState(215);
    match(CompiscriptParser::T__9);
    setState(216);
    match(CompiscriptParser::Identifier);
    setState(217);
    match(CompiscriptParser::T__17);
    setState(218);
    expression();
    setState(219);
    match(CompiscriptParser::T__10);
    setState(220);
    controlBody();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BreakStatementContext ------------------------------------------------------------------

CompiscriptParser::BreakStatementContext::BreakStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t CompiscriptParser::BreakStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleBreakStatement;
}


std::any CompiscriptParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::BreakStatementContext* CompiscriptParser::breakStatement() {
  BreakStatementContext *_localctx = _tracker.createInstance<BreakStatementContext>(_ctx, getState());
  enterRule(_localctx, 32, CompiscriptParser::RuleBreakStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(222);
    match(CompiscriptParser::T__18);
    setState(223);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContinueStatementContext ------------------------------------------------------------------

CompiscriptParser::ContinueStatementContext::ContinueStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t CompiscriptParser::ContinueStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleContinueStatement;
}


std::any CompiscriptParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ContinueStatementContext* CompiscriptParser::continueStatement() {
  ContinueStatementContext *_localctx = _tracker.createInstance<ContinueStatementContext>(_ctx, getState());
  enterRule(_localctx, 34, CompiscriptParser::RuleContinueStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(225);
    match(CompiscriptParser::T__19);
    setState(226);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnStatementContext ------------------------------------------------------------------

CompiscriptParser::ReturnStatementContext::ReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ReturnStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::ReturnStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleReturnStatement;
}


std::any CompiscriptParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ReturnStatementContext* CompiscriptParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 36, CompiscriptParser::RuleReturnStatement);
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
    setState(228);
    match(CompiscriptParser::T__20);
    setState(230);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690949414945792) != 0)) {
      setState(229);
      expression();
    }
    setState(232);
    match(CompiscriptParser::T__4);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TryCatchStatementContext ------------------------------------------------------------------

CompiscriptParser::TryCatchStatementContext::TryCatchStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::BlockContext *> CompiscriptParser::TryCatchStatementContext::block() {
  return getRuleContexts<CompiscriptParser::BlockContext>();
}

CompiscriptParser::BlockContext* CompiscriptParser::TryCatchStatementContext::block(size_t i) {
  return getRuleContext<CompiscriptParser::BlockContext>(i);
}

tree::TerminalNode* CompiscriptParser::TryCatchStatementContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}


size_t CompiscriptParser::TryCatchStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleTryCatchStatement;
}


std::any CompiscriptParser::TryCatchStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitTryCatchStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::TryCatchStatementContext* CompiscriptParser::tryCatchStatement() {
  TryCatchStatementContext *_localctx = _tracker.createInstance<TryCatchStatementContext>(_ctx, getState());
  enterRule(_localctx, 38, CompiscriptParser::RuleTryCatchStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(234);
    match(CompiscriptParser::T__21);
    setState(235);
    block();
    setState(236);
    match(CompiscriptParser::T__22);
    setState(237);
    match(CompiscriptParser::T__9);
    setState(238);
    match(CompiscriptParser::Identifier);
    setState(239);
    match(CompiscriptParser::T__10);
    setState(240);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwitchStatementContext ------------------------------------------------------------------

CompiscriptParser::SwitchStatementContext::SwitchStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::SwitchStatementContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

std::vector<CompiscriptParser::SwitchCaseContext *> CompiscriptParser::SwitchStatementContext::switchCase() {
  return getRuleContexts<CompiscriptParser::SwitchCaseContext>();
}

CompiscriptParser::SwitchCaseContext* CompiscriptParser::SwitchStatementContext::switchCase(size_t i) {
  return getRuleContext<CompiscriptParser::SwitchCaseContext>(i);
}

CompiscriptParser::DefaultCaseContext* CompiscriptParser::SwitchStatementContext::defaultCase() {
  return getRuleContext<CompiscriptParser::DefaultCaseContext>(0);
}


size_t CompiscriptParser::SwitchStatementContext::getRuleIndex() const {
  return CompiscriptParser::RuleSwitchStatement;
}


std::any CompiscriptParser::SwitchStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitSwitchStatement(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::SwitchStatementContext* CompiscriptParser::switchStatement() {
  SwitchStatementContext *_localctx = _tracker.createInstance<SwitchStatementContext>(_ctx, getState());
  enterRule(_localctx, 40, CompiscriptParser::RuleSwitchStatement);
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
    setState(242);
    match(CompiscriptParser::T__23);
    setState(243);
    match(CompiscriptParser::T__9);
    setState(244);
    expression();
    setState(245);
    match(CompiscriptParser::T__10);
    setState(246);
    match(CompiscriptParser::T__0);
    setState(250);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__24) {
      setState(247);
      switchCase();
      setState(252);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(254);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__25) {
      setState(253);
      defaultCase();
    }
    setState(256);
    match(CompiscriptParser::T__1);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwitchCaseContext ------------------------------------------------------------------

CompiscriptParser::SwitchCaseContext::SwitchCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::ExpressionContext* CompiscriptParser::SwitchCaseContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

std::vector<CompiscriptParser::StatementContext *> CompiscriptParser::SwitchCaseContext::statement() {
  return getRuleContexts<CompiscriptParser::StatementContext>();
}

CompiscriptParser::StatementContext* CompiscriptParser::SwitchCaseContext::statement(size_t i) {
  return getRuleContext<CompiscriptParser::StatementContext>(i);
}


size_t CompiscriptParser::SwitchCaseContext::getRuleIndex() const {
  return CompiscriptParser::RuleSwitchCase;
}


std::any CompiscriptParser::SwitchCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitSwitchCase(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::SwitchCaseContext* CompiscriptParser::switchCase() {
  SwitchCaseContext *_localctx = _tracker.createInstance<SwitchCaseContext>(_ctx, getState());
  enterRule(_localctx, 42, CompiscriptParser::RuleSwitchCase);
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
    setState(258);
    match(CompiscriptParser::T__24);
    setState(259);
    expression();
    setState(260);
    match(CompiscriptParser::T__7);
    setState(264);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690950110926426) != 0)) {
      setState(261);
      statement();
      setState(266);
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

//----------------- DefaultCaseContext ------------------------------------------------------------------

CompiscriptParser::DefaultCaseContext::DefaultCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::StatementContext *> CompiscriptParser::DefaultCaseContext::statement() {
  return getRuleContexts<CompiscriptParser::StatementContext>();
}

CompiscriptParser::StatementContext* CompiscriptParser::DefaultCaseContext::statement(size_t i) {
  return getRuleContext<CompiscriptParser::StatementContext>(i);
}


size_t CompiscriptParser::DefaultCaseContext::getRuleIndex() const {
  return CompiscriptParser::RuleDefaultCase;
}


std::any CompiscriptParser::DefaultCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitDefaultCase(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::DefaultCaseContext* CompiscriptParser::defaultCase() {
  DefaultCaseContext *_localctx = _tracker.createInstance<DefaultCaseContext>(_ctx, getState());
  enterRule(_localctx, 44, CompiscriptParser::RuleDefaultCase);
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
    setState(267);
    match(CompiscriptParser::T__25);
    setState(268);
    match(CompiscriptParser::T__7);
    setState(272);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690950110926426) != 0)) {
      setState(269);
      statement();
      setState(274);
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

//----------------- FunctionDeclarationContext ------------------------------------------------------------------

CompiscriptParser::FunctionDeclarationContext::FunctionDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::FunctionDeclarationContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::BlockContext* CompiscriptParser::FunctionDeclarationContext::block() {
  return getRuleContext<CompiscriptParser::BlockContext>(0);
}

CompiscriptParser::ParametersContext* CompiscriptParser::FunctionDeclarationContext::parameters() {
  return getRuleContext<CompiscriptParser::ParametersContext>(0);
}

CompiscriptParser::TypeContext* CompiscriptParser::FunctionDeclarationContext::type() {
  return getRuleContext<CompiscriptParser::TypeContext>(0);
}


size_t CompiscriptParser::FunctionDeclarationContext::getRuleIndex() const {
  return CompiscriptParser::RuleFunctionDeclaration;
}


std::any CompiscriptParser::FunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::FunctionDeclarationContext* CompiscriptParser::functionDeclaration() {
  FunctionDeclarationContext *_localctx = _tracker.createInstance<FunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 46, CompiscriptParser::RuleFunctionDeclaration);
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
    setState(275);
    match(CompiscriptParser::T__26);
    setState(276);
    match(CompiscriptParser::Identifier);
    setState(277);
    match(CompiscriptParser::T__9);
    setState(279);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::Identifier) {
      setState(278);
      parameters();
    }
    setState(281);
    match(CompiscriptParser::T__10);
    setState(284);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__7) {
      setState(282);
      match(CompiscriptParser::T__7);
      setState(283);
      type();
    }
    setState(286);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParametersContext ------------------------------------------------------------------

CompiscriptParser::ParametersContext::ParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::ParameterContext *> CompiscriptParser::ParametersContext::parameter() {
  return getRuleContexts<CompiscriptParser::ParameterContext>();
}

CompiscriptParser::ParameterContext* CompiscriptParser::ParametersContext::parameter(size_t i) {
  return getRuleContext<CompiscriptParser::ParameterContext>(i);
}


size_t CompiscriptParser::ParametersContext::getRuleIndex() const {
  return CompiscriptParser::RuleParameters;
}


std::any CompiscriptParser::ParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitParameters(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ParametersContext* CompiscriptParser::parameters() {
  ParametersContext *_localctx = _tracker.createInstance<ParametersContext>(_ctx, getState());
  enterRule(_localctx, 48, CompiscriptParser::RuleParameters);
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
    setState(288);
    parameter();
    setState(293);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__27) {
      setState(289);
      match(CompiscriptParser::T__27);
      setState(290);
      parameter();
      setState(295);
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

//----------------- ParameterContext ------------------------------------------------------------------

CompiscriptParser::ParameterContext::ParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::ParameterContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::TypeContext* CompiscriptParser::ParameterContext::type() {
  return getRuleContext<CompiscriptParser::TypeContext>(0);
}


size_t CompiscriptParser::ParameterContext::getRuleIndex() const {
  return CompiscriptParser::RuleParameter;
}


std::any CompiscriptParser::ParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitParameter(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ParameterContext* CompiscriptParser::parameter() {
  ParameterContext *_localctx = _tracker.createInstance<ParameterContext>(_ctx, getState());
  enterRule(_localctx, 50, CompiscriptParser::RuleParameter);
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
    setState(296);
    match(CompiscriptParser::Identifier);
    setState(299);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__7) {
      setState(297);
      match(CompiscriptParser::T__7);
      setState(298);
      type();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassDeclarationContext ------------------------------------------------------------------

CompiscriptParser::ClassDeclarationContext::ClassDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> CompiscriptParser::ClassDeclarationContext::Identifier() {
  return getTokens(CompiscriptParser::Identifier);
}

tree::TerminalNode* CompiscriptParser::ClassDeclarationContext::Identifier(size_t i) {
  return getToken(CompiscriptParser::Identifier, i);
}

std::vector<CompiscriptParser::ClassMemberContext *> CompiscriptParser::ClassDeclarationContext::classMember() {
  return getRuleContexts<CompiscriptParser::ClassMemberContext>();
}

CompiscriptParser::ClassMemberContext* CompiscriptParser::ClassDeclarationContext::classMember(size_t i) {
  return getRuleContext<CompiscriptParser::ClassMemberContext>(i);
}


size_t CompiscriptParser::ClassDeclarationContext::getRuleIndex() const {
  return CompiscriptParser::RuleClassDeclaration;
}


std::any CompiscriptParser::ClassDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitClassDeclaration(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ClassDeclarationContext* CompiscriptParser::classDeclaration() {
  ClassDeclarationContext *_localctx = _tracker.createInstance<ClassDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 52, CompiscriptParser::RuleClassDeclaration);
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
    setState(301);
    match(CompiscriptParser::T__28);
    setState(302);
    match(CompiscriptParser::Identifier);
    setState(305);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__7) {
      setState(303);
      match(CompiscriptParser::T__7);
      setState(304);
      match(CompiscriptParser::Identifier);
    }
    setState(307);
    match(CompiscriptParser::T__0);
    setState(311);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 134217816) != 0)) {
      setState(308);
      classMember();
      setState(313);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(314);
    match(CompiscriptParser::T__1);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassMemberContext ------------------------------------------------------------------

CompiscriptParser::ClassMemberContext::ClassMemberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::FunctionDeclarationContext* CompiscriptParser::ClassMemberContext::functionDeclaration() {
  return getRuleContext<CompiscriptParser::FunctionDeclarationContext>(0);
}

CompiscriptParser::VariableDeclarationContext* CompiscriptParser::ClassMemberContext::variableDeclaration() {
  return getRuleContext<CompiscriptParser::VariableDeclarationContext>(0);
}

CompiscriptParser::ConstantDeclarationContext* CompiscriptParser::ClassMemberContext::constantDeclaration() {
  return getRuleContext<CompiscriptParser::ConstantDeclarationContext>(0);
}


size_t CompiscriptParser::ClassMemberContext::getRuleIndex() const {
  return CompiscriptParser::RuleClassMember;
}


std::any CompiscriptParser::ClassMemberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitClassMember(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ClassMemberContext* CompiscriptParser::classMember() {
  ClassMemberContext *_localctx = _tracker.createInstance<ClassMemberContext>(_ctx, getState());
  enterRule(_localctx, 54, CompiscriptParser::RuleClassMember);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(319);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::T__26: {
        enterOuterAlt(_localctx, 1);
        setState(316);
        functionDeclaration();
        break;
      }

      case CompiscriptParser::T__2:
      case CompiscriptParser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(317);
        variableDeclaration();
        break;
      }

      case CompiscriptParser::T__5: {
        enterOuterAlt(_localctx, 3);
        setState(318);
        constantDeclaration();
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

//----------------- ExpressionContext ------------------------------------------------------------------

CompiscriptParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::AssignmentExprContext* CompiscriptParser::ExpressionContext::assignmentExpr() {
  return getRuleContext<CompiscriptParser::AssignmentExprContext>(0);
}


size_t CompiscriptParser::ExpressionContext::getRuleIndex() const {
  return CompiscriptParser::RuleExpression;
}


std::any CompiscriptParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::expression() {
  ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 56, CompiscriptParser::RuleExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(321);
    assignmentExpr();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssignmentExprContext ------------------------------------------------------------------

CompiscriptParser::AssignmentExprContext::AssignmentExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t CompiscriptParser::AssignmentExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleAssignmentExpr;
}

void CompiscriptParser::AssignmentExprContext::copyFrom(AssignmentExprContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ExprNoAssignContext ------------------------------------------------------------------

CompiscriptParser::ConditionalExprContext* CompiscriptParser::ExprNoAssignContext::conditionalExpr() {
  return getRuleContext<CompiscriptParser::ConditionalExprContext>(0);
}

CompiscriptParser::ExprNoAssignContext::ExprNoAssignContext(AssignmentExprContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::ExprNoAssignContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitExprNoAssign(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignExprContext ------------------------------------------------------------------

CompiscriptParser::LeftHandSideContext* CompiscriptParser::AssignExprContext::leftHandSide() {
  return getRuleContext<CompiscriptParser::LeftHandSideContext>(0);
}

CompiscriptParser::AssignmentExprContext* CompiscriptParser::AssignExprContext::assignmentExpr() {
  return getRuleContext<CompiscriptParser::AssignmentExprContext>(0);
}

CompiscriptParser::AssignExprContext::AssignExprContext(AssignmentExprContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::AssignExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitAssignExpr(this);
  else
    return visitor->visitChildren(this);
}
CompiscriptParser::AssignmentExprContext* CompiscriptParser::assignmentExpr() {
  AssignmentExprContext *_localctx = _tracker.createInstance<AssignmentExprContext>(_ctx, getState());
  enterRule(_localctx, 58, CompiscriptParser::RuleAssignmentExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(328);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<CompiscriptParser::AssignExprContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(323);
      leftHandSide();
      setState(324);
      match(CompiscriptParser::T__6);
      setState(325);
      assignmentExpr();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<CompiscriptParser::ExprNoAssignContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(327);
      conditionalExpr();
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

//----------------- ConditionalExprContext ------------------------------------------------------------------

CompiscriptParser::ConditionalExprContext::ConditionalExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::LogicalOrExprContext* CompiscriptParser::ConditionalExprContext::logicalOrExpr() {
  return getRuleContext<CompiscriptParser::LogicalOrExprContext>(0);
}

std::vector<CompiscriptParser::ExpressionContext *> CompiscriptParser::ConditionalExprContext::expression() {
  return getRuleContexts<CompiscriptParser::ExpressionContext>();
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ConditionalExprContext::expression(size_t i) {
  return getRuleContext<CompiscriptParser::ExpressionContext>(i);
}


size_t CompiscriptParser::ConditionalExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleConditionalExpr;
}


std::any CompiscriptParser::ConditionalExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitConditionalExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ConditionalExprContext* CompiscriptParser::conditionalExpr() {
  ConditionalExprContext *_localctx = _tracker.createInstance<ConditionalExprContext>(_ctx, getState());
  enterRule(_localctx, 60, CompiscriptParser::RuleConditionalExpr);
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
    setState(330);
    logicalOrExpr();
    setState(336);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == CompiscriptParser::T__29) {
      setState(331);
      match(CompiscriptParser::T__29);
      setState(332);
      expression();
      setState(333);
      match(CompiscriptParser::T__7);
      setState(334);
      expression();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LogicalOrExprContext ------------------------------------------------------------------

CompiscriptParser::LogicalOrExprContext::LogicalOrExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::LogicalAndExprContext *> CompiscriptParser::LogicalOrExprContext::logicalAndExpr() {
  return getRuleContexts<CompiscriptParser::LogicalAndExprContext>();
}

CompiscriptParser::LogicalAndExprContext* CompiscriptParser::LogicalOrExprContext::logicalAndExpr(size_t i) {
  return getRuleContext<CompiscriptParser::LogicalAndExprContext>(i);
}


size_t CompiscriptParser::LogicalOrExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleLogicalOrExpr;
}


std::any CompiscriptParser::LogicalOrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::LogicalOrExprContext* CompiscriptParser::logicalOrExpr() {
  LogicalOrExprContext *_localctx = _tracker.createInstance<LogicalOrExprContext>(_ctx, getState());
  enterRule(_localctx, 62, CompiscriptParser::RuleLogicalOrExpr);
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
    setState(338);
    logicalAndExpr();
    setState(343);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__30) {
      setState(339);
      match(CompiscriptParser::T__30);
      setState(340);
      logicalAndExpr();
      setState(345);
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

//----------------- LogicalAndExprContext ------------------------------------------------------------------

CompiscriptParser::LogicalAndExprContext::LogicalAndExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::EqualityExprContext *> CompiscriptParser::LogicalAndExprContext::equalityExpr() {
  return getRuleContexts<CompiscriptParser::EqualityExprContext>();
}

CompiscriptParser::EqualityExprContext* CompiscriptParser::LogicalAndExprContext::equalityExpr(size_t i) {
  return getRuleContext<CompiscriptParser::EqualityExprContext>(i);
}


size_t CompiscriptParser::LogicalAndExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleLogicalAndExpr;
}


std::any CompiscriptParser::LogicalAndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::LogicalAndExprContext* CompiscriptParser::logicalAndExpr() {
  LogicalAndExprContext *_localctx = _tracker.createInstance<LogicalAndExprContext>(_ctx, getState());
  enterRule(_localctx, 64, CompiscriptParser::RuleLogicalAndExpr);
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
    setState(346);
    equalityExpr();
    setState(351);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__31) {
      setState(347);
      match(CompiscriptParser::T__31);
      setState(348);
      equalityExpr();
      setState(353);
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

//----------------- EqualityExprContext ------------------------------------------------------------------

CompiscriptParser::EqualityExprContext::EqualityExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::RelationalExprContext *> CompiscriptParser::EqualityExprContext::relationalExpr() {
  return getRuleContexts<CompiscriptParser::RelationalExprContext>();
}

CompiscriptParser::RelationalExprContext* CompiscriptParser::EqualityExprContext::relationalExpr(size_t i) {
  return getRuleContext<CompiscriptParser::RelationalExprContext>(i);
}


size_t CompiscriptParser::EqualityExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleEqualityExpr;
}


std::any CompiscriptParser::EqualityExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitEqualityExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::EqualityExprContext* CompiscriptParser::equalityExpr() {
  EqualityExprContext *_localctx = _tracker.createInstance<EqualityExprContext>(_ctx, getState());
  enterRule(_localctx, 66, CompiscriptParser::RuleEqualityExpr);
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
    relationalExpr();
    setState(359);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__32

    || _la == CompiscriptParser::T__33) {
      setState(355);
      _la = _input->LA(1);
      if (!(_la == CompiscriptParser::T__32

      || _la == CompiscriptParser::T__33)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(356);
      relationalExpr();
      setState(361);
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

//----------------- RelationalExprContext ------------------------------------------------------------------

CompiscriptParser::RelationalExprContext::RelationalExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::AdditiveExprContext *> CompiscriptParser::RelationalExprContext::additiveExpr() {
  return getRuleContexts<CompiscriptParser::AdditiveExprContext>();
}

CompiscriptParser::AdditiveExprContext* CompiscriptParser::RelationalExprContext::additiveExpr(size_t i) {
  return getRuleContext<CompiscriptParser::AdditiveExprContext>(i);
}


size_t CompiscriptParser::RelationalExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleRelationalExpr;
}


std::any CompiscriptParser::RelationalExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitRelationalExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::RelationalExprContext* CompiscriptParser::relationalExpr() {
  RelationalExprContext *_localctx = _tracker.createInstance<RelationalExprContext>(_ctx, getState());
  enterRule(_localctx, 68, CompiscriptParser::RuleRelationalExpr);
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
    setState(362);
    additiveExpr();
    setState(367);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 515396075520) != 0)) {
      setState(363);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 515396075520) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(364);
      additiveExpr();
      setState(369);
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

//----------------- AdditiveExprContext ------------------------------------------------------------------

CompiscriptParser::AdditiveExprContext::AdditiveExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::MultiplicativeExprContext *> CompiscriptParser::AdditiveExprContext::multiplicativeExpr() {
  return getRuleContexts<CompiscriptParser::MultiplicativeExprContext>();
}

CompiscriptParser::MultiplicativeExprContext* CompiscriptParser::AdditiveExprContext::multiplicativeExpr(size_t i) {
  return getRuleContext<CompiscriptParser::MultiplicativeExprContext>(i);
}


size_t CompiscriptParser::AdditiveExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleAdditiveExpr;
}


std::any CompiscriptParser::AdditiveExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::AdditiveExprContext* CompiscriptParser::additiveExpr() {
  AdditiveExprContext *_localctx = _tracker.createInstance<AdditiveExprContext>(_ctx, getState());
  enterRule(_localctx, 70, CompiscriptParser::RuleAdditiveExpr);
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
    setState(370);
    multiplicativeExpr();
    setState(375);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__38

    || _la == CompiscriptParser::T__39) {
      setState(371);
      _la = _input->LA(1);
      if (!(_la == CompiscriptParser::T__38

      || _la == CompiscriptParser::T__39)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(372);
      multiplicativeExpr();
      setState(377);
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

//----------------- MultiplicativeExprContext ------------------------------------------------------------------

CompiscriptParser::MultiplicativeExprContext::MultiplicativeExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::UnaryExprContext *> CompiscriptParser::MultiplicativeExprContext::unaryExpr() {
  return getRuleContexts<CompiscriptParser::UnaryExprContext>();
}

CompiscriptParser::UnaryExprContext* CompiscriptParser::MultiplicativeExprContext::unaryExpr(size_t i) {
  return getRuleContext<CompiscriptParser::UnaryExprContext>(i);
}


size_t CompiscriptParser::MultiplicativeExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleMultiplicativeExpr;
}


std::any CompiscriptParser::MultiplicativeExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::MultiplicativeExprContext* CompiscriptParser::multiplicativeExpr() {
  MultiplicativeExprContext *_localctx = _tracker.createInstance<MultiplicativeExprContext>(_ctx, getState());
  enterRule(_localctx, 72, CompiscriptParser::RuleMultiplicativeExpr);
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
    setState(378);
    unaryExpr();
    setState(383);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15393162788864) != 0)) {
      setState(379);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 15393162788864) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(380);
      unaryExpr();
      setState(385);
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

//----------------- UnaryExprContext ------------------------------------------------------------------

CompiscriptParser::UnaryExprContext::UnaryExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::UnaryExprContext* CompiscriptParser::UnaryExprContext::unaryExpr() {
  return getRuleContext<CompiscriptParser::UnaryExprContext>(0);
}

CompiscriptParser::PrimaryExprContext* CompiscriptParser::UnaryExprContext::primaryExpr() {
  return getRuleContext<CompiscriptParser::PrimaryExprContext>(0);
}


size_t CompiscriptParser::UnaryExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleUnaryExpr;
}


std::any CompiscriptParser::UnaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitUnaryExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::UnaryExprContext* CompiscriptParser::unaryExpr() {
  UnaryExprContext *_localctx = _tracker.createInstance<UnaryExprContext>(_ctx, getState());
  enterRule(_localctx, 74, CompiscriptParser::RuleUnaryExpr);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(389);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::T__39:
      case CompiscriptParser::T__43: {
        enterOuterAlt(_localctx, 1);
        setState(386);
        _la = _input->LA(1);
        if (!(_la == CompiscriptParser::T__39

        || _la == CompiscriptParser::T__43)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(387);
        unaryExpr();
        break;
      }

      case CompiscriptParser::T__9:
      case CompiscriptParser::T__44:
      case CompiscriptParser::T__45:
      case CompiscriptParser::T__46:
      case CompiscriptParser::T__47:
      case CompiscriptParser::T__48:
      case CompiscriptParser::T__49:
      case CompiscriptParser::FloatLiteral:
      case CompiscriptParser::IntegerLiteral:
      case CompiscriptParser::StringLiteral:
      case CompiscriptParser::Identifier: {
        enterOuterAlt(_localctx, 2);
        setState(388);
        primaryExpr();
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

//----------------- PrimaryExprContext ------------------------------------------------------------------

CompiscriptParser::PrimaryExprContext::PrimaryExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::LiteralExprContext* CompiscriptParser::PrimaryExprContext::literalExpr() {
  return getRuleContext<CompiscriptParser::LiteralExprContext>(0);
}

CompiscriptParser::LeftHandSideContext* CompiscriptParser::PrimaryExprContext::leftHandSide() {
  return getRuleContext<CompiscriptParser::LeftHandSideContext>(0);
}

CompiscriptParser::ExpressionContext* CompiscriptParser::PrimaryExprContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}


size_t CompiscriptParser::PrimaryExprContext::getRuleIndex() const {
  return CompiscriptParser::RulePrimaryExpr;
}


std::any CompiscriptParser::PrimaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::PrimaryExprContext* CompiscriptParser::primaryExpr() {
  PrimaryExprContext *_localctx = _tracker.createInstance<PrimaryExprContext>(_ctx, getState());
  enterRule(_localctx, 76, CompiscriptParser::RulePrimaryExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(397);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::T__44:
      case CompiscriptParser::T__45:
      case CompiscriptParser::T__46:
      case CompiscriptParser::T__49:
      case CompiscriptParser::FloatLiteral:
      case CompiscriptParser::IntegerLiteral:
      case CompiscriptParser::StringLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(391);
        literalExpr();
        break;
      }

      case CompiscriptParser::T__47:
      case CompiscriptParser::T__48:
      case CompiscriptParser::Identifier: {
        enterOuterAlt(_localctx, 2);
        setState(392);
        leftHandSide();
        break;
      }

      case CompiscriptParser::T__9: {
        enterOuterAlt(_localctx, 3);
        setState(393);
        match(CompiscriptParser::T__9);
        setState(394);
        expression();
        setState(395);
        match(CompiscriptParser::T__10);
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

//----------------- LiteralExprContext ------------------------------------------------------------------

CompiscriptParser::LiteralExprContext::LiteralExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::LiteralExprContext::IntegerLiteral() {
  return getToken(CompiscriptParser::IntegerLiteral, 0);
}

tree::TerminalNode* CompiscriptParser::LiteralExprContext::FloatLiteral() {
  return getToken(CompiscriptParser::FloatLiteral, 0);
}

tree::TerminalNode* CompiscriptParser::LiteralExprContext::StringLiteral() {
  return getToken(CompiscriptParser::StringLiteral, 0);
}

CompiscriptParser::ArrayLiteralContext* CompiscriptParser::LiteralExprContext::arrayLiteral() {
  return getRuleContext<CompiscriptParser::ArrayLiteralContext>(0);
}


size_t CompiscriptParser::LiteralExprContext::getRuleIndex() const {
  return CompiscriptParser::RuleLiteralExpr;
}


std::any CompiscriptParser::LiteralExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitLiteralExpr(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::LiteralExprContext* CompiscriptParser::literalExpr() {
  LiteralExprContext *_localctx = _tracker.createInstance<LiteralExprContext>(_ctx, getState());
  enterRule(_localctx, 78, CompiscriptParser::RuleLiteralExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(406);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::IntegerLiteral: {
        enterOuterAlt(_localctx, 1);
        setState(399);
        match(CompiscriptParser::IntegerLiteral);
        break;
      }

      case CompiscriptParser::FloatLiteral: {
        enterOuterAlt(_localctx, 2);
        setState(400);
        match(CompiscriptParser::FloatLiteral);
        break;
      }

      case CompiscriptParser::StringLiteral: {
        enterOuterAlt(_localctx, 3);
        setState(401);
        match(CompiscriptParser::StringLiteral);
        break;
      }

      case CompiscriptParser::T__49: {
        enterOuterAlt(_localctx, 4);
        setState(402);
        arrayLiteral();
        break;
      }

      case CompiscriptParser::T__44: {
        enterOuterAlt(_localctx, 5);
        setState(403);
        match(CompiscriptParser::T__44);
        break;
      }

      case CompiscriptParser::T__45: {
        enterOuterAlt(_localctx, 6);
        setState(404);
        match(CompiscriptParser::T__45);
        break;
      }

      case CompiscriptParser::T__46: {
        enterOuterAlt(_localctx, 7);
        setState(405);
        match(CompiscriptParser::T__46);
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

//----------------- LeftHandSideContext ------------------------------------------------------------------

CompiscriptParser::LeftHandSideContext::LeftHandSideContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::PrimaryAtomContext* CompiscriptParser::LeftHandSideContext::primaryAtom() {
  return getRuleContext<CompiscriptParser::PrimaryAtomContext>(0);
}

std::vector<CompiscriptParser::SuffixOpContext *> CompiscriptParser::LeftHandSideContext::suffixOp() {
  return getRuleContexts<CompiscriptParser::SuffixOpContext>();
}

CompiscriptParser::SuffixOpContext* CompiscriptParser::LeftHandSideContext::suffixOp(size_t i) {
  return getRuleContext<CompiscriptParser::SuffixOpContext>(i);
}


size_t CompiscriptParser::LeftHandSideContext::getRuleIndex() const {
  return CompiscriptParser::RuleLeftHandSide;
}


std::any CompiscriptParser::LeftHandSideContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitLeftHandSide(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::LeftHandSideContext* CompiscriptParser::leftHandSide() {
  LeftHandSideContext *_localctx = _tracker.createInstance<LeftHandSideContext>(_ctx, getState());
  enterRule(_localctx, 80, CompiscriptParser::RuleLeftHandSide);
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
    setState(408);
    primaryAtom();
    setState(412);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5629499534214144) != 0)) {
      setState(409);
      suffixOp();
      setState(414);
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

//----------------- PrimaryAtomContext ------------------------------------------------------------------

CompiscriptParser::PrimaryAtomContext::PrimaryAtomContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t CompiscriptParser::PrimaryAtomContext::getRuleIndex() const {
  return CompiscriptParser::RulePrimaryAtom;
}

void CompiscriptParser::PrimaryAtomContext::copyFrom(PrimaryAtomContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- IdentifierExprContext ------------------------------------------------------------------

tree::TerminalNode* CompiscriptParser::IdentifierExprContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::IdentifierExprContext::IdentifierExprContext(PrimaryAtomContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::IdentifierExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitIdentifierExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NewExprContext ------------------------------------------------------------------

tree::TerminalNode* CompiscriptParser::NewExprContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::ArgumentsContext* CompiscriptParser::NewExprContext::arguments() {
  return getRuleContext<CompiscriptParser::ArgumentsContext>(0);
}

CompiscriptParser::NewExprContext::NewExprContext(PrimaryAtomContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::NewExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitNewExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ThisExprContext ------------------------------------------------------------------

CompiscriptParser::ThisExprContext::ThisExprContext(PrimaryAtomContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::ThisExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitThisExpr(this);
  else
    return visitor->visitChildren(this);
}
CompiscriptParser::PrimaryAtomContext* CompiscriptParser::primaryAtom() {
  PrimaryAtomContext *_localctx = _tracker.createInstance<PrimaryAtomContext>(_ctx, getState());
  enterRule(_localctx, 82, CompiscriptParser::RulePrimaryAtom);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(424);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::Identifier: {
        _localctx = _tracker.createInstance<CompiscriptParser::IdentifierExprContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(415);
        match(CompiscriptParser::Identifier);
        break;
      }

      case CompiscriptParser::T__47: {
        _localctx = _tracker.createInstance<CompiscriptParser::NewExprContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(416);
        match(CompiscriptParser::T__47);
        setState(417);
        match(CompiscriptParser::Identifier);
        setState(418);
        match(CompiscriptParser::T__9);
        setState(420);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 4325690949414945792) != 0)) {
          setState(419);
          arguments();
        }
        setState(422);
        match(CompiscriptParser::T__10);
        break;
      }

      case CompiscriptParser::T__48: {
        _localctx = _tracker.createInstance<CompiscriptParser::ThisExprContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(423);
        match(CompiscriptParser::T__48);
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

//----------------- SuffixOpContext ------------------------------------------------------------------

CompiscriptParser::SuffixOpContext::SuffixOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t CompiscriptParser::SuffixOpContext::getRuleIndex() const {
  return CompiscriptParser::RuleSuffixOp;
}

void CompiscriptParser::SuffixOpContext::copyFrom(SuffixOpContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- CallExprContext ------------------------------------------------------------------

CompiscriptParser::ArgumentsContext* CompiscriptParser::CallExprContext::arguments() {
  return getRuleContext<CompiscriptParser::ArgumentsContext>(0);
}

CompiscriptParser::CallExprContext::CallExprContext(SuffixOpContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::CallExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitCallExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PropertyAccessExprContext ------------------------------------------------------------------

tree::TerminalNode* CompiscriptParser::PropertyAccessExprContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}

CompiscriptParser::PropertyAccessExprContext::PropertyAccessExprContext(SuffixOpContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::PropertyAccessExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitPropertyAccessExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IndexExprContext ------------------------------------------------------------------

CompiscriptParser::ExpressionContext* CompiscriptParser::IndexExprContext::expression() {
  return getRuleContext<CompiscriptParser::ExpressionContext>(0);
}

CompiscriptParser::IndexExprContext::IndexExprContext(SuffixOpContext *ctx) { copyFrom(ctx); }


std::any CompiscriptParser::IndexExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitIndexExpr(this);
  else
    return visitor->visitChildren(this);
}
CompiscriptParser::SuffixOpContext* CompiscriptParser::suffixOp() {
  SuffixOpContext *_localctx = _tracker.createInstance<SuffixOpContext>(_ctx, getState());
  enterRule(_localctx, 84, CompiscriptParser::RuleSuffixOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(437);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case CompiscriptParser::T__9: {
        _localctx = _tracker.createInstance<CompiscriptParser::CallExprContext>(_localctx);
        enterOuterAlt(_localctx, 1);
        setState(426);
        match(CompiscriptParser::T__9);
        setState(428);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 4325690949414945792) != 0)) {
          setState(427);
          arguments();
        }
        setState(430);
        match(CompiscriptParser::T__10);
        break;
      }

      case CompiscriptParser::T__49: {
        _localctx = _tracker.createInstance<CompiscriptParser::IndexExprContext>(_localctx);
        enterOuterAlt(_localctx, 2);
        setState(431);
        match(CompiscriptParser::T__49);
        setState(432);
        expression();
        setState(433);
        match(CompiscriptParser::T__50);
        break;
      }

      case CompiscriptParser::T__51: {
        _localctx = _tracker.createInstance<CompiscriptParser::PropertyAccessExprContext>(_localctx);
        enterOuterAlt(_localctx, 3);
        setState(435);
        match(CompiscriptParser::T__51);
        setState(436);
        match(CompiscriptParser::Identifier);
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

//----------------- ArgumentsContext ------------------------------------------------------------------

CompiscriptParser::ArgumentsContext::ArgumentsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::ExpressionContext *> CompiscriptParser::ArgumentsContext::expression() {
  return getRuleContexts<CompiscriptParser::ExpressionContext>();
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ArgumentsContext::expression(size_t i) {
  return getRuleContext<CompiscriptParser::ExpressionContext>(i);
}


size_t CompiscriptParser::ArgumentsContext::getRuleIndex() const {
  return CompiscriptParser::RuleArguments;
}


std::any CompiscriptParser::ArgumentsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitArguments(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ArgumentsContext* CompiscriptParser::arguments() {
  ArgumentsContext *_localctx = _tracker.createInstance<ArgumentsContext>(_ctx, getState());
  enterRule(_localctx, 86, CompiscriptParser::RuleArguments);
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
    setState(439);
    expression();
    setState(444);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__27) {
      setState(440);
      match(CompiscriptParser::T__27);
      setState(441);
      expression();
      setState(446);
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

//----------------- ArrayLiteralContext ------------------------------------------------------------------

CompiscriptParser::ArrayLiteralContext::ArrayLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<CompiscriptParser::ExpressionContext *> CompiscriptParser::ArrayLiteralContext::expression() {
  return getRuleContexts<CompiscriptParser::ExpressionContext>();
}

CompiscriptParser::ExpressionContext* CompiscriptParser::ArrayLiteralContext::expression(size_t i) {
  return getRuleContext<CompiscriptParser::ExpressionContext>(i);
}


size_t CompiscriptParser::ArrayLiteralContext::getRuleIndex() const {
  return CompiscriptParser::RuleArrayLiteral;
}


std::any CompiscriptParser::ArrayLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitArrayLiteral(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::ArrayLiteralContext* CompiscriptParser::arrayLiteral() {
  ArrayLiteralContext *_localctx = _tracker.createInstance<ArrayLiteralContext>(_ctx, getState());
  enterRule(_localctx, 88, CompiscriptParser::RuleArrayLiteral);
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
    setState(447);
    match(CompiscriptParser::T__49);
    setState(456);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 4325690949414945792) != 0)) {
      setState(448);
      expression();
      setState(453);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == CompiscriptParser::T__27) {
        setState(449);
        match(CompiscriptParser::T__27);
        setState(450);
        expression();
        setState(455);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(458);
    match(CompiscriptParser::T__50);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

CompiscriptParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

CompiscriptParser::BaseTypeContext* CompiscriptParser::TypeContext::baseType() {
  return getRuleContext<CompiscriptParser::BaseTypeContext>(0);
}


size_t CompiscriptParser::TypeContext::getRuleIndex() const {
  return CompiscriptParser::RuleType;
}


std::any CompiscriptParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::TypeContext* CompiscriptParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 90, CompiscriptParser::RuleType);
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
    setState(460);
    baseType();
    setState(465);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == CompiscriptParser::T__49) {
      setState(461);
      match(CompiscriptParser::T__49);
      setState(462);
      match(CompiscriptParser::T__50);
      setState(467);
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

//----------------- BaseTypeContext ------------------------------------------------------------------

CompiscriptParser::BaseTypeContext::BaseTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* CompiscriptParser::BaseTypeContext::Identifier() {
  return getToken(CompiscriptParser::Identifier, 0);
}


size_t CompiscriptParser::BaseTypeContext::getRuleIndex() const {
  return CompiscriptParser::RuleBaseType;
}


std::any CompiscriptParser::BaseTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<CompiscriptVisitor*>(visitor))
    return parserVisitor->visitBaseType(this);
  else
    return visitor->visitChildren(this);
}

CompiscriptParser::BaseTypeContext* CompiscriptParser::baseType() {
  BaseTypeContext *_localctx = _tracker.createInstance<BaseTypeContext>(_ctx, getState());
  enterRule(_localctx, 92, CompiscriptParser::RuleBaseType);
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
    setState(468);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 2585066186110664704) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void CompiscriptParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  compiscriptParserInitialize();
#else
  ::antlr4::internal::call_once(compiscriptParserOnceFlag, compiscriptParserInitialize);
#endif
}
