// Intérprete: ejecuta sobre los archivos las sentencias ya validadas.
#pragma once

#include "sql/ast.h"
#include "sql/model.h"
#include "sql/storage.h"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace sql {

// Error que solo puede detectarse con los datos reales (clave duplicada,
// división entre cero, subconsulta escalar con varios registros...).
struct ExecutionError {
  std::string code;
  std::string message;
  SourceLocation loc;
};

class Executor {
public:
  explicit Executor(Storage &storage);

  // Ejecuta una sentencia. Cada sentencia es atómica: trabaja sobre copias de
  // la tabla y sus índices y solo las guarda en disco si termina sin error.
  bool execute(Statement &statement, StatementResult &result, Diagnostic &failure);

private:
  // Contexto de evaluación: la fila actual, el grupo (para agregados) y el
  // contexto de la consulta externa (para subconsultas correlacionadas).
  struct Context {
    const Row *row = nullptr;
    const std::vector<const Row *> *group = nullptr;
    const Context *outer = nullptr;
  };

  // Resultado de consultar un índice para una tabla.
  struct IndexAccess {
    std::string index;
    std::string condition;
    std::set<long long> rowids;
  };

  struct SubqueryStats {
    int executions = 0;
    bool correlated = false;
    size_t line = 0;
    std::vector<std::string> plan;
  };

  void createTable(CreateTableStatement &node, StatementResult &result);
  void dropTable(DropTableStatement &node, StatementResult &result);
  void createIndex(CreateIndexStatement &node, StatementResult &result);
  void dropIndex(DropIndexStatement &node, StatementResult &result);
  void insert(InsertStatement &node, StatementResult &result);
  void update(UpdateStatement &node, StatementResult &result);
  void remove(DeleteStatement &node, StatementResult &result);
  void select(SelectStatement &node, StatementResult &result);

  std::vector<Row> runSelect(const SelectStatement &select, const Context *outer,
                             std::vector<std::string> *plan);
  std::vector<Row> subqueryRows(const Expr &expr, const Context &ctx);

  Value evaluate(const Expr &expr, const Context &ctx);
  Value evaluateBinary(const Expr &expr, const Context &ctx);
  Value evaluateAggregate(const Expr &expr, const Context &ctx);
  static bool isTrue(const Value &value);

  // Planificador: busca en la condición un predicado sobre una columna
  // indexada (col = v, col IN (...), col < v...) para no recorrer la tabla.
  std::optional<IndexAccess> chooseIndex(const std::string &table, size_t offset,
                                         size_t width, const Expr *condition,
                                         const Context &ctx);

  // Restricciones verificadas con los datos.
  void checkUnique(const std::string &table, const Row &row, long long rowid,
                   std::map<std::string, IndexData> &indexes, const SourceLocation &loc);
  void checkForeignKeys(const TableSchema &schema, const Row &row,
                        const TableData &own_data, const SourceLocation &loc);
  void checkReferencedBy(const TableSchema &schema, const std::vector<Row> &old_rows,
                         const std::vector<Row> &new_rows, const std::set<long long> &removed,
                         const SourceLocation &loc);
  Row coerceRow(const TableSchema &schema, Row row, const SourceLocation &loc);

  TableData &loadTable(const std::string &name);
  TableData loadRows(const std::string &name, const std::set<long long> &rowids);
  size_t rowCount(const std::string &name);
  IndexData &loadIndex(const std::string &name);
  void persistTable(const std::string &name, TableData data,
                    std::map<std::string, IndexData> indexes, StatementResult &result);
  void saveCatalog(StatementResult &result);

  Storage &storage_;
  std::map<const SelectStatement *, std::vector<Row>> subquery_cache_;
  std::map<const SelectStatement *, SubqueryStats> subquery_stats_;
};

} // namespace sql
