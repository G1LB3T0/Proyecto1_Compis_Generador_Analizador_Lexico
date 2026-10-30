// Intérprete del AST decorado. Lee y escribe únicamente archivos de texto.
//
// EXE001 PRIMARY KEY duplicada       EXE005 subconsulta escalar con >1 registro
// EXE002 valor UNIQUE duplicado      EXE006 división entre cero
// EXE003 FK sin registro referenciado EXE007 valor incompatible con la columna
// EXE004 registro referenciado por FK EXE008 error de almacenamiento
#include "sql/executor.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace sql {

namespace {

[[noreturn]] void fail(const std::string &code, const std::string &message,
                       const SourceLocation &loc) {
  throw ExecutionError{code, message, loc};
}

Value boolValue(bool value) { return Value::makeBool(value); }

// Divide una condición en sus conjunciones: a AND b AND c -> [a, b, c].
void conjuncts(const Expr *expr, std::vector<const Expr *> &output) {
  if (!expr)
    return;
  if (expr->kind == ExprKind::Binary && expr->op == "AND") {
    conjuncts(expr->children[0].get(), output);
    conjuncts(expr->children[1].get(), output);
    return;
  }
  output.push_back(expr);
}

// ¿La expresión solo usa columnas ya disponibles (posición < offset)?
// Así su valor es conocido antes de leer la tabla que se quiere indexar.
bool availableBefore(const Expr &expr, size_t offset) {
  switch (expr.kind) {
  case ExprKind::Literal:
    return true;
  case ExprKind::Column:
    return expr.depth > 0 || (expr.slot >= 0 && static_cast<size_t>(expr.slot) < offset);
  case ExprKind::Unary:
  case ExprKind::Binary:
    for (const auto &child : expr.children) {
      if (!availableBefore(*child, offset))
        return false;
    }
    return true;
  case ExprKind::ScalarSubquery:
    // Sin correlación su valor es fijo: se calcula una vez y se busca en el índice.
    return !expr.subquery->correlated;
  default:
    return false; // agregados y subconsultas correlacionadas no sirven aquí
  }
}

std::string flip(const std::string &op) {
  if (op == "<")
    return ">";
  if (op == "<=")
    return ">=";
  if (op == ">")
    return "<";
  if (op == ">=")
    return "<=";
  return op;
}

struct RowsLess {
  bool operator()(const Row &left, const Row &right) const {
    for (size_t index = 0; index < left.size() && index < right.size(); ++index) {
      const int order = compareValues(left[index], right[index]);
      if (order != 0)
        return order < 0;
    }
    return left.size() < right.size();
  }
};

std::string plural(size_t count, const std::string &singular,
                   const std::string &plural_form) {
  return std::to_string(count) + " " + (count == 1 ? singular : plural_form);
}

// Condición en una sola línea y de largo acotado para el plan de ejecución.
std::string compact(const std::string &text) {
  std::string result;
  for (char character : text) {
    const bool space = character == ' ' || character == '\n' || character == '\r' ||
                       character == '\t';
    if (space && (result.empty() || result.back() == ' '))
      continue;
    result += space ? ' ' : character;
  }
  return result.size() > 70 ? result.substr(0, 67) + "..." : result;
}

} // namespace

Executor::Executor(Storage &storage) : storage_(storage) {}

bool Executor::isTrue(const Value &value) {
  return value.kind == Value::Kind::Boolean && value.boolean;
}

TableData &Executor::loadTable(const std::string &name) {
  std::string error;
  TableData *data = storage_.table(name, error);
  if (!data)
    fail("EXE008", "Error al leer la tabla: " + error, {});
  return *data;
}

IndexData &Executor::loadIndex(const std::string &name) {
  std::string error;
  IndexData *data = storage_.index(name, error);
  if (!data)
    fail("EXE008", "Error al leer el índice: " + error, {});
  return *data;
}

void Executor::saveCatalog(StatementResult &result) {
  std::string error;
  if (!storage_.saveCatalog(error))
    fail("EXE008", "Error al guardar metadata.txt: " + error, {});
  result.files.push_back("metadata.txt");
}

// Confirma la sentencia: reemplaza tabla e índices en memoria y los escribe.
void Executor::persistTable(const std::string &name, TableData data,
                           std::map<std::string, IndexData> indexes,
                           StatementResult &result) {
  std::string error;
  storage_.putTable(name, std::move(data));
  if (!storage_.saveTable(name, error))
    fail("EXE008", "Error al guardar " + name + ".csv: " + error, {});
  result.files.push_back(name + ".csv");
  for (auto &[index_name, index] : indexes) {
    storage_.putIndex(index_name, std::move(index));
    if (!storage_.saveIndex(index_name, error))
      fail("EXE008", "Error al guardar el índice " + index_name + ": " + error, {});
    result.files.push_back("indexes/" + index_name + ".idx");
  }
}

