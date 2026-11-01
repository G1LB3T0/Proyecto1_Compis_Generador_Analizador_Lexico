// Reglas semánticas del intérprete SQL. Cada error se reporta con un código
// estable (SEM0xx), la sentencia, la línea y la columna donde ocurre.
//
// SEM001 tabla inexistente           SEM011 columna duplicada
// SEM002 columna inexistente         SEM012 restricción inválida
// SEM003 referencia ambigua          SEM013 índice inexistente
// SEM004 alias/tabla inválido        SEM014 índice duplicado
// SEM005 tipos incompatibles         SEM015 subconsulta con >1 columna
// SEM006 cantidad de valores INSERT  SEM016 uso inválido de agregación
// SEM007 NULL en columna NOT NULL    SEM017 tabla referenciada por FK
// SEM008 texto excede VARCHAR(n)     SEM018 literal fuera de rango
// SEM009 fecha inválida              SEM019 condición no booleana
// SEM010 tabla ya existe             SEM020 ORDER BY inválido
#include "sql/semantic_analyzer.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace sql {

namespace {

std::string normalized(const std::string &text) {
  std::string result;
  for (unsigned char character : text) {
    if (!std::isspace(character))
      result += static_cast<char>(std::tolower(character));
  }
  return result;
}

bool isText(const DataType &type) { return type.base == BaseType::Varchar; }

bool containsAggregate(const Expr *expr) {
  if (!expr)
    return false;
  if (expr->kind == ExprKind::Aggregate)
    return true;
  for (const auto &child : expr->children) {
    if (containsAggregate(child.get()))
      return true;
  }
  return false; // las subconsultas tienen su propio nivel de agregación
}

// Convierte un literal de texto en DATE cuando se compara o asigna a fechas.
bool literalToDate(Expr &expr) {
  if (expr.kind != ExprKind::Literal || expr.literal.kind != Value::Kind::Text)
    return false;
  if (!isValidDate(expr.literal.text))
    return false;
  expr.literal = Value::makeDate(expr.literal.text);
  expr.type = DataType::date();
  return true;
}

bool isTextLiteral(const Expr &expr) {
  return expr.kind == ExprKind::Literal && expr.literal.kind == Value::Kind::Text;
}

} // namespace

bool typesComparable(const DataType &left, const DataType &right) {
  if (left.isNull() || right.isNull() || left.isUnknown() || right.isUnknown())
    return true;
  if (left.isNumeric() && right.isNumeric())
    return true;
  return left.base == right.base;
}

SemanticAnalyzer::SemanticAnalyzer(Catalog catalog) : catalog_(std::move(catalog)) {}

void SemanticAnalyzer::error(const std::string &code, const std::string &message,
                             const SourceLocation &loc) {
  diagnostics_.push_back({code, "semántico", "error", message, loc.line,
                          loc.column, statement_number_});
}

std::vector<Diagnostic> SemanticAnalyzer::analyze(Script &script) {
  diagnostics_.clear();
  for (size_t index = 0; index < script.statements.size(); ++index) {
    statement_number_ = static_cast<int>(index) + 1;
    analyzeStatement(script.statements[index]);
  }
  return diagnostics_;
}

void SemanticAnalyzer::analyzeStatement(Statement &statement) {
  scopes_.clear();
  aggregate_depth_ = 0;
  switch (statement.kind) {
  case StatementKind::CreateTable:
    analyzeCreateTable(std::get<CreateTableStatement>(statement.node), statement);
    break;
  case StatementKind::DropTable:
    analyzeDropTable(std::get<DropTableStatement>(statement.node), statement);
    break;
  case StatementKind::CreateIndex:
    analyzeCreateIndex(std::get<CreateIndexStatement>(statement.node), statement);
    break;
  case StatementKind::DropIndex:
    analyzeDropIndex(std::get<DropIndexStatement>(statement.node), statement);
    break;
  case StatementKind::Insert:
    analyzeInsert(std::get<InsertStatement>(statement.node), statement);
    break;
  case StatementKind::Update:
    analyzeUpdate(std::get<UpdateStatement>(statement.node), statement);
    break;
  case StatementKind::Delete:
    analyzeDelete(std::get<DeleteStatement>(statement.node), statement);
    break;
  case StatementKind::Select:
    analyzeSelect(*std::get<std::shared_ptr<SelectStatement>>(statement.node));
    break;
  }
}

