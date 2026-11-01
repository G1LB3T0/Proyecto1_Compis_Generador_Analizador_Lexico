// Orquesta el intérprete:
//   SQL -> lexer -> parser -> AST -> análisis semántico -> interpretación
//       -> archivos de texto -> resultado
// Las fases léxica/sintáctica y la tabla de tokens se reutilizan del
// Proyecto 2; AST, semántica SQL e interpretación son nuevas.
#include "sql/service.h"

#include "SQLLexer.h"
#include "SQLParser.h"
#include "sql/ast_builder.h"
#include "sql/ast_serializer.h"
#include "sql/diagnostics.h"
#include "sql/executor.h"
#include "sql/json.h"
#include "sql/semantic_analyzer.h"
#include "sql/storage.h"
#include "sql/token_classifier.h"
#include "sql/tree_serializer.h"

#include "antlr4-runtime.h"

namespace sql {

RunResult runScript(const std::string &source, const RunOptions &options) {
  RunResult result;
  result.filename = options.filename;
  result.mode = options.execute ? "execute" : "analyze";

  // Fase léxica: ANTLR transforma caracteres en tokens y reporta símbolos no
  // reconocidos mediante nuestro listener.
  antlr4::ANTLRInputStream input(source);
  SQLLexer lexer(&input);
  CollectingErrorListener lexical_listener("léxico");
  lexer.removeErrorListeners();
  lexer.addErrorListener(&lexical_listener);

  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();
  for (antlr4::Token *token : tokens.getTokens()) {
    if (token->getType() == antlr4::Token::EOF)
      continue;
    std::string name(lexer.getVocabulary().getSymbolicName(token->getType()));
    if (name.empty())
      name = std::to_string(token->getType());
    const std::string text = token->getText();
    result.tokens.push_back({name, classifyToken(name, text), text, token->getLine(),
                             token->getCharPositionInLine() + 1});
  }

  // Fase sintáctica: el parser valida la gramática y produce el parse tree.
  SQLParser parser(&tokens);
  CollectingErrorListener syntax_listener("sintáctico");
  parser.removeErrorListeners();
  parser.addErrorListener(&syntax_listener);
  SQLParser::ScriptContext *tree = parser.script();
  result.tree_json = parseTreeToJson(tree, parser);

  result.lexical_diagnostics = lexical_listener.diagnostics();
  result.syntax_diagnostics = syntax_listener.diagnostics();
  result.lexical_ok = result.lexical_diagnostics.empty();
  result.syntax_ok = result.lexical_ok && result.syntax_diagnostics.empty();

  Storage storage(options.database);
  std::string storage_error;
  if (!storage.open(storage_error)) {
    result.runtime_diagnostics.push_back(
        {"EXE008", "ejecución", "error", storage_error, 0, 0});
  }

  if (!result.syntax_ok) {
    result.semantic_skipped = true;
    result.execution_skipped = true;
    result.execution_reason = "El análisis semántico y la ejecución se omitieron "
                              "porque existen errores léxicos o sintácticos.";
    result.database_json = databaseToJson(storage);
    return result;
  }

  // AST: el parse tree concreto se reduce a nodos con significado.
  AstBuilder builder;
  Script script = builder.build(tree);
  result.semantic_diagnostics = builder.diagnostics();

  // Semántica: se valida todo el script contra una copia del catálogo.
  SemanticAnalyzer analyzer(storage.catalog());
  const auto semantic = analyzer.analyze(script);
  result.semantic_diagnostics.insert(result.semantic_diagnostics.end(), semantic.begin(),
                                     semantic.end());
  result.semantic_ok = result.semantic_diagnostics.empty();
  result.ast_json = astToJson(script); // ya decorado con tipos y columnas

  for (size_t index = 0; index < script.statements.size(); ++index) {
    const Statement &statement = script.statements[index];
    StatementResult item;
    item.index = static_cast<int>(index) + 1;
    item.kind = statementKindName(statement.kind);
    item.sql = statement.sql;
    item.line = statement.loc.line;
    item.column = statement.loc.column;
    item.status = "pendiente";
    result.statements.push_back(std::move(item));
  }
  for (const auto &diagnostic : result.semantic_diagnostics) {
    if (diagnostic.statement < 1)
      continue;
    StatementResult &item = result.statements[diagnostic.statement - 1];
    if (item.status != "error") {
      item.status = "error";
      item.message = diagnostic.message;
    }
  }

  if (!result.semantic_ok || !result.runtime_diagnostics.empty() || !options.execute) {
    result.execution_skipped = true;
    if (!result.runtime_diagnostics.empty())
      result.execution_reason = "No se pudo abrir la base de datos.";
    else if (!options.execute)
      result.execution_reason = "Modo análisis: se validó el script sin ejecutarlo "
                                "ni modificar los archivos.";
    else if (!result.semantic_ok)
      result.execution_reason = "La ejecución se omitió porque existen errores "
                                "semánticos; ningún archivo fue modificado.";
    else
      result.execution_reason = "No se pudo abrir la base de datos.";
    for (auto &item : result.statements) {
      if (item.status == "pendiente") {
        item.status = options.execute ? "omitida" : "analizada";
        item.message = options.execute ? "No se ejecutó." : "Semánticamente válida.";
      }
    }
    result.ok = result.semantic_ok && !options.execute && result.runtime_diagnostics.empty();
    result.database_json = databaseToJson(storage);
    return result;
  }

  // Interpretación: cada sentencia se ejecuta y persiste en orden; al primer
  // error de ejecución las siguientes se omiten (las anteriores ya quedaron).
  Executor executor(storage);
  result.execution_ok = true;
  for (size_t index = 0; index < script.statements.size(); ++index) {
    StatementResult &item = result.statements[index];
    if (!result.execution_ok) {
      item.status = "omitida";
      item.message = "No se ejecutó por el error de una sentencia anterior.";
      continue;
    }
    Diagnostic failure;
    if (!executor.execute(script.statements[index], item, failure)) {
      result.runtime_diagnostics.push_back(failure);
      result.execution_ok = false;
    }
  }
  if (!result.execution_ok)
    result.execution_reason = "La ejecución se detuvo en la primera sentencia con "
                              "error; las anteriores ya fueron guardadas.";
  result.ok = result.execution_ok;
  result.database_json = databaseToJson(storage);
  return result;
}

std::string describeDatabase(const std::string &database, std::string &error) {
  Storage storage(database);
  if (!storage.open(error))
    return "";
  return databaseToJson(storage);
}

bool resetDatabase(const std::string &database, std::string &error) {
  Storage storage(database);
  if (!storage.open(error))
    return false;
  return storage.reset(error);
}

} // namespace sql
