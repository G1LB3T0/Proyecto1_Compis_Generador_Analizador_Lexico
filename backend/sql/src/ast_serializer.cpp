// Recorre el AST (ya decorado por la semántica) y lo expresa como árbol JSON.
// Nodos internos = "rule" (operaciones/cláusulas); hojas = "token" (valores).
#include "sql/ast_serializer.h"

#include "sql/json.h"

#include <sstream>
#include <vector>

namespace sql {

namespace {

struct Node {
  std::string kind; // rule | token
  std::string name;
  std::string text;
  std::string category; // hojas: qué representa el valor
  std::string detail;   // nodos internos: tipo inferido o descripción
  SourceLocation loc;
  std::vector<Node> children;
};

Node rule(const std::string &name, const SourceLocation &loc,
          std::vector<Node> children = {}, const std::string &detail = "") {
  return {"rule", name, "", "", detail, loc, std::move(children)};
}

Node leaf(const std::string &text, const std::string &category,
          const SourceLocation &loc) {
  return {"token", text, text, category, "", loc, {}};
}

std::string typeSuffix(const DataType &type) {
  return type.isUnknown() ? "" : " → " + type.toString();
}

Node selectNode(const SelectStatement &select);

Node expression(const Expr &expr) {
  switch (expr.kind) {
  case ExprKind::Literal: {
    std::string text = expr.literal.toString();
    if (expr.literal.kind == Value::Kind::Text)
      text = "'" + text + "'";
    if (expr.literal.kind == Value::Kind::Date)
      text = "DATE '" + text + "'";
    return leaf(text, "Literal " + expr.literal.typeName(), expr.loc);
  }
  case ExprKind::Column: {
    const std::string name = expr.table.empty() ? expr.column : expr.table + "." + expr.column;
    std::string category = "Columna";
    if (!expr.type.isUnknown())
      category += " " + expr.type.toString();
    if (expr.depth > 0)
      category += " · externa";
    return leaf(name, category, expr.loc);
  }
  case ExprKind::Unary: {
    std::vector<Node> children;
    children.push_back(expression(*expr.children[0]));
    return rule(expr.op, expr.loc, std::move(children),
                (expr.op == "NOT" ? "Negación" : "Signo") + typeSuffix(expr.type));
  }
  case ExprKind::Binary: {
    std::vector<Node> children;
    for (const auto &child : expr.children)
      children.push_back(expression(*child));
    const std::string &op = expr.op;
    std::string role = "Aritmética";
    if (op == "AND" || op == "OR")
      role = "Lógica";
    else if (op == "=" || op == "<>" || op == "<" || op == "<=" || op == ">" || op == ">=")
      role = "Comparación";
    return rule(op, expr.loc, std::move(children), role + typeSuffix(expr.type));
  }
  case ExprKind::IsNull: {
    std::vector<Node> children;
    children.push_back(expression(*expr.children[0]));
    return rule(expr.negated ? "IS NOT NULL" : "IS NULL", expr.loc, std::move(children),
                "Prueba de nulo → BOOLEAN");
  }
  case ExprKind::InList: {
    std::vector<Node> items;
    for (size_t index = 1; index < expr.children.size(); ++index)
      items.push_back(expression(*expr.children[index]));
    std::vector<Node> children;
    children.push_back(expression(*expr.children[0]));
    children.push_back(rule("Lista", expr.loc, std::move(items)));
    return rule(expr.negated ? "NOT IN" : "IN", expr.loc, std::move(children),
                "Pertenencia → BOOLEAN");
  }
  case ExprKind::InSubquery: {
    std::vector<Node> children;
    children.push_back(expression(*expr.children[0]));
    children.push_back(selectNode(*expr.subquery));
    return rule(expr.negated ? "NOT IN" : "IN", expr.loc, std::move(children),
                expr.subquery->correlated ? "Subconsulta correlacionada"
                                          : "Subconsulta");
  }
  case ExprKind::Exists: {
    std::vector<Node> children;
    children.push_back(selectNode(*expr.subquery));
    return rule("EXISTS", expr.loc, std::move(children),
                expr.subquery->correlated ? "Subconsulta correlacionada"
                                          : "Subconsulta");
  }
  case ExprKind::ScalarSubquery: {
    std::vector<Node> children;
    children.push_back(selectNode(*expr.subquery));
    return rule("Subconsulta escalar", expr.loc, std::move(children),
                std::string(expr.subquery->correlated ? "Correlacionada" : "Un valor") +
                    typeSuffix(expr.type));
  }
  case ExprKind::Aggregate: {
    std::vector<Node> children;
    if (expr.star)
      children.push_back(leaf("*", "Todas las filas", expr.loc));
    else
      children.push_back(expression(*expr.children[0]));
    return rule(expr.function + (expr.distinct ? " DISTINCT" : ""), expr.loc,
                std::move(children), "Agregación" + typeSuffix(expr.type));
  }
  }
  return leaf("?", "", expr.loc);
}

Node tableNode(const TableReference &table) {
  const std::string label =
      table.alias == table.name ? table.name : table.name + " AS " + table.alias;
  return leaf(label, "Tabla", table.loc);
}

Node clause(const std::string &name, const Expr &expr) {
  std::vector<Node> children;
  children.push_back(expression(expr));
  return rule(name, expr.loc, std::move(children));
}

Node selectNode(const SelectStatement &select) {
  std::vector<Node> children;
  std::vector<Node> items;
  for (const auto &item : select.items) {
    if (item.star) {
      items.push_back(leaf(item.star_table.empty() ? "*" : item.star_table + ".*",
                           "Todas las columnas", item.loc));
      continue;
    }
    Node node = expression(*item.expr);
    if (!item.alias.empty()) {
      std::vector<Node> aliased;
      aliased.push_back(std::move(node));
      aliased.push_back(leaf(item.alias, "Alias", item.loc));
      node = rule("AS", item.loc, std::move(aliased));
    }
    items.push_back(std::move(node));
  }
  children.push_back(rule(select.distinct ? "Proyección DISTINCT" : "Proyección",
                          select.loc, std::move(items)));
  if (select.has_from) {
    std::vector<Node> from;
    from.push_back(tableNode(select.from));
    children.push_back(rule("FROM", select.from.loc, std::move(from)));
  }
  for (const auto &join : select.joins) {
    std::vector<Node> parts;
    parts.push_back(tableNode(join.table));
    parts.push_back(clause("ON", *join.condition));
    children.push_back(rule("INNER JOIN", join.table.loc, std::move(parts)));
  }
  if (select.where)
    children.push_back(clause("WHERE", *select.where));
  if (!select.group_by.empty()) {
    std::vector<Node> groups;
    for (const auto &expr : select.group_by)
      groups.push_back(expression(*expr));
    children.push_back(rule("GROUP BY", select.group_by.front()->loc, std::move(groups)));
  }
  if (select.having)
    children.push_back(clause("HAVING", *select.having));
  if (!select.order_by.empty()) {
    std::vector<Node> orders;
    for (const auto &item : select.order_by) {
      std::vector<Node> key;
      key.push_back(expression(*item.expr));
      orders.push_back(rule(item.descending ? "DESC" : "ASC", item.expr->loc, std::move(key)));
    }
    children.push_back(rule("ORDER BY", select.order_by.front().expr->loc, std::move(orders)));
  }
  if (select.limit)
    children.push_back(leaf(std::to_string(*select.limit), "LIMIT", select.loc));
  return rule("SELECT", select.loc, std::move(children),
              select.aggregate_query ? "Consulta con agregación" : "Consulta");
}

std::string constraintsText(const ColumnDefinition &column) {
  std::string text;
  auto add = [&](const std::string &part) { text += (text.empty() ? "" : " ") + part; };
  if (column.primary_key)
    add("PRIMARY KEY");
  if (column.not_null)
    add("NOT NULL");
  if (column.unique)
    add("UNIQUE");
  if (!column.ref_table.empty())
    add("→ " + column.ref_table + "(" + column.ref_column + ")");
  return text;
}

Node statementNode(const Statement &statement) {
  std::vector<Node> children;
  switch (statement.kind) {
  case StatementKind::CreateTable: {
    const auto &node = std::get<CreateTableStatement>(statement.node);
    children.push_back(leaf(node.table, "Tabla", statement.loc));
    std::vector<Node> columns;
    for (const auto &column : node.columns) {
      std::vector<Node> parts;
      parts.push_back(leaf(column.type.toString(), "Tipo de dato", column.loc));
      const std::string constraints = constraintsText(column);
      if (!constraints.empty())
        parts.push_back(leaf(constraints, "Restricciones", column.loc));
      columns.push_back(rule(column.name, column.loc, std::move(parts), "Columna"));
    }
    children.push_back(rule("Columnas", statement.loc, std::move(columns)));
    for (const auto &constraint : node.constraints) {
      std::string text = constraint.kind + " (" + constraint.column + ")";
      if (!constraint.ref_table.empty())
        text += " → " + constraint.ref_table + "(" + constraint.ref_column + ")";
      children.push_back(leaf(text, "Restricción de tabla", constraint.loc));
    }
    if (node.if_not_exists)
      children.push_back(leaf("IF NOT EXISTS", "Opción", statement.loc));
    break;
  }
  case StatementKind::DropTable: {
    const auto &node = std::get<DropTableStatement>(statement.node);
    children.push_back(leaf(node.table, "Tabla", statement.loc));
    if (node.if_exists)
      children.push_back(leaf("IF EXISTS", "Opción", statement.loc));
    break;
  }
  case StatementKind::CreateIndex: {
    const auto &node = std::get<CreateIndexStatement>(statement.node);
    children.push_back(leaf(node.name, node.unique ? "Índice UNIQUE" : "Índice", statement.loc));
    children.push_back(leaf(node.table, "Tabla", node.table_loc));
    children.push_back(leaf(node.column, "Columna indexada", node.column_loc));
    break;
  }
  case StatementKind::DropIndex: {
    const auto &node = std::get<DropIndexStatement>(statement.node);
    children.push_back(leaf(node.name, "Índice", statement.loc));
    if (node.if_exists)
      children.push_back(leaf("IF EXISTS", "Opción", statement.loc));
    break;
  }
  case StatementKind::Insert: {
    const auto &node = std::get<InsertStatement>(statement.node);
    children.push_back(leaf(node.table, "Tabla", statement.loc));
    if (!node.columns.empty()) {
      std::vector<Node> columns;
      for (size_t index = 0; index < node.columns.size(); ++index)
        columns.push_back(leaf(node.columns[index], "Columna", node.column_locs[index]));
      children.push_back(rule("Columnas", statement.loc, std::move(columns)));
    }
    std::vector<Node> rows;
    for (size_t row = 0; row < node.rows.size(); ++row) {
      std::vector<Node> values;
      for (const auto &value : node.rows[row])
        values.push_back(expression(*value));
      rows.push_back(rule("Fila " + std::to_string(row + 1), node.row_locs[row],
                          std::move(values)));
    }
    children.push_back(rule("VALUES", statement.loc, std::move(rows)));
    break;
  }
  case StatementKind::Update: {
    const auto &node = std::get<UpdateStatement>(statement.node);
    children.push_back(leaf(node.table, "Tabla", statement.loc));
    std::vector<Node> assignments;
    for (const auto &assignment : node.assignments) {
      std::vector<Node> parts;
      parts.push_back(leaf(assignment.column, "Columna destino", assignment.loc));
      parts.push_back(expression(*assignment.value));
      assignments.push_back(rule("=", assignment.loc, std::move(parts), "Asignación"));
    }
    children.push_back(rule("SET", statement.loc, std::move(assignments)));
    if (node.where)
      children.push_back(clause("WHERE", *node.where));
    break;
  }
  case StatementKind::Delete: {
    const auto &node = std::get<DeleteStatement>(statement.node);
    children.push_back(leaf(node.table, "Tabla", statement.loc));
    if (node.where)
      children.push_back(clause("WHERE", *node.where));
    break;
  }
  case StatementKind::Select:
    return selectNode(*std::get<std::shared_ptr<SelectStatement>>(statement.node));
  }
  return rule(statementKindName(statement.kind), statement.loc, std::move(children),
              "Sentencia");
}

void write(std::ostringstream &output, const Node &node) {
  output << "{\"kind\":" << quoteJson(node.kind) << ",\"name\":" << quoteJson(node.name)
         << ",\"text\":" << quoteJson(node.text)
         << ",\"category\":" << quoteJson(node.category)
         << ",\"detail\":" << quoteJson(node.detail) << ",\"line\":" << node.loc.line
         << ",\"column\":" << node.loc.column << ",\"children\":[";
  for (size_t index = 0; index < node.children.size(); ++index) {
    if (index)
      output << ',';
    write(output, node.children[index]);
  }
  output << "]}";
}

} // namespace

std::string astToJson(const Script &script) {
  std::vector<Node> statements;
  for (const auto &statement : script.statements)
    statements.push_back(statementNode(statement));
  const Node root = rule("Script", {1, 1}, std::move(statements),
                         std::to_string(script.statements.size()) + " sentencia(s)");
  std::ostringstream output;
  write(output, root);
  return output.str();
}

} // namespace sql