// ───────────────────────────── DDL ─────────────────────────────

void SemanticAnalyzer::analyzeCreateTable(CreateTableStatement &node,
                                          const Statement &statement) {
  if (catalog_.table(node.table)) {
    if (!node.if_not_exists)
      error("SEM010", "La tabla " + node.table + " ya existe.", statement.loc);
    return;
  }
  const size_t errors_before = diagnostics_.size();
  TableSchema schema;
  schema.name = node.table;
  for (const auto &definition : node.columns) {
    if (schema.column(definition.name)) {
      error("SEM011", "La columna " + definition.name +
                          " está declarada más de una vez en " + node.table + ".",
            definition.loc);
      continue;
    }
    if (definition.type.base == BaseType::Varchar && definition.type.length <= 0)
      error("SEM012", "VARCHAR(" + std::to_string(definition.type.length) +
                          ") no es válido: la longitud debe ser mayor que 0.",
            definition.loc);
    ColumnSchema column;
    column.name = definition.name;
    column.type = definition.type;
    column.primary_key = definition.primary_key;
    column.not_null = definition.not_null || definition.primary_key;
    column.unique = definition.unique;
    column.ref_table = definition.ref_table;
    column.ref_column = definition.ref_column;
    schema.columns.push_back(column);
  }

  // Las restricciones de tabla se aplican sobre las columnas ya declaradas.
  for (const auto &constraint : node.constraints) {
    const int index = schema.columnIndex(constraint.column);
    if (index < 0) {
      error("SEM002", "La restricción " + constraint.kind +
                          " usa la columna inexistente " + constraint.column + ".",
            constraint.loc);
      continue;
    }
    ColumnSchema &column = schema.columns[index];
    if (constraint.kind == "PRIMARY KEY") {
      column.primary_key = true;
      column.not_null = true;
    } else if (constraint.kind == "UNIQUE") {
      column.unique = true;
    } else {
      column.ref_table = constraint.ref_table;
      column.ref_column = constraint.ref_column;
    }
  }

  const auto primary_keys =
      std::count_if(schema.columns.begin(), schema.columns.end(),
                    [](const ColumnSchema &column) { return column.primary_key; });
  if (primary_keys > 1)
    error("SEM012", "La tabla " + node.table +
                        " declara más de una PRIMARY KEY; solo se permite una columna.",
          statement.loc);

  // Claves foráneas: la columna destino debe existir y ser PRIMARY KEY o UNIQUE.
  for (const auto &column : schema.columns) {
    if (column.ref_table.empty())
      continue;
    const TableSchema *target =
        column.ref_table == node.table ? &schema : catalog_.table(column.ref_table);
    if (!target) {
      error("SEM001", "La clave foránea " + node.table + "." + column.name +
                          " referencia la tabla inexistente " + column.ref_table + ".",
            statement.loc);
      continue;
    }
    const ColumnSchema *referenced = target->column(column.ref_column);
    if (!referenced) {
      error("SEM002", "La clave foránea " + node.table + "." + column.name +
                          " referencia la columna inexistente " + column.ref_table +
                          "." + column.ref_column + ".",
            statement.loc);
      continue;
    }
    if (!referenced->primary_key && !referenced->unique)
      error("SEM012", "La clave foránea " + node.table + "." + column.name +
                          " debe referenciar una columna PRIMARY KEY o UNIQUE; " +
                          column.ref_table + "." + column.ref_column + " no lo es.",
            statement.loc);
    if (!typesComparable(column.type, referenced->type))
      error("SEM005", "La clave foránea " + node.table + "." + column.name + " (" +
                          column.type.toString() + ") no es compatible con " +
                          column.ref_table + "." + column.ref_column + " (" +
                          referenced->type.toString() + ").",
            statement.loc);
  }

  // Índices automáticos: aceleran PRIMARY KEY, UNIQUE y la validación de FK.
  std::vector<IndexSchema> automatic;
  for (const auto &column : schema.columns) {
    if (!column.primary_key && !column.unique)
      continue;
    IndexSchema index;
    index.name = column.primary_key ? "pk_" + node.table
                                    : "uq_" + node.table + "_" + column.name;
    index.table = node.table;
    index.column = column.name;
    index.unique = true;
    index.automatic = true;
    if (catalog_.index(index.name)) {
      error("SEM014", "Ya existe un índice llamado " + index.name +
                          "; no se puede crear el índice automático de " +
                          node.table + "." + column.name + ".",
            statement.loc);
      continue;
    }
    automatic.push_back(index);
  }

  if (diagnostics_.size() != errors_before)
    return;
  node.schema = schema;
  node.automatic_indexes = automatic;
  catalog_.addTable(schema);
  for (const auto &index : automatic)
    catalog_.addIndex(index);
}