bool Executor::execute(Statement &statement, StatementResult &result,
                       Diagnostic &failure) {
  subquery_cache_.clear();
  subquery_stats_.clear();
  try {
    switch (statement.kind) {
    case StatementKind::CreateTable:
      createTable(std::get<CreateTableStatement>(statement.node), result);
      break;
    case StatementKind::DropTable:
      dropTable(std::get<DropTableStatement>(statement.node), result);
      break;
    case StatementKind::CreateIndex:
      createIndex(std::get<CreateIndexStatement>(statement.node), result);
      break;
    case StatementKind::DropIndex:
      dropIndex(std::get<DropIndexStatement>(statement.node), result);
      break;
    case StatementKind::Insert:
      insert(std::get<InsertStatement>(statement.node), result);
      break;
    case StatementKind::Update:
      update(std::get<UpdateStatement>(statement.node), result);
      break;
    case StatementKind::Delete:
      remove(std::get<DeleteStatement>(statement.node), result);
      break;
    case StatementKind::Select:
      select(*std::get<std::shared_ptr<SelectStatement>>(statement.node), result);
      break;
    }
  } catch (const ExecutionError &error) {
    // Si no se conoce una ubicación exacta se usa el inicio de la sentencia.
    const SourceLocation loc = error.loc.line ? error.loc : statement.loc;
    failure = {error.code, "ejecución", "error", error.message, loc.line,
               loc.column, result.index};
    result.status = "error";
    result.message = error.message;
    // Una sentencia fallida no deja cambios: se descarta la caché en memoria.
    std::string ignored;
    storage_.open(ignored);
    return false;
  }
  for (const auto &[select, stats] : subquery_stats_) {
    std::string line = "Subconsulta (línea " + std::to_string(stats.line) + "): " +
                       (stats.correlated
                            ? "correlacionada, se ejecutó " +
                                  plural(stats.executions, "vez", "veces") +
                                  " (una por fila externa)"
                            : "no correlacionada, se ejecutó 1 vez y su resultado se reutilizó");
    result.plan.push_back(line);
    if (stats.correlated && !stats.plan.empty())
      result.plan.push_back("   └ plan de la primera ejecución:");
    for (const auto &step : stats.plan)
      result.plan.push_back("   └ " + step);
  }
  result.status = "ok";
  return true;
}

// ───────────────────────────── DDL ─────────────────────────────

void Executor::createTable(CreateTableStatement &node, StatementResult &result) {
  if (storage_.catalog().table(node.table)) {
    result.message = "La tabla " + node.table + " ya existía; no se realizaron cambios.";
    return;
  }
  storage_.catalog().addTable(node.schema);
  for (const auto &index : node.automatic_indexes)
    storage_.catalog().addIndex(index);
  std::map<std::string, IndexData> indexes;
  for (const auto &index : node.automatic_indexes) {
    indexes[index.name] = IndexData{};
    result.plan.push_back("Índice automático " + index.name + " sobre " + node.table +
                          "(" + index.column + ") para validar " +
                          (node.schema.column(index.column)->primary_key ? "PRIMARY KEY"
                                                                         : "UNIQUE"));
  }
  persistTable(node.table, TableData{}, std::move(indexes), result);
  saveCatalog(result);
  result.message = "Tabla " + node.table + " creada con " +
                   plural(node.schema.columns.size(), "columna", "columnas") + ".";
}

void Executor::dropTable(DropTableStatement &node, StatementResult &result) {
  if (!storage_.catalog().table(node.table)) {
    result.message = "La tabla " + node.table + " no existía; no se realizaron cambios.";
    return;
  }
  std::string error;
  for (const IndexSchema *index : storage_.catalog().indexesOf(node.table)) {
    const std::string name = index->name;
    if (!storage_.removeIndex(name, error))
      fail("EXE008", error, {});
    result.files.push_back("indexes/" + name + ".idx (eliminado)");
  }
  if (!storage_.removeTable(node.table, error))
    fail("EXE008", error, {});
  result.files.push_back(node.table + ".csv (eliminado)");
  storage_.catalog().dropTable(node.table);
  saveCatalog(result);
  result.message = "Tabla " + node.table + " eliminada junto con sus índices.";
}

void Executor::createIndex(CreateIndexStatement &node, StatementResult &result) {
  IndexSchema schema{node.name, node.table, node.column, node.unique, false};
  const TableData &data = loadTable(node.table);
  IndexData index = storage_.buildIndex(schema, data);
  if (node.unique) {
    for (const auto &[key, rowids] : index.entries) {
      if (rowids.size() > 1)
        fail("EXE002", "No se puede crear el índice único " + node.name + ": el valor " +
                           key.toString() + " se repite en " +
                           plural(rowids.size(), "registro", "registros") + ".",
             node.column_loc);
    }
  }
  storage_.catalog().addIndex(schema);
  storage_.putIndex(node.name, std::move(index));
  std::string error;
  if (!storage_.saveIndex(node.name, error))
    fail("EXE008", error, {});
  result.files.push_back("indexes/" + node.name + ".idx");
  saveCatalog(result);
  const IndexData &saved = loadIndex(node.name);
  result.plan.push_back("Se recorrió " + node.table + " una vez (" +
                        plural(data.rows.size(), "registro", "registros") +
                        ") para construir el índice ordenado.");
  result.message = "Índice " + node.name + " creado sobre " + node.table + "(" +
                   node.column + ") con " +
                   plural(saved.entries.size(), "clave distinta", "claves distintas") + ".";
}

