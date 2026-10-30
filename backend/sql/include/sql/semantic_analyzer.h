// Análisis semántico: valida cada sentencia contra el catálogo antes de
// ejecutarla y decora el AST con tipos y posiciones de columnas.
#pragma once

#include "sql/ast.h"
#include "sql/catalog.h"
#include "sql/model.h"

#include <string>
#include <vector>

namespace sql {

class SemanticAnalyzer {
public:
  // Trabaja sobre una COPIA del catálogo: un CREATE TABLE de la sentencia 1
  // ya es visible para la sentencia 2, pero nada se escribe en disco aquí.
  explicit SemanticAnalyzer(Catalog catalog);

  std::vector<Diagnostic> analyze(Script &script);

private:
  // Ámbito de nombres de una consulta: tablas visibles y sus columnas.
  struct Scope {
    RowLayout layout;
    SelectStatement *select = nullptr;
    bool incomplete = false; // alguna tabla no existe: evita errores en cascada
  };

  // Reglas de la cláusula donde aparece una expresión.
  struct Clause {
    std::string name;
    bool aggregates = false;
  };

  void analyzeStatement(Statement &statement);
  void analyzeCreateTable(CreateTableStatement &node, const Statement &statement);
  void analyzeDropTable(DropTableStatement &node, const Statement &statement);
  void analyzeCreateIndex(CreateIndexStatement &node, const Statement &statement);
  void analyzeDropIndex(DropIndexStatement &node, const Statement &statement);
  void analyzeInsert(InsertStatement &node, const Statement &statement);
  void analyzeUpdate(UpdateStatement &node, const Statement &statement);
  void analyzeDelete(DeleteStatement &node, const Statement &statement);
  void analyzeSelect(SelectStatement &select);

  DataType check(Expr &expr, const Clause &clause);
  DataType checkBinary(Expr &expr, const Clause &clause);
  void checkCondition(Expr *expr, const Clause &clause);
  void checkAssignable(Expr &value, const ColumnSchema &column);
  bool comparable(Expr &left, Expr &right, const std::string &context,
                  const SourceLocation &loc);
  void resolveColumn(Expr &expr);
  void checkGrouped(const Expr &expr, const SelectStatement &select);
  bool addTableToScope(Scope &scope, const TableReference &reference);

  void error(const std::string &code, const std::string &message,
             const SourceLocation &loc);

  Catalog catalog_;
  std::vector<Scope *> scopes_;
  std::vector<Diagnostic> diagnostics_;
  int statement_number_ = 0;
  int aggregate_depth_ = 0;
};

// Expresiones comparables (=, <, IN...): numéricos entre sí, texto con texto,
// fechas con fechas o con un literal de texto con formato de fecha.
bool typesComparable(const DataType &left, const DataType &right);

} // namespace sql