void SemanticAnalyzer::analyzeDropTable(DropTableStatement &node,
                                        const Statement &statement) {
  if (!catalog_.table(node.table)) {
    if (!node.if_exists)
      error("SEM001", "La tabla " + node.table + " no existe.", statement.loc);
    return;
  }
  const auto referencing = catalog_.referencingTables(node.table);
  if (!referencing.empty()) {
    error("SEM017", "No se puede eliminar " + node.table +
                        ": la tabla " + referencing.front() +
                        " tiene una clave foránea que la referencia.",
          statement.loc);
    return;
  }
  catalog_.dropTable(node.table);
}

void SemanticAnalyzer::analyzeCreateIndex(CreateIndexStatement &node,
                                          const Statement &statement) {
  if (catalog_.index(node.name)) {
    error("SEM014", "El índice " + node.name + " ya existe.", statement.loc);
    return;
  }
  const TableSchema *table = catalog_.table(node.table);
  if (!table) {
    error("SEM001", "La tabla " + node.table + " no existe.", node.table_loc);
    return;
  }
  if (!table->column(node.column)) {
    error("SEM002", "La columna " + node.column + " no existe en la tabla " +
                        node.table + ".",
          node.column_loc);
    return;
  }
  if (const IndexSchema *existing = catalog_.indexOn(node.table, node.column)) {
    error("SEM014", "La columna " + node.table + "." + node.column +
                        " ya está indexada por el índice " + existing->name +
                        (existing->automatic ? " (automático)." : "."),
          node.column_loc);
    return;
  }
  catalog_.addIndex({node.name, node.table, node.column, node.unique, false});
}

void SemanticAnalyzer::analyzeDropIndex(DropIndexStatement &node,
                                        const Statement &statement) {
  const IndexSchema *index = catalog_.index(node.name);
  if (!index) {
    if (!node.if_exists)
      error("SEM013", "El índice " + node.name + " no existe.", statement.loc);
    return;
  }
  if (index->automatic) {
    error("SEM012", "El índice " + node.name +
                        " lo creó una restricción PRIMARY KEY/UNIQUE y no puede eliminarse.",
          statement.loc);
    return;
  }
  catalog_.dropIndex(node.name);
}

// ───────────────────────────── DML ─────────────────────────────

void SemanticAnalyzer::checkAssignable(Expr &value, const ColumnSchema &column) {
  const DataType &type = value.type;
  const std::string target = "la columna " + column.name + " (" +
                             column.type.toString() + ")";
  if (type.isUnknown())
    return;
  if (type.isNull()) {
    if (column.not_null)
      error("SEM007", "No se puede asignar NULL a " + target +
                          (column.primary_key ? ", que es PRIMARY KEY." : ", que es NOT NULL."),
            value.loc);
    return;
  }
  switch (column.type.base) {
  case BaseType::Int:
    if (type.base == BaseType::Int)
      return;
    break;
  case BaseType::Float:
    if (type.isNumeric())
      return;
    break;
  case BaseType::Varchar:
    if (isText(type)) {
      if (isTextLiteral(value) &&
          characterCount(value.literal.text) > static_cast<size_t>(column.type.length))
        error("SEM008", "El texto '" + value.literal.text + "' tiene " +
                            std::to_string(characterCount(value.literal.text)) +
                            " caracteres y excede " + target + ".",
              value.loc);
      return;
    }
    break;
  case BaseType::Boolean:
    if (type.base == BaseType::Boolean)
      return;
    break;
  case BaseType::Date:
    if (type.base == BaseType::Date)
      return;
    if (isTextLiteral(value)) {
      if (!literalToDate(value))
        error("SEM009", "'" + value.literal.text +
                            "' no es una fecha válida (YYYY-MM-DD) para " + target + ".",
              value.loc);
      return;
    }
    break;
  default:
    return;
  }
  error("SEM005", "Tipos incompatibles: no se puede asignar " + type.toString() +
                      " a " + target + ".",
        value.loc);
}