void Executor::dropIndex(DropIndexStatement &node, StatementResult &result) {
  if (!storage_.catalog().index(node.name)) {
    result.message = "El índice " + node.name + " no existía; no se realizaron cambios.";
    return;
  }
  std::string error;
  if (!storage_.removeIndex(node.name, error))
    fail("EXE008", error, {});
  storage_.catalog().dropIndex(node.name);
  result.files.push_back("indexes/" + node.name + ".idx (eliminado)");
  saveCatalog(result);
  result.message = "Índice " + node.name + " eliminado.";
}

// ───────────────────────── Restricciones ─────────────────────────

Row Executor::coerceRow(const TableSchema &schema, Row row, const SourceLocation &loc) {
  for (size_t index = 0; index < schema.columns.size(); ++index) {
    const ColumnSchema &column = schema.columns[index];
    std::string error;
    auto coerced = coerceToType(row[index], column.type, error);
    if (!coerced)
      fail("EXE007", "Columna " + schema.name + "." + column.name + ": " + error + ".", loc);
    if (coerced->isNull() && column.not_null)
      fail("EXE007", "La columna " + schema.name + "." + column.name +
                         " no admite NULL.",
           loc);
    row[index] = *coerced;
  }
  return row;
}

void Executor::checkUnique(const std::string &table, const Row &row, long long rowid,
                           std::map<std::string, IndexData> &indexes,
                           const SourceLocation &loc) {
  const TableSchema &schema = *storage_.catalog().table(table);
  for (auto &[name, index] : indexes) {
    const IndexSchema &index_schema = *storage_.catalog().index(name);
    if (!index_schema.unique)
      continue;
    const int column = schema.columnIndex(index_schema.column);
    const Value &key = row[column];
    if (key.isNull())
      continue;
    auto found = index.entries.find(key);
    if (found == index.entries.end())
      continue;
    if (found->second.size() == 1 && *found->second.begin() == rowid)
      continue;
    const bool primary = schema.columns[column].primary_key;
    fail(primary ? "EXE001" : "EXE002",
         std::string(primary ? "Clave primaria duplicada: " : "Valor UNIQUE duplicado: ") +
             table + "." + index_schema.column + " = " + key.toString() +
             " ya existe (detectado con el índice " + name + ").",
         loc);
  }
}

void Executor::checkForeignKeys(const TableSchema &schema, const Row &row,
                                const TableData &own_data, const SourceLocation &loc) {
  for (size_t index = 0; index < schema.columns.size(); ++index) {
    const ColumnSchema &column = schema.columns[index];
    if (column.ref_table.empty() || row[index].isNull())
      continue;
    bool exists = false;
    if (column.ref_table == schema.name) {
      // Auto-referencia: se busca en la copia de trabajo de la misma tabla.
      const int target = schema.columnIndex(column.ref_column);
      for (const auto &[rowid, other] : own_data.rows) {
        if (valuesEqual(other[target], row[index])) {
          exists = true;
          break;
        }
      }
    } else {
      const IndexSchema *target = storage_.catalog().indexOn(column.ref_table,
                                                             column.ref_column);
      if (target) {
        const IndexData &referenced = loadIndex(target->name);
        exists = referenced.entries.count(row[index]) > 0;
      }
    }
    if (!exists)
      fail("EXE003", "Violación de clave foránea: " + schema.name + "." + column.name +
                         " = " + row[index].toString() + " no existe en " +
                         column.ref_table + "." + column.ref_column + ".",
           loc);
  }
}

// Política RESTRICT: no se borra ni se cambia un valor que otra tabla usa.
void Executor::checkReferencedBy(const TableSchema &schema, const std::vector<Row> &old_rows,
                                 const std::vector<Row> &new_rows,
                                 const std::set<long long> &removed,
                                 const SourceLocation &loc) {
  for (const auto &[child_name, child] : storage_.catalog().tables()) {
    for (size_t child_column = 0; child_column < child.columns.size(); ++child_column) {
      const ColumnSchema &fk = child.columns[child_column];
      if (fk.ref_table != schema.name)
        continue;
      const int parent_column = schema.columnIndex(fk.ref_column);
      const TableData &child_data = loadTable(child_name);
      for (size_t row = 0; row < old_rows.size(); ++row) {
        const Value &old_value = old_rows[row][parent_column];
        if (old_value.isNull())
          continue;
        if (!new_rows.empty() && valuesEqual(old_value, new_rows[row][parent_column]))
          continue;
        for (const auto &[child_rowid, child_row] : child_data.rows) {
          if (child_name == schema.name && removed.count(child_rowid))
            continue;
          if (valuesEqual(child_row[child_column], old_value))
            fail("EXE004", "No se puede " +
                               std::string(new_rows.empty() ? "eliminar" : "modificar") +
                               " " + schema.name + "." + fk.ref_column + " = " +
                               old_value.toString() + ": el registro es referenciado por " +
                               child_name + "." + fk.name + ".",
                 loc);
        }
      }
    }
  }
}

// ───────────────────────────── DML ─────────────────────────────

