// Árbol de Sintaxis Abstracta (AST) del intérprete SQL.
//
// El parse tree de ANTLR conserva cada token (paréntesis, comas, palabras
// clave). El AST guarda solo el significado de cada sentencia y es lo que
// recorren la semántica y el intérprete. La semántica además lo "decora":
// completa tipos y la posición de cada columna (campos marcados abajo).
#pragma once

#include "sql/catalog.h"
#include "sql/value.h"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace sql {

struct SourceLocation {
  size_t line = 0;
  size_t column = 0;
};

struct SelectStatement;

enum class ExprKind {
  Literal,        // 18, 'Ana', TRUE, NULL, DATE '2024-01-01'
  Column,         // edad, e.nombre
  Unary,          // -x, NOT x
  Binary,         // a + b, a >= b, a AND b
  IsNull,         // x IS [NOT] NULL
  InList,         // x [NOT] IN (1, 2, 3)
  InSubquery,     // x [NOT] IN (SELECT ...)
  Exists,         // EXISTS (SELECT ...)
  ScalarSubquery, // (SELECT MAX(x) FROM t)
  Aggregate       // COUNT, SUM, AVG, MIN, MAX
};

// Nodo de expresión: un solo tipo con `kind` para todas las variantes.
struct Expr {
  ExprKind kind = ExprKind::Literal;
  SourceLocation loc;
  std::string text; // fragmento original, para mensajes de error

  Value literal;          // Literal
  std::string table;      // Column: calificador opcional (alias o tabla)
  std::string column;     // Column
  std::string op;         // Unary/Binary: + - * / % = <> < <= > >= AND OR NOT
  std::string function;   // Aggregate: COUNT SUM AVG MIN MAX
  bool negated = false;   // NOT IN, IS NOT NULL
  bool distinct = false;  // COUNT(DISTINCT x)
  bool star = false;      // COUNT(*)
  std::vector<std::unique_ptr<Expr>> children;
  std::shared_ptr<SelectStatement> subquery;

  // ── Decoración semántica ──
  DataType type;  // tipo inferido
  int depth = 0;  // Column: 0 = consulta actual, n = n consultas hacia afuera
  int slot = -1;  // Column: posición en la fila combinada de esa consulta
};

using ExprPtr = std::unique_ptr<Expr>;

// ── DDL ──

struct ColumnDefinition {
  std::string name;
  DataType type;
  bool primary_key = false;
  bool not_null = false;
  bool unique = false;
  std::string ref_table; // REFERENCES tabla(columna)
  std::string ref_column;
  SourceLocation loc;
};

// PRIMARY KEY (c), UNIQUE (c), FOREIGN KEY (c) REFERENCES t(c)
struct TableConstraint {
  std::string kind;
  std::string column;
  std::string ref_table;
  std::string ref_column;
  SourceLocation loc;
};

struct CreateTableStatement {
  std::string table;
  bool if_not_exists = false;
  std::vector<ColumnDefinition> columns;
  std::vector<TableConstraint> constraints;
  // Decoración: esquema final e índices automáticos (PRIMARY KEY / UNIQUE).
  TableSchema schema;
  std::vector<IndexSchema> automatic_indexes;
};

struct DropTableStatement {
  std::string table;
  bool if_exists = false;
};

struct CreateIndexStatement {
  std::string name;
  std::string table;
  std::string column;
  bool unique = false;
  SourceLocation table_loc;
  SourceLocation column_loc;
};

struct DropIndexStatement {
  std::string name;
  bool if_exists = false;
};

// ── DML ──

struct InsertStatement {
  std::string table;
  std::vector<std::string> columns; // vacío = todas, en orden de la tabla
  std::vector<SourceLocation> column_locs;
  std::vector<std::vector<ExprPtr>> rows;
  std::vector<SourceLocation> row_locs;
  // Decoración: índice de columna de la tabla para cada valor.
  std::vector<int> target_columns;
};

struct Assignment {
  std::string column;
  SourceLocation loc;
  ExprPtr value;
  int column_index = -1; // decoración
};

struct UpdateStatement {
  std::string table;
  std::vector<Assignment> assignments;
  ExprPtr where;
};

struct DeleteStatement {
  std::string table;
  ExprPtr where;
};

// ── SELECT ──

struct TableReference {
  std::string name;
  std::string alias; // si no hay alias, el nombre de la tabla
  SourceLocation loc;
};

struct JoinClause {
  TableReference table;
  ExprPtr condition;
};

struct SelectItem {
  ExprPtr expr;          // nulo cuando es * o tabla.*
  std::string alias;     // AS alias
  bool star = false;     // * o tabla.*
  std::string star_table;
  SourceLocation loc;
};

struct OrderItem {
  ExprPtr expr;
  bool descending = false;
  int output_index = -1; // decoración: ORDER BY alias -> columna del resultado
};

// Columna del resultado de una consulta (decoración).
struct OutputColumn {
  std::string name;
  DataType type;
};

struct SelectStatement {
  SourceLocation loc;
  bool distinct = false;
  std::vector<SelectItem> items;
  bool has_from = false;
  TableReference from;
  std::vector<JoinClause> joins;
  ExprPtr where;
  std::vector<ExprPtr> group_by;
  ExprPtr having;
  std::vector<OrderItem> order_by;
  std::optional<long long> limit;

  // ── Decoración semántica ──
  bool aggregate_query = false; // GROUP BY o funciones de agregación
  bool correlated = false;      // usa columnas de una consulta externa
  std::vector<OutputColumn> output;
  std::vector<ExprPtr> expanded;        // columnas creadas al expandir * y t.*
  std::vector<const Expr *> projection; // expresiones finales, en orden
};

enum class StatementKind {
  CreateTable,
  DropTable,
  CreateIndex,
  DropIndex,
  Insert,
  Update,
  Delete,
  Select
};

struct Statement {
  StatementKind kind = StatementKind::Select;
  SourceLocation loc;
  std::string sql; // texto original de la sentencia
  std::variant<CreateTableStatement, DropTableStatement, CreateIndexStatement,
               DropIndexStatement, InsertStatement, UpdateStatement,
               DeleteStatement, std::shared_ptr<SelectStatement>>
      node;
};

struct Script {
  std::vector<Statement> statements;
};

std::string statementKindName(StatementKind kind);

} // namespace sql