void SemanticAnalyzer::analyzeInsert(InsertStatement &node, const Statement &statement) {
  const TableSchema *table = catalog_.table(node.table);
  if (!table) {
    error("SEM001", "La tabla " + node.table + " no existe.", statement.loc);
    return;
  }
  node.target_columns.clear();
  if (node.columns.empty()) {
    for (size_t index = 0; index < table->columns.size(); ++index)
      node.target_columns.push_back(static_cast<int>(index));
  } else {
    std::set<std::string> seen;
    for (size_t index = 0; index < node.columns.size(); ++index) {
      const std::string &name = node.columns[index];
      const int column = table->columnIndex(name);
      if (column < 0)
        error("SEM002", "La columna " + name + " no existe en la tabla " +
                            node.table + ".",
              node.column_locs[index]);
      else if (!seen.insert(name).second)
        error("SEM011", "La columna " + name + " aparece dos veces en el INSERT.",
              node.column_locs[index]);
      node.target_columns.push_back(column);
    }
    // Columnas omitidas: reciben NULL, lo cual viola NOT NULL / PRIMARY KEY.
    for (const auto &column : table->columns) {
      if (column.not_null && !seen.count(column.name))
        error("SEM007", "La columna " + column.name + " es " +
                            (column.primary_key ? "PRIMARY KEY" : "NOT NULL") +
                            " y el INSERT no le asigna valor.",
              statement.loc);
    }
  }

  const Clause clause{"VALUES", false};
  for (size_t row = 0; row < node.rows.size(); ++row) {
    auto &values = node.rows[row];
    if (values.size() != node.target_columns.size()) {
      error("SEM006", "La fila " + std::to_string(row + 1) + " tiene " +
                          std::to_string(values.size()) + " valor(es) pero se esperaban " +
                          std::to_string(node.target_columns.size()) + ".",
            node.row_locs[row]);
      continue;
    }
    for (size_t index = 0; index < values.size(); ++index) {
      check(*values[index], clause);
      const int column = node.target_columns[index];
      if (column >= 0)
        checkAssignable(*values[index], table->columns[column]);
    }
  }
}

void SemanticAnalyzer::analyzeUpdate(UpdateStatement &node, const Statement &statement) {
  const TableSchema *table = catalog_.table(node.table);
  if (!table) {
    error("SEM001", "La tabla " + node.table + " no existe.", statement.loc);
    return;
  }
  Scope scope;
  addTableToScope(scope, {node.table, node.table, statement.loc});
  scopes_.push_back(&scope);
  std::set<std::string> seen;
  const Clause clause{"SET", false};
  for (auto &assignment : node.assignments) {
    assignment.column_index = table->columnIndex(assignment.column);
    check(*assignment.value, clause);
    if (assignment.column_index < 0) {
      error("SEM002", "La columna " + assignment.column + " no existe en la tabla " +
                          node.table + ".",
            assignment.loc);
      continue;
    }
    if (!seen.insert(assignment.column).second)
      error("SEM011", "La columna " + assignment.column +
                          " se asigna más de una vez en el UPDATE.",
            assignment.loc);
    checkAssignable(*assignment.value, table->columns[assignment.column_index]);
  }
  checkCondition(node.where.get(), {"WHERE", false});
  scopes_.pop_back();
}

void SemanticAnalyzer::analyzeDelete(DeleteStatement &node, const Statement &statement) {
  if (!catalog_.table(node.table)) {
    error("SEM001", "La tabla " + node.table + " no existe.", statement.loc);
    return;
  }
  Scope scope;
  addTableToScope(scope, {node.table, node.table, statement.loc});
  scopes_.push_back(&scope);
  checkCondition(node.where.get(), {"WHERE", false});
  scopes_.pop_back();
}

// ───────────────────────────── SELECT ─────────────────────────────