void Executor::insert(InsertStatement &node, StatementResult &result) {
  TableSchema schema = *storage_.catalog().table(node.table);
  TableData working = loadTable(node.table); // copia de trabajo
  std::map<std::string, IndexData> indexes;
  for (const IndexSchema *index : storage_.catalog().indexesOf(node.table))
    indexes[index->name] = loadIndex(index->name);

  const Context empty;
  for (size_t row_number = 0; row_number < node.rows.size(); ++row_number) {
    const SourceLocation &loc = node.row_locs[row_number];
    Row row(schema.columns.size(), Value::makeNull());
    for (size_t index = 0; index < node.rows[row_number].size(); ++index)
      row[node.target_columns[index]] = evaluate(*node.rows[row_number][index], empty);
    row = coerceRow(schema, std::move(row), loc);
    const long long rowid = schema.next_rowid++;
    checkUnique(node.table, row, rowid, indexes, loc);
    working.rows[rowid] = row;
    checkForeignKeys(schema, row, working, loc);
    for (auto &[name, index] : indexes)
      index.add(row[schema.columnIndex(storage_.catalog().index(name)->column)], rowid);
  }

  for (const auto &[name, index] : indexes)
    result.plan.push_back("Índice " + name + " actualizado (" +
                          plural(index.size(), "entrada", "entradas") + ").");
  *storage_.catalog().table(node.table) = schema; // conserva next_rowid
  persistTable(node.table, std::move(working), std::move(indexes), result);
  saveCatalog(result);
  result.affected = static_cast<long long>(node.rows.size());
  result.message = plural(node.rows.size(), "registro insertado", "registros insertados") +
                   " en " + node.table + ".";
}

void Executor::update(UpdateStatement &node, StatementResult &result) {
  const TableSchema schema = *storage_.catalog().table(node.table);
  TableData working = loadTable(node.table);
  std::map<std::string, IndexData> indexes;
  for (const IndexSchema *index : storage_.catalog().indexesOf(node.table))
    indexes[index->name] = loadIndex(index->name);

  // 1) Localizar los registros: con índice si la condición lo permite.
  const Context empty;
  auto access = chooseIndex(node.table, 0, schema.columns.size(), node.where.get(), empty);
  std::vector<long long> candidates;
  if (access) {
    candidates.assign(access->rowids.begin(), access->rowids.end());
    result.plan.push_back("Index Scan en " + node.table + " usando " + access->index +
                          " (" + access->condition + "): " +
                          plural(candidates.size(), "registro candidato", "registros candidatos") +
                          " de " + std::to_string(working.rows.size()) + ".");
  } else {
    for (const auto &[rowid, row] : working.rows)
      candidates.push_back(rowid);
    result.plan.push_back("Full Scan en " + node.table + ": se evaluó WHERE en " +
                          plural(candidates.size(), "registro", "registros") + ".");
  }

  // 2) Calcular los valores nuevos con los valores ANTERIORES de cada fila.
  std::vector<long long> changed;
  std::vector<Row> old_rows, new_rows;
  for (long long rowid : candidates) {
    auto found = working.rows.find(rowid);
    if (found == working.rows.end())
      continue;
    const Context ctx{&found->second, nullptr, nullptr};
    if (node.where && !isTrue(evaluate(*node.where, ctx)))
      continue;
    Row row = found->second;
    for (const auto &assignment : node.assignments)
      row[assignment.column_index] = evaluate(*assignment.value, ctx);
    old_rows.push_back(found->second);
    new_rows.push_back(coerceRow(schema, std::move(row), node.assignments.front().loc));
    changed.push_back(rowid);
  }
  checkReferencedBy(schema, old_rows, new_rows, {}, node.assignments.front().loc);

  // 3) Sacar las claves viejas de los índices y validar/insertar las nuevas.
  for (size_t index = 0; index < changed.size(); ++index) {
    for (auto &[name, data] : indexes)
      data.remove(old_rows[index][schema.columnIndex(storage_.catalog().index(name)->column)],
                  changed[index]);
  }
  for (size_t index = 0; index < changed.size(); ++index) {
    checkUnique(node.table, new_rows[index], changed[index], indexes,
                node.assignments.front().loc);
    working.rows[changed[index]] = new_rows[index];
    for (auto &[name, data] : indexes)
      data.add(new_rows[index][schema.columnIndex(storage_.catalog().index(name)->column)],
               changed[index]);
  }
  for (size_t index = 0; index < changed.size(); ++index)
    checkForeignKeys(schema, new_rows[index], working, node.assignments.front().loc);

  persistTable(node.table, std::move(working), std::move(indexes), result);
  result.affected = static_cast<long long>(changed.size());
  result.message = plural(changed.size(), "registro actualizado", "registros actualizados") +
                   " en " + node.table + ".";
}

