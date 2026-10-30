// Modelos que viajan del núcleo C++ al IDE: diagnósticos, tokens y resultados.
#pragma once

#include "sql/value.h"

#include <string>
#include <vector>

namespace sql {

// Error listo para mostrar, con fase, código y ubicación en el script.
struct Diagnostic {
  std::string code;     // LEX001, SYN001, SEM0xx, EXE0xx
  std::string category; // léxico, sintáctico, semántico, ejecución
  std::string severity = "error";
  std::string message;
  size_t line = 0;
  size_t column = 0;
  int statement = -1; // número de sentencia (1..n) cuando aplica
};

struct TokenInfo {
  std::string type;
  std::string category;
  std::string text;
  size_t line = 0;
  size_t column = 0;
};

// Resultado de interpretar una sentencia.
struct StatementResult {
  int index = 0;
  std::string kind; // CREATE TABLE, INSERT, SELECT...
  std::string sql;
  size_t line = 0;
  size_t column = 0;
  std::string status = "pendiente"; // ok | error | omitida | analizada
  std::string message;
  std::vector<std::string> columns;
  std::vector<std::string> column_types;
  std::vector<std::vector<Value>> rows;
  long long affected = 0;
  std::vector<std::string> plan;  // decisiones del intérprete (índices, joins)
  std::vector<std::string> files; // archivos escritos en disco
};

// Resultado completo del pipeline: tokens → árbol → AST → semántica → ejecución.
struct RunResult {
  std::string filename;
  std::string mode; // execute | analyze
  std::vector<TokenInfo> tokens;
  std::string tree_json;
  std::string ast_json;
  std::vector<Diagnostic> lexical_diagnostics;
  std::vector<Diagnostic> syntax_diagnostics;
  std::vector<Diagnostic> semantic_diagnostics;
  std::vector<Diagnostic> runtime_diagnostics;
  std::vector<StatementResult> statements;
  bool lexical_ok = false;
  bool syntax_ok = false;
  bool semantic_ok = false;
  bool semantic_skipped = false;
  bool execution_ok = false;
  bool execution_skipped = false;
  std::string execution_reason;
  bool ok = false;
  std::string database_json; // catálogo y archivos después de ejecutar
};

} // namespace sql