bool SemanticAnalyzer::addTableToScope(Scope &scope, const TableReference &reference) {
  if (std::find(scope.layout.aliases.begin(), scope.layout.aliases.end(),
                reference.alias) != scope.layout.aliases.end()) {
    error("SEM004", "El nombre o alias " + reference.alias +
                        " se usa dos veces en FROM/JOIN; usa alias distintos.",
          reference.loc);
    scope.incomplete = true;
    return false;
  }
  const TableSchema *table = catalog_.table(reference.name);
  if (!table) {
    error("SEM001", "La tabla " + reference.name + " no existe.", reference.loc);
    scope.incomplete = true;
    return false;
  }
  scope.layout.aliases.push_back(reference.alias);
  scope.layout.offsets.push_back(scope.layout.columns.size());
  for (const auto &column : table->columns)
    scope.layout.columns.push_back({reference.alias, table->name, column.name, column.type});
  return true;
}

void SemanticAnalyzer::analyzeSelect(SelectStatement &select) {
  Scope scope;
  scope.select = &select;
  scopes_.push_back(&scope);
  const int saved_aggregate_depth = aggregate_depth_;
  aggregate_depth_ = 0;

  // FROM y cada JOIN amplían la fila combinada; el ON solo ve las tablas
  // que ya fueron unidas hasta ese punto.
  if (select.has_from) {
    addTableToScope(scope, select.from);
    for (auto &join : select.joins) {
      addTableToScope(scope, join.table);
      checkCondition(join.condition.get(), {"ON", false});
    }
  }
  checkCondition(select.where.get(), {"WHERE", false});
  for (auto &expr : select.group_by)
    check(*expr, {"GROUP BY", false});

  // HAVING crea un grupo implícito aunque no haya funciones de agregación.
  select.aggregate_query = !select.group_by.empty() || select.having != nullptr;
  for (const auto &item : select.items)
    select.aggregate_query = select.aggregate_query || containsAggregate(item.expr.get());
  for (const auto &item : select.order_by)
    select.aggregate_query = select.aggregate_query || containsAggregate(item.expr.get());

  // Proyección: * y tabla.* se expanden a columnas concretas.
  select.expanded.clear();
  select.projection.clear();
  select.output.clear();
  const Clause select_clause{"SELECT", true};
  for (auto &item : select.items) {
    if (!item.star) {
      check(*item.expr, select_clause);
      select.projection.push_back(item.expr.get());
      std::string name = item.alias;
      if (name.empty())
        name = item.expr->kind == ExprKind::Column ? item.expr->column : item.expr->text;
      select.output.push_back({name, item.expr->type});
      continue;
    }
    if (!select.has_from) {
      error("SEM004", "SELECT * requiere una cláusula FROM.", item.loc);
      continue;
    }
    if (!item.star_table.empty() &&
        std::find(scope.layout.aliases.begin(), scope.layout.aliases.end(),
                  item.star_table) == scope.layout.aliases.end()) {
      if (!scope.incomplete)
        error("SEM004", "La tabla o alias " + item.star_table +
                            " no aparece en FROM/JOIN.",
              item.loc);
      continue;
    }
    for (size_t slot = 0; slot < scope.layout.columns.size(); ++slot) {
      const BoundColumn &bound = scope.layout.columns[slot];
      if (!item.star_table.empty() && bound.alias != item.star_table)
        continue;
      auto expr = std::make_unique<Expr>();
      expr->kind = ExprKind::Column;
      expr->loc = item.loc;
      expr->table = bound.alias;
      expr->column = bound.column;
      expr->text = bound.alias + "." + bound.column;
      expr->type = bound.type;
      expr->slot = static_cast<int>(slot);
      select.projection.push_back(expr.get());
      select.output.push_back({bound.column, bound.type});
      select.expanded.push_back(std::move(expr));
    }
  }

  if (select.having)
    checkCondition(select.having.get(), {"HAVING", true});

  // ORDER BY acepta alias del SELECT, posiciones (1, 2...) o expresiones.
  for (auto &item : select.order_by) {
    Expr &expr = *item.expr;
    item.output_index = -1;
    if (expr.kind == ExprKind::Column && expr.table.empty()) {
      for (size_t index = 0; index < select.items.size(); ++index) {
        if (!select.items[index].alias.empty() &&
            select.items[index].alias == expr.column) {
          // Los * se expanden antes, por eso se busca la posición proyectada.
          const Expr *target = select.items[index].expr.get();
          for (size_t output = 0; output < select.projection.size(); ++output) {
            if (select.projection[output] == target)
              item.output_index = static_cast<int>(output);
          }
        }
      }
    }
    if (item.output_index < 0 && expr.kind == ExprKind::Literal &&
        expr.literal.kind == Value::Kind::Int) {
      const long long position = expr.literal.integer;
      if (position < 1 || position > static_cast<long long>(select.projection.size()))
        error("SEM020", "ORDER BY " + std::to_string(position) +
                            " está fuera de rango: el SELECT tiene " +
                            std::to_string(select.projection.size()) + " columna(s).",
              expr.loc);
      else
        item.output_index = static_cast<int>(position - 1);
      expr.type = DataType::integer();
      continue;
    }
    if (item.output_index < 0)
      check(expr, {"ORDER BY", true});
  }

  // En consultas agregadas cada columna fuera de una función debe estar en GROUP BY.
  if (select.aggregate_query && !scope.incomplete) {
    for (const Expr *expr : select.projection)
      checkGrouped(*expr, select);
    if (select.having)
      checkGrouped(*select.having, select);
    for (const auto &item : select.order_by) {
      if (item.output_index < 0)
        checkGrouped(*item.expr, select);
    }
  }

  aggregate_depth_ = saved_aggregate_depth;
  scopes_.pop_back();
}