void Executor::remove(DeleteStatement &node, StatementResult &result) {
  const TableSchema schema = *storage_.catalog().table(node.table);
  TableData working = loadTable(node.table);
  std::map<std::string, IndexData> indexes;
  for (const IndexSchema *index : storage_.catalog().indexesOf(node.table))
    indexes[index->name] = loadIndex(index->name);

  const Context empty;
  auto access = chooseIndex(node.table, 0, schema.columns.size(), node.where.get(), empty);
  std::vector<long long> candidates;
  if (access) {
    candidates.assign(access->rowids.begin(), access->rowids.end());
    result.plan.push_back("Index Scan en " + node.table + " usando " + access->index +
                          " (" + access->condition + "): " +
                          plural(candidates.size(), "registro candidato", "registros candidatos") +
                          " de " + std::to_string(working.rows.size()) + ".");
  } else {
    for (const auto &[rowid, row] : working.rows)
      candidates.push_back(rowid);
    result.plan.push_back("Full Scan en " + node.table + ": se evaluó WHERE en " +
                          plural(candidates.size(), "registro", "registros") + ".");
  }

  std::set<long long> removed;
  std::vector<Row> old_rows;
  for (long long rowid : candidates) {
    auto found = working.rows.find(rowid);
    if (found == working.rows.end())
      continue;
    const Context ctx{&found->second, nullptr, nullptr};
    if (node.where && !isTrue(evaluate(*node.where, ctx)))
      continue;
    removed.insert(rowid);
    old_rows.push_back(found->second);
  }
  checkReferencedBy(schema, old_rows, {}, removed, {});
  for (size_t index = 0; index < old_rows.size(); ++index) {
    const long long rowid = *std::next(removed.begin(), static_cast<long>(index));
    for (auto &[name, data] : indexes)
      data.remove(old_rows[index][schema.columnIndex(storage_.catalog().index(name)->column)],
                  rowid);
    working.rows.erase(rowid);
  }
  persistTable(node.table, std::move(working), std::move(indexes), result);
  result.affected = static_cast<long long>(removed.size());
  result.message = plural(removed.size(), "registro eliminado", "registros eliminados") +
                   " de " + node.table + ".";
}

// ───────────────────────────── Índices ─────────────────────────────

std::optional<Executor::IndexAccess>
Executor::chooseIndex(const std::string &table, size_t offset, size_t width,
                      const Expr *condition, const Context &ctx) {
  std::vector<const Expr *> parts;
  conjuncts(condition, parts);
  const TableSchema *schema = storage_.catalog().table(table);

  // Una columna de la tabla objetivo que tenga índice.
  auto indexed = [&](const Expr &expr) -> const IndexSchema * {
    if (expr.kind != ExprKind::Column || expr.depth != 0 || expr.slot < 0)
      return nullptr;
    const size_t slot = static_cast<size_t>(expr.slot);
    if (slot < offset || slot >= offset + width)
      return nullptr;
    return storage_.catalog().indexOn(table, schema->columns[slot - offset].name);
  };

  // Prioridad: igualdad, luego IN, luego rangos.
  for (int pass = 0; pass < 3; ++pass) {
    for (const Expr *part : parts) {
      if (pass == 1 && part->kind == ExprKind::InList && !part->negated) {
        const IndexSchema *index = indexed(*part->children[0]);
        bool constant = index != nullptr;
        for (size_t item = 1; constant && item < part->children.size(); ++item)
          constant = availableBefore(*part->children[item], offset);
        if (!constant)
          continue;
        IndexAccess access{index->name, compact(part->text), {}};
        const IndexData &data = loadIndex(index->name);
        for (size_t item = 1; item < part->children.size(); ++item) {
          auto found = data.entries.find(evaluate(*part->children[item], ctx));
          if (found != data.entries.end())
            access.rowids.insert(found->second.begin(), found->second.end());
        }
        return access;
      }
      // col IN (SELECT ...) sin correlación: cada valor del resultado se busca
      // en el índice en lugar de comparar contra todos los registros.
      if (pass == 1 && part->kind == ExprKind::InSubquery && !part->negated &&
          !part->subquery->correlated) {
        const IndexSchema *index = indexed(*part->children[0]);
        if (!index)
          continue;
        IndexAccess access{index->name, compact(part->text), {}};
        const IndexData &data = loadIndex(index->name);
        for (const Row &row : subqueryRows(*part, ctx)) {
          auto found = data.entries.find(row.front());
          if (found != data.entries.end())
            access.rowids.insert(found->second.begin(), found->second.end());
        }
        return access;
      }
      if (part->kind != ExprKind::Binary)
        continue;
      std::string op = part->op;
      const bool is_equality = op == "=";
      const bool is_range = op == "<" || op == "<=" || op == ">" || op == ">=";
      if ((pass == 0 && !is_equality) || (pass == 2 && !is_range) || pass == 1)
        continue;
      const Expr *column = part->children[0].get();
      const Expr *other = part->children[1].get();
      const IndexSchema *index = indexed(*column);
      if (!index || !availableBefore(*other, offset)) {
        std::swap(column, other);
        op = flip(op);
        index = indexed(*column);
        if (!index || !availableBefore(*other, offset))
          continue;
      }
      const Value key = evaluate(*other, ctx);
      IndexAccess access{index->name, compact(part->text), {}};
      if (key.isNull())
        return access; // col = NULL nunca es verdadero
      const IndexData &data = loadIndex(index->name);
      auto first = data.entries.begin();
      auto last = data.entries.end();
      if (op == "=") {
        first = data.entries.lower_bound(key);
        last = data.entries.upper_bound(key);
      } else if (op == ">") {
        first = data.entries.upper_bound(key);
      } else if (op == ">=") {
        first = data.entries.lower_bound(key);
      } else if (op == "<") {
        last = data.entries.lower_bound(key);
      } else if (op == "<=") {
        last = data.entries.upper_bound(key);
      }
      for (auto entry = first; entry != last; ++entry)
        access.rowids.insert(entry->second.begin(), entry->second.end());
      return access;
    }
  }
  return std::nullopt;
}

