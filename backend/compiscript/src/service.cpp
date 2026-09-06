#include "compiscript/service.h"

#include "CompiscriptLexer.h"
#include "CompiscriptParser.h"
#include "compiscript/diagnostics.h"
#include "compiscript/semantic_analyzer.h"
#include "compiscript/token_classifier.h"
#include "compiscript/tree_serializer.h"

#include "antlr4-runtime.h"

namespace compiscript {

AnalysisResult analyzeSource(const std::string &source,
                             const std::string &filename) {
  AnalysisResult result;
  result.filename = filename;

  antlr4::ANTLRInputStream input(source);
  CompiscriptLexer lexer(&input);
  CollectingErrorListener lexical_listener("léxico");
  lexer.removeErrorListeners();
  lexer.addErrorListener(&lexical_listener);

  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();
  for (antlr4::Token *token : tokens.getTokens()) {
    if (token->getType() == antlr4::Token::EOF)
      continue;
    std::string name(lexer.getVocabulary().getSymbolicName(token->getType()));
    if (name.empty()) {
      name =
          std::string(lexer.getVocabulary().getLiteralName(token->getType()));
    }
    if (name.empty())
      name = std::to_string(token->getType());
    const std::string text = token->getText();
    result.tokens.push_back({name, classifyToken(name, text), text,
                             token->getLine(),
                             token->getCharPositionInLine() + 1});
  }

  CompiscriptParser parser(&tokens);
  CollectingErrorListener syntax_listener("sintáctico");
  parser.removeErrorListeners();
  parser.addErrorListener(&syntax_listener);
  CompiscriptParser::ProgramContext *tree = parser.program();
  result.tree_json = parseTreeToJson(tree, parser);

  result.syntax_diagnostics = lexical_listener.diagnostics();
  const auto &parser_errors = syntax_listener.diagnostics();
  result.syntax_diagnostics.insert(result.syntax_diagnostics.end(),
                                   parser_errors.begin(), parser_errors.end());
  result.syntax_ok = result.syntax_diagnostics.empty();
  if (result.syntax_ok) {
    SemanticAnalyzer analyzer;
    result.semantic = analyzer.analyze(tree);
  } else {
    result.semantic.ok = false;
    result.semantic.skipped = true;
    result.semantic.reason = "El análisis semántico se omitió porque existen "
                             "errores léxicos o sintácticos.";
  }
  result.semantic_ok = result.syntax_ok && result.semantic.ok;
  result.ok = result.syntax_ok && result.semantic_ok;
  return result;
}

} // namespace compiscript