void SemanticAnalyzer::checkGrouped(const Expr &expr, const SelectStatement &select) {
  for (const auto &group : select.group_by) {
    if (normalized(group->text) == normalized(expr.text))
      return;
    if (expr.kind == ExprKind::Column && group->kind == ExprKind::Column &&
        expr.depth == 0 && group->depth == 0 && expr.slot >= 0 &&
        expr.slot == group->slot)
      return;
  }
  switch (expr.kind) {
  case ExprKind::Aggregate:
  case ExprKind::Literal:
  case ExprKind::Exists:
  case ExprKind::ScalarSubquery:
    return;
  case ExprKind::Column:
    if (expr.depth == 0 && expr.slot >= 0)
      error("SEM016", "La columna " + expr.text +
                          " debe aparecer en GROUP BY o dentro de una función de agregación.",
            expr.loc);
    return;
  default:
    for (const auto &child : expr.children)
      checkGrouped(*child, select);
  }
}

// ───────────────────────────── Expresiones ─────────────────────────────

void SemanticAnalyzer::checkCondition(Expr *expr, const Clause &clause) {
  if (!expr)
    return;
  const DataType type = check(*expr, clause);
  if (type.base != BaseType::Boolean && !type.isNull() && !type.isUnknown())
    error("SEM019", "La condición de " + clause.name +
                        " debe ser BOOLEAN; la expresión '" + expr->text +
                        "' es " + type.toString() + ".",
          expr->loc);
}

void SemanticAnalyzer::resolveColumn(Expr &expr) {
  bool alias_found = false;
  bool silent = false;
  for (size_t depth = 0; depth < scopes_.size(); ++depth) {
    Scope &scope = *scopes_[scopes_.size() - 1 - depth];
    silent = silent || scope.incomplete;
    const RowLayout &layout = scope.layout;
    std::vector<size_t> matches;
    if (!expr.table.empty()) {
      if (std::find(layout.aliases.begin(), layout.aliases.end(), expr.table) ==
          layout.aliases.end())
        continue;
      alias_found = true;
      for (size_t slot = 0; slot < layout.columns.size(); ++slot) {
        if (layout.columns[slot].alias == expr.table &&
            layout.columns[slot].column == expr.column)
          matches.push_back(slot);
      }
      if (matches.empty()) {
        error("SEM002", "La columna " + expr.column + " no existe en " + expr.table +
                            ".",
              expr.loc);
        expr.type = DataType::unknown();
        return;
      }
    } else {
      for (size_t slot = 0; slot < layout.columns.size(); ++slot) {
        if (layout.columns[slot].column == expr.column)
          matches.push_back(slot);
      }
      if (matches.size() > 1) {
        std::string owners;
        for (size_t slot : matches)
          owners += (owners.empty() ? "" : " y ") + layout.columns[slot].alias;
        error("SEM003", "La referencia a " + expr.column + " es ambigua: existe en " +
                            owners + ". Califícala como tabla.columna.",
              expr.loc);
        expr.type = DataType::unknown();
        return;
      }
      if (matches.empty())
        continue;
    }
    expr.slot = static_cast<int>(matches.front());
    expr.depth = static_cast<int>(depth);
    expr.type = layout.columns[expr.slot].type;
    // Una columna de una consulta externa vuelve correlacionadas a las
    // subconsultas intermedias: deben reevaluarse por cada fila externa.
    for (size_t level = 0; level < depth; ++level) {
      Scope *inner = scopes_[scopes_.size() - 1 - level];
      if (inner->select)
        inner->select->correlated = true;
    }
    return;
  }
  expr.type = DataType::unknown();
  if (silent)
    return;
  if (!expr.table.empty() && !alias_found)
    error("SEM004", "La tabla o alias " + expr.table +
                        " no aparece en FROM/JOIN de la consulta.",
          expr.loc);
  else if (scopes_.empty())
    error("SEM002", "La columna " + expr.column +
                        " no puede usarse aquí: VALUES solo acepta valores.",
          expr.loc);
  else
    error("SEM002", "La columna " + expr.column +
                        " no existe en las tablas de la consulta.",
          expr.loc);
}