// ───────────────────────────── SELECT ─────────────────────────────

void Executor::select(SelectStatement &node, StatementResult &result) {
  result.rows = runSelect(node, nullptr, &result.plan);
  for (const auto &column : node.output) {
    result.columns.push_back(column.name);
    result.column_types.push_back(column.type.toString());
  }
  result.affected = static_cast<long long>(result.rows.size());
  result.message = plural(result.rows.size(), "registro", "registros") + " en el resultado.";
}

std::vector<Row> Executor::runSelect(const SelectStatement &select, const Context *outer,
                                     std::vector<std::string> *plan) {
  const Context base_context{nullptr, nullptr, outer};

  // 1) FROM: registros de la tabla base (por índice o recorrido completo).
  std::vector<Row> combined;
  if (!select.has_from) {
    combined.emplace_back();
  } else {
    const TableSchema &schema = *storage_.catalog().table(select.from.name);
    const TableData &data = loadTable(select.from.name);
    auto access = chooseIndex(select.from.name, 0, schema.columns.size(),
                              select.where.get(), base_context);
    if (access) {
      for (long long rowid : access->rowids) {
        auto found = data.rows.find(rowid);
        if (found != data.rows.end())
          combined.push_back(found->second);
      }
      if (plan)
        plan->push_back("Index Scan en " + select.from.name + " usando " + access->index +
                        " (" + access->condition + "): se leyeron " +
                        plural(combined.size(), "registro", "registros") + " de " +
                        std::to_string(data.rows.size()) + " sin recorrer la tabla.");
    } else {
      for (const auto &[rowid, row] : data.rows)
        combined.push_back(row);
      if (plan)
        plan->push_back("Full Scan en " + select.from.name + ": se leyeron los " +
                        plural(data.rows.size(), "registro", "registros") + ".");
    }

    // 2) INNER JOIN: por cada fila acumulada se buscan las filas que cumplen ON.
    size_t offset = schema.columns.size();
    for (const auto &join : select.joins) {
      const TableSchema &right_schema = *storage_.catalog().table(join.table.name);
      const TableData &right = loadTable(join.table.name);
      std::vector<Row> next;
      size_t probes = 0, comparisons = 0;
      std::string used_index;
      for (const Row &left : combined) {
        const Context left_context{&left, nullptr, outer};
        auto access = chooseIndex(join.table.name, offset, right_schema.columns.size(),
                                  join.condition.get(), left_context);
        std::vector<const Row *> candidates;
        if (access) {
          ++probes;
          used_index = access->index + " (" + access->condition + ")";
          for (long long rowid : access->rowids) {
            auto found = right.rows.find(rowid);
            if (found != right.rows.end())
              candidates.push_back(&found->second);
          }
        } else {
          for (const auto &[rowid, row] : right.rows)
            candidates.push_back(&row);
        }
        for (const Row *candidate : candidates) {
          ++comparisons;
          Row row = left;
          row.insert(row.end(), candidate->begin(), candidate->end());
          const Context ctx{&row, nullptr, outer};
          if (isTrue(evaluate(*join.condition, ctx)))
            next.push_back(std::move(row));
        }
      }
      if (plan) {
        if (!used_index.empty())
          plan->push_back("INNER JOIN " + join.table.name + ": Index Nested Loop con " +
                          used_index + ", " + plural(probes, "búsqueda", "búsquedas") +
                          " en el índice y " + std::to_string(comparisons) +
                          " combinaciones evaluadas → " +
                          plural(next.size(), "fila", "filas") + ".");
        else
          plan->push_back("INNER JOIN " + join.table.name + ": Nested Loop, " +
                          std::to_string(combined.size()) + " × " +
                          std::to_string(right.rows.size()) + " = " +
                          std::to_string(comparisons) + " combinaciones evaluadas → " +
                          plural(next.size(), "fila", "filas") + ".");
      }
      combined = std::move(next);
      offset += right_schema.columns.size();
    }
  }

  // 3) WHERE: se evalúa completo sobre cada fila (también tras usar un índice).
  std::vector<Row> filtered;
  for (Row &row : combined) {
    const Context ctx{&row, nullptr, outer};
    if (!select.where || isTrue(evaluate(*select.where, ctx)))
      filtered.push_back(std::move(row));
  }
  if (plan && select.where)
    plan->push_back("WHERE: " + plural(filtered.size(), "fila cumple", "filas cumplen") +
                    " la condición.");

  // 4) Proyección (y agrupación si la consulta usa GROUP BY o agregados).
  struct Record {
    Row values;
    Row keys;
  };
  std::vector<Record> records;
  auto project = [&](const Context &ctx) {
    Record record;
    for (const Expr *expr : select.projection)
      record.values.push_back(evaluate(*expr, ctx));
    for (const auto &item : select.order_by)
      record.keys.push_back(item.output_index >= 0 ? record.values[item.output_index]
                                                   : evaluate(*item.expr, ctx));
    records.push_back(std::move(record));
  };

  if (select.aggregate_query) {
    std::vector<std::vector<const Row *>> groups;
    std::map<Row, size_t, RowsLess> group_of;
    if (select.group_by.empty())
      groups.emplace_back(); // un único grupo, aunque no haya filas
    for (const Row &row : filtered) {
      if (select.group_by.empty()) {
        groups.front().push_back(&row);
        continue;
      }
      const Context ctx{&row, nullptr, outer};
      Row key;
      for (const auto &expr : select.group_by)
        key.push_back(evaluate(*expr, ctx));
      auto [position, inserted] = group_of.emplace(key, groups.size());
      if (inserted)
        groups.emplace_back();
      groups[position->second].push_back(&row);
    }
    size_t kept = 0;
    for (const auto &group : groups) {
      const Context ctx{group.empty() ? nullptr : group.front(), &group, outer};
      if (select.having && !isTrue(evaluate(*select.having, ctx)))
        continue;
      ++kept;
      project(ctx);
    }
    if (plan)
      plan->push_back(select.group_by.empty()
                          ? "Agregación sobre " + plural(filtered.size(), "fila", "filas") +
                                " (un solo grupo)."
                          : "GROUP BY: " + plural(groups.size(), "grupo", "grupos") +
                                (select.having ? ", " + std::to_string(kept) +
                                                     " cumplen HAVING."
                                               : "."));
  } else {
    for (const Row &row : filtered)
      project({&row, nullptr, outer});
  }

  // 5) DISTINCT, ORDER BY y LIMIT.
  if (select.distinct) {
    std::set<Row, RowsLess> seen;
    std::vector<Record> unique;
    for (auto &record : records) {
      if (seen.insert(record.values).second)
        unique.push_back(std::move(record));
    }
    records = std::move(unique);
  }
  if (!select.order_by.empty()) {
    std::stable_sort(records.begin(), records.end(), [&](const Record &a, const Record &b) {
      for (size_t index = 0; index < select.order_by.size(); ++index) {
        const int order = compareValues(a.keys[index], b.keys[index]);
        if (order != 0)
          return select.order_by[index].descending ? order > 0 : order < 0;
      }
      return false;
    });
  }
  if (select.limit && records.size() > static_cast<size_t>(*select.limit))
    records.resize(static_cast<size_t>(*select.limit));

  std::vector<Row> rows;
  for (auto &record : records)
    rows.push_back(std::move(record.values));
  return rows;
}

std::vector<Row> Executor::subqueryRows(const Expr &expr, const Context &ctx) {
  const SelectStatement *subquery = expr.subquery.get();
  SubqueryStats &stats = subquery_stats_[subquery];
  stats.correlated = subquery->correlated;
  stats.line = subquery->loc.line;
  // Sin correlación el resultado no depende de la fila externa: se reutiliza.
  if (!subquery->correlated) {
    auto cached = subquery_cache_.find(subquery);
    if (cached != subquery_cache_.end())
      return cached->second;
  }
  std::vector<std::string> plan;
  std::vector<Row> rows = runSelect(*subquery, &ctx, stats.executions == 0 ? &plan : nullptr);
  if (stats.executions == 0)
    stats.plan = plan;
  ++stats.executions;
  if (!subquery->correlated)
    subquery_cache_[subquery] = rows;
  return rows;
}

// ───────────────────────────── Expresiones ─────────────────────────────

Value Executor::evaluate(const Expr &expr, const Context &ctx) {
  switch (expr.kind) {
  case ExprKind::Literal:
    return expr.literal;

  case ExprKind::Column: {
    const Context *scope = &ctx;
    for (int level = 0; level < expr.depth && scope; ++level)
      scope = scope->outer;
    if (!scope || !scope->row || expr.slot < 0 ||
        static_cast<size_t>(expr.slot) >= scope->row->size())
      return Value::makeNull();
    return (*scope->row)[expr.slot];
  }

  case ExprKind::Unary: {
    const Value operand = evaluate(*expr.children[0], ctx);
    if (operand.isNull())
      return operand;
    if (expr.op == "NOT")
      return boolValue(!operand.boolean);
    if (expr.op == "-")
      return operand.kind == Value::Kind::Int ? Value::makeInt(-operand.integer)
                                              : Value::makeFloat(-operand.real);
    return operand;
  }

  case ExprKind::Binary:
    return evaluateBinary(expr, ctx);

  case ExprKind::IsNull: {
    const bool is_null = evaluate(*expr.children[0], ctx).isNull();
    return boolValue(expr.negated ? !is_null : is_null);
  }

  case ExprKind::InList:
  case ExprKind::InSubquery: {
    // Lógica de tres valores: sin coincidencia pero con NULL en la lista -> NULL.
    const Value operand = evaluate(*expr.children[0], ctx);
    if (operand.isNull())
      return Value::makeNull();
    std::vector<Value> values;
    if (expr.kind == ExprKind::InList) {
      for (size_t index = 1; index < expr.children.size(); ++index)
        values.push_back(evaluate(*expr.children[index], ctx));
    } else {
      for (const Row &row : subqueryRows(expr, ctx))
        values.push_back(row.front());
    }
    bool saw_null = false;
    for (const Value &value : values) {
      if (value.isNull()) {
        saw_null = true;
      } else if (valuesEqual(operand, value)) {
        return boolValue(!expr.negated);
      }
    }
    if (saw_null)
      return Value::makeNull();
    return boolValue(expr.negated);
  }

  case ExprKind::Exists:
    return boolValue(!subqueryRows(expr, ctx).empty());

  case ExprKind::ScalarSubquery: {
    const auto rows = subqueryRows(expr, ctx);
    if (rows.size() > 1)
      fail("EXE005", "La subconsulta escalar '" + expr.text + "' retornó " +
                         std::to_string(rows.size()) +
                         " registros; solo puede retornar uno.",
           expr.loc);
    return rows.empty() ? Value::makeNull() : rows.front().front();
  }

  case ExprKind::Aggregate:
    return evaluateAggregate(expr, ctx);
  }
  return Value::makeNull();
}