bool SemanticAnalyzer::comparable(Expr &left, Expr &right, const std::string &context,
                                  const SourceLocation &loc) {
  // 'YYYY-MM-DD' frente a una columna DATE se interpreta como fecha.
  if (left.type.base == BaseType::Date && isTextLiteral(right)) {
    if (!literalToDate(right)) {
      error("SEM009", "'" + right.literal.text +
                          "' no es una fecha válida con formato YYYY-MM-DD.",
            right.loc);
      return false;
    }
  }
  if (right.type.base == BaseType::Date && isTextLiteral(left)) {
    if (!literalToDate(left)) {
      error("SEM009", "'" + left.literal.text +
                          "' no es una fecha válida con formato YYYY-MM-DD.",
            left.loc);
      return false;
    }
  }
  if (typesComparable(left.type, right.type))
    return true;
  error("SEM005", "Tipos incompatibles en " + context + ": " +
                      left.type.toString() + " ('" + left.text + "') y " +
                      right.type.toString() + " ('" + right.text + "').",
        loc);
  return false;
}

DataType SemanticAnalyzer::checkBinary(Expr &expr, const Clause &clause) {
  Expr &left = *expr.children[0];
  Expr &right = *expr.children[1];
  const DataType left_type = check(left, clause);
  const DataType right_type = check(right, clause);
  const std::string &op = expr.op;
  const bool unknown = left_type.isUnknown() || right_type.isUnknown();

  if (op == "AND" || op == "OR") {
    for (const Expr *operand : {&left, &right}) {
      const DataType &type = operand->type;
      if (type.base != BaseType::Boolean && !type.isNull() && !type.isUnknown())
        error("SEM005", "El operador " + op + " requiere operandos BOOLEAN; '" +
                            operand->text + "' es " + type.toString() + ".",
              operand->loc);
    }
    return DataType::boolean();
  }

  if (op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=") {
    comparable(left, right, "la comparación '" + expr.text + "'", expr.loc);
    return DataType::boolean();
  }

  // Aritmética: + - * / %
  if (unknown)
    return DataType::unknown();
  for (const Expr *operand : {&left, &right}) {
    if (!operand->type.isNumeric() && !operand->type.isNull()) {
      error("SEM005", "El operador '" + op + "' requiere operandos numéricos; '" +
                          operand->text + "' es " + operand->type.toString() + ".",
            operand->loc);
      return DataType::unknown();
    }
  }
  if (op == "%" && (left_type.base == BaseType::Float || right_type.base == BaseType::Float)) {
    error("SEM005", "El operador % solo acepta operandos INT.", expr.loc);
    return DataType::unknown();
  }
  if (left_type.base == BaseType::Float || right_type.base == BaseType::Float)
    return DataType::real();
  if (left_type.isNull() && right_type.isNull())
    return DataType::null();
  return DataType::integer();
}