Value Executor::evaluateBinary(const Expr &expr, const Context &ctx) {
  const std::string &op = expr.op;
  // AND / OR con cortocircuito y lógica de tres valores (TRUE, FALSE, NULL).
  if (op == "AND" || op == "OR") {
    const Value left = evaluate(*expr.children[0], ctx);
    if (op == "AND" && !left.isNull() && !left.boolean)
      return boolValue(false);
    if (op == "OR" && !left.isNull() && left.boolean)
      return boolValue(true);
    const Value right = evaluate(*expr.children[1], ctx);
    if (op == "AND") {
      if (!right.isNull() && !right.boolean)
        return boolValue(false);
      if (left.isNull() || right.isNull())
        return Value::makeNull();
      return boolValue(true);
    }
    if (!right.isNull() && right.boolean)
      return boolValue(true);
    if (left.isNull() || right.isNull())
      return Value::makeNull();
    return boolValue(false);
  }

  const Value left = evaluate(*expr.children[0], ctx);
  const Value right = evaluate(*expr.children[1], ctx);
  if (left.isNull() || right.isNull())
    return Value::makeNull();

  if (op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=") {
    const int order = compareValues(left, right);
    if (op == "=")
      return boolValue(order == 0);
    if (op == "<>")
      return boolValue(order != 0);
    if (op == "<")
      return boolValue(order < 0);
    if (op == "<=")
      return boolValue(order <= 0);
    if (op == ">")
      return boolValue(order > 0);
    return boolValue(order >= 0);
  }

  const bool integers = left.kind == Value::Kind::Int && right.kind == Value::Kind::Int;
  if ((op == "/" || op == "%") && right.asDouble() == 0.0)
    fail("EXE006", "División entre cero en '" + expr.text + "'.", expr.loc);
  if (integers) {
    const long long a = left.integer, b = right.integer;
    if (op == "+")
      return Value::makeInt(a + b);
    if (op == "-")
      return Value::makeInt(a - b);
    if (op == "*")
      return Value::makeInt(a * b);
    if (op == "/")
      return Value::makeInt(a / b); // división entera, como en PostgreSQL
    return Value::makeInt(a % b);
  }
  const double a = left.asDouble(), b = right.asDouble();
  if (op == "+")
    return Value::makeFloat(a + b);
  if (op == "-")
    return Value::makeFloat(a - b);
  if (op == "*")
    return Value::makeFloat(a * b);
  if (op == "/")
    return Value::makeFloat(a / b);
  return Value::makeFloat(std::fmod(a, b));
}

Value Executor::evaluateAggregate(const Expr &expr, const Context &ctx) {
  static const std::vector<const Row *> no_rows;
  const std::vector<const Row *> &group = ctx.group ? *ctx.group : no_rows;
  if (expr.star)
    return Value::makeInt(static_cast<long long>(group.size()));

  // Valores del argumento en cada fila del grupo, ignorando NULL.
  std::vector<Value> values;
  std::set<Value, ValueLess> distinct;
  for (const Row *row : group) {
    const Context row_context{row, nullptr, ctx.outer};
    Value value = evaluate(*expr.children[0], row_context);
    if (value.isNull())
      continue;
    if (expr.distinct && !distinct.insert(value).second)
      continue;
    values.push_back(std::move(value));
  }

  if (expr.function == "COUNT")
    return Value::makeInt(static_cast<long long>(values.size()));
  if (values.empty())
    return Value::makeNull();
  if (expr.function == "MIN" || expr.function == "MAX") {
    Value best = values.front();
    for (const Value &value : values) {
      const int order = compareValues(value, best);
      if ((expr.function == "MIN" && order < 0) || (expr.function == "MAX" && order > 0))
        best = value;
    }
    return best;
  }
  bool all_integers = true;
  long long integer_sum = 0;
  double real_sum = 0.0;
  for (const Value &value : values) {
    all_integers = all_integers && value.kind == Value::Kind::Int;
    integer_sum += value.kind == Value::Kind::Int ? value.integer : 0;
    real_sum += value.asDouble();
  }
  if (expr.function == "SUM")
    return all_integers ? Value::makeInt(integer_sum) : Value::makeFloat(real_sum);
  return Value::makeFloat(real_sum / static_cast<double>(values.size())); // AVG
}

} // namespace sql