DataType SemanticAnalyzer::check(Expr &expr, const Clause &clause) {
  switch (expr.kind) {
  case ExprKind::Literal:
    switch (expr.literal.kind) {
    case Value::Kind::Null:
      expr.type = DataType::null();
      break;
    case Value::Kind::Int:
      expr.type = DataType::integer();
      break;
    case Value::Kind::Float:
      expr.type = DataType::real();
      break;
    case Value::Kind::Text:
      expr.type = DataType::varchar(static_cast<int>(characterCount(expr.literal.text)));
      break;
    case Value::Kind::Boolean:
      expr.type = DataType::boolean();
      break;
    case Value::Kind::Date:
      expr.type = DataType::date();
      break;
    }
    break;

  case ExprKind::Column:
    resolveColumn(expr);
    break;

  case ExprKind::Unary: {
    const DataType operand = check(*expr.children[0], clause);
    if (expr.op == "NOT") {
      if (operand.base != BaseType::Boolean && !operand.isNull() && !operand.isUnknown())
        error("SEM005", "NOT requiere un operando BOOLEAN; '" +
                            expr.children[0]->text + "' es " + operand.toString() + ".",
              expr.loc);
      expr.type = DataType::boolean();
    } else {
      if (!operand.isNumeric() && !operand.isNull() && !operand.isUnknown())
        error("SEM005", "El signo " + expr.op + " requiere un operando numérico; '" +
                            expr.children[0]->text + "' es " + operand.toString() + ".",
              expr.loc);
      expr.type = operand.isNumeric() ? operand : DataType::unknown();
    }
    break;
  }

  case ExprKind::Binary:
    expr.type = checkBinary(expr, clause);
    break;

  case ExprKind::IsNull:
    check(*expr.children[0], clause);
    expr.type = DataType::boolean();
    break;

  case ExprKind::InList: {
    Expr &operand = *expr.children[0];
    check(operand, clause);
    for (size_t index = 1; index < expr.children.size(); ++index) {
      check(*expr.children[index], clause);
      comparable(operand, *expr.children[index], "la lista de IN", expr.children[index]->loc);
    }
    expr.type = DataType::boolean();
    break;
  }

  case ExprKind::InSubquery:
  case ExprKind::Exists:
  case ExprKind::ScalarSubquery: {
    if (expr.kind == ExprKind::InSubquery)
      check(*expr.children[0], clause);
    analyzeSelect(*expr.subquery);
    const auto &output = expr.subquery->output;
    if (expr.kind == ExprKind::Exists) {
      expr.type = DataType::boolean();
      break;
    }
    if (output.size() != 1) {
      error("SEM015", std::string(expr.kind == ExprKind::ScalarSubquery
                                      ? "La subconsulta escalar"
                                      : "La subconsulta de IN") +
                          " debe retornar exactamente una columna; retorna " +
                          std::to_string(output.size()) + ".",
            expr.subquery->loc);
      expr.type = DataType::unknown();
      break;
    }
    if (expr.kind == ExprKind::ScalarSubquery) {
      expr.type = output.front().type;
      break;
    }
    Expr probe;
    probe.text = "subconsulta";
    probe.type = output.front().type;
    comparable(*expr.children[0], probe, "IN (SELECT ...)", expr.loc);
    expr.type = DataType::boolean();
    break;
  }

  case ExprKind::Aggregate: {
    if (!clause.aggregates) {
      error("SEM016", "No se permite la función " + expr.function + " en " +
                          clause.name + "; usa HAVING para filtrar grupos.",
            expr.loc);
    } else if (aggregate_depth_ > 0) {
      error("SEM016", "No se permite anidar funciones de agregación (" +
                          expr.function + " dentro de otra).",
            expr.loc);
    }
    if (expr.star) {
      expr.type = DataType::integer();
      break;
    }
    ++aggregate_depth_;
    const DataType argument = check(*expr.children[0], clause);
    --aggregate_depth_;
    if (expr.function == "COUNT") {
      expr.type = DataType::integer();
    } else if (expr.function == "SUM" || expr.function == "AVG") {
      if (!argument.isNumeric() && !argument.isNull() && !argument.isUnknown())
        error("SEM005", expr.function + " requiere una expresión numérica; '" +
                            expr.children[0]->text + "' es " + argument.toString() + ".",
              expr.loc);
      expr.type = expr.function == "AVG" ? DataType::real()
                  : argument.isNumeric() ? argument
                                         : DataType::unknown();
    } else {
      if (argument.base == BaseType::Boolean)
        error("SEM005", expr.function + " no acepta valores BOOLEAN.", expr.loc);
      expr.type = argument;
    }
    break;
  }
  }
  return expr.type;
}

} // namespace sql
