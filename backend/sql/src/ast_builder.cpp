// Construye el AST: descarta tokens de puntuación y normaliza nombres y literales.
#include "sql/ast_builder.h"

#include <algorithm>
#include <cctype>

namespace sql {

namespace {

std::string lower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return text;
}

std::string upper(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  return text;
}

// 'O''Brien' -> O'Brien
std::string unquote(const std::string &literal) {
  std::string body = literal.substr(1, literal.size() - 2);
  std::string result;
  for (size_t index = 0; index < body.size(); ++index) {
    result += body[index];
    if (body[index] == '\'' && index + 1 < body.size() && body[index + 1] == '\'')
      ++index;
  }
  return result;
}

template <typename T> T *adopt(std::any value) {
  return std::any_cast<T *>(value);
}

} // namespace

SourceLocation locationOf(antlr4::Token *token) {
  if (!token)
    return {};
  return {token->getLine(), token->getCharPositionInLine() + 1};
}

std::string originalText(antlr4::ParserRuleContext *ctx) {
  if (!ctx || !ctx->getStart() || !ctx->getStop())
    return "";
  const size_t start = ctx->getStart()->getStartIndex();
  const size_t stop = ctx->getStop()->getStopIndex();
  if (stop < start || stop == static_cast<size_t>(-1))
    return ctx->getText();
  return ctx->getStart()->getInputStream()->getText(
      antlr4::misc::Interval(start, stop));
}

Script AstBuilder::build(SQLParser::ScriptContext *tree) {
  Script script;
  for (auto *statement : tree->statement()) {
    std::unique_ptr<Statement> node(adopt<Statement>(visit(statement)));
    node->loc = locationOf(statement->getStart());
    node->sql = originalText(statement);
    script.statements.push_back(std::move(*node));
  }
  return script;
}

void AstBuilder::error(const std::string &code, const std::string &message,
                       antlr4::Token *token) {
  const SourceLocation loc = locationOf(token);
  diagnostics_.push_back({code, "semántico", "error", message, loc.line,
                          loc.column});
}

std::string AstBuilder::name(SQLParser::IdentifierContext *ctx) const {
  return lower(ctx->getText());
}

ExprPtr AstBuilder::newExpr(ExprKind kind, antlr4::ParserRuleContext *ctx) {
  auto expr = std::make_unique<Expr>();
  expr->kind = kind;
  expr->loc = locationOf(ctx->getStart());
  expr->text = originalText(ctx);
  return expr;
}

ExprPtr AstBuilder::expression(antlr4::tree::ParseTree *ctx) {
  return ExprPtr(adopt<Expr>(visit(ctx)));
}

std::shared_ptr<SelectStatement>
AstBuilder::select(SQLParser::SelectStatementContext *ctx) {
  return std::shared_ptr<SelectStatement>(adopt<SelectStatement>(visit(ctx)));
}

// ───────────────────────────── Sentencias ─────────────────────────────

std::any AstBuilder::visitStatement(SQLParser::StatementContext *ctx) {
  // Cada alternativa devuelve su propio Statement*.
  return visit(ctx->children.front());
}

DataType AstBuilder::dataType(SQLParser::DataTypeContext *ctx) {
  if (ctx->K_INT())
    return DataType::integer();
  if (ctx->K_FLOAT())
    return DataType::real();
  if (ctx->K_BOOLEAN())
    return DataType::boolean();
  if (ctx->K_DATE())
    return DataType::date();
  int length = 0;
  try {
    length = std::stoi(ctx->INT_LITERAL()->getText());
  } catch (...) {
    error("SEM018", "La longitud de VARCHAR está fuera de rango.",
          ctx->INT_LITERAL()->getSymbol());
  }
  return DataType::varchar(length);
}

std::any AstBuilder::visitCreateTableStatement(
    SQLParser::CreateTableStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::CreateTable;
  CreateTableStatement node;
  node.table = name(ctx->identifier());
  node.if_not_exists = ctx->K_IF() != nullptr;
  for (auto *element : ctx->tableElement()) {
    if (auto *column = element->columnDefinition()) {
      ColumnDefinition definition;
      definition.name = name(column->identifier());
      definition.type = dataType(column->dataType());
      definition.loc = locationOf(column->getStart());
      for (auto *constraint : column->columnConstraint()) {
        if (constraint->K_PRIMARY())
          definition.primary_key = true;
        else if (constraint->K_NOT())
          definition.not_null = true;
        else if (constraint->K_UNIQUE())
          definition.unique = true;
        else if (constraint->K_REFERENCES()) {
          definition.ref_table = name(constraint->identifier(0));
          definition.ref_column = name(constraint->identifier(1));
        }
      }
      node.columns.push_back(std::move(definition));
      continue;
    }
    auto *constraint = element->tableConstraint();
    TableConstraint table_constraint;
    table_constraint.loc = locationOf(constraint->getStart());
    table_constraint.column = name(constraint->identifier(0));
    if (constraint->K_PRIMARY()) {
      table_constraint.kind = "PRIMARY KEY";
    } else if (constraint->K_FOREIGN()) {
      table_constraint.kind = "FOREIGN KEY";
      table_constraint.ref_table = name(constraint->identifier(1));
      table_constraint.ref_column = name(constraint->identifier(2));
    } else {
      table_constraint.kind = "UNIQUE";
    }
    node.constraints.push_back(std::move(table_constraint));
  }
  statement->node = std::move(node);
  return statement;
}

std::any AstBuilder::visitDropTableStatement(
    SQLParser::DropTableStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::DropTable;
  statement->node = DropTableStatement{name(ctx->identifier()), ctx->K_IF() != nullptr};
  return statement;
}

std::any AstBuilder::visitCreateIndexStatement(
    SQLParser::CreateIndexStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::CreateIndex;
  CreateIndexStatement node;
  node.name = name(ctx->identifier(0));
  node.table = name(ctx->identifier(1));
  node.column = name(ctx->identifier(2));
  node.unique = ctx->K_UNIQUE() != nullptr;
  node.table_loc = locationOf(ctx->identifier(1)->getStart());
  node.column_loc = locationOf(ctx->identifier(2)->getStart());
  statement->node = std::move(node);
  return statement;
}

std::any AstBuilder::visitDropIndexStatement(
    SQLParser::DropIndexStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::DropIndex;
  statement->node = DropIndexStatement{name(ctx->identifier()), ctx->K_IF() != nullptr};
  return statement;
}

std::any AstBuilder::visitInsertStatement(SQLParser::InsertStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::Insert;
  InsertStatement node;
  auto identifiers = ctx->identifier();
  node.table = name(identifiers.front());
  for (size_t index = 1; index < identifiers.size(); ++index) {
    node.columns.push_back(name(identifiers[index]));
    node.column_locs.push_back(locationOf(identifiers[index]->getStart()));
  }
  for (auto *row : ctx->valuesRow()) {
    std::vector<ExprPtr> values;
    for (auto *value : row->expression())
      values.push_back(expression(value));
    node.rows.push_back(std::move(values));
    node.row_locs.push_back(locationOf(row->getStart()));
  }
  statement->node = std::move(node);
  return statement;
}

std::any AstBuilder::visitUpdateStatement(SQLParser::UpdateStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::Update;
  UpdateStatement node;
  node.table = name(ctx->identifier());
  for (auto *assignment : ctx->assignment()) {
    Assignment item;
    item.column = name(assignment->identifier());
    item.loc = locationOf(assignment->getStart());
    item.value = expression(assignment->expression());
    node.assignments.push_back(std::move(item));
  }
  if (ctx->whereClause())
    node.where = expression(ctx->whereClause()->expression());
  statement->node = std::move(node);
  return statement;
}

std::any AstBuilder::visitDeleteStatement(SQLParser::DeleteStatementContext *ctx) {
  auto *statement = new Statement();
  statement->kind = StatementKind::Delete;
  DeleteStatement node;
  node.table = name(ctx->identifier());
  if (ctx->whereClause())
    node.where = expression(ctx->whereClause()->expression());
  statement->node = std::move(node);
  return statement;
}

TableReference AstBuilder::tableReference(SQLParser::TableReferenceContext *ctx) {
  TableReference reference;
  reference.name = name(ctx->identifier(0));
  reference.alias = ctx->identifier().size() > 1 ? name(ctx->identifier(1))
                                                 : reference.name;
  reference.loc = locationOf(ctx->getStart());
  return reference;
}

std::any AstBuilder::visitSelectStatement(SQLParser::SelectStatementContext *ctx) {
  // Una consulta anidada también pasa por aquí: devuelve SelectStatement*.
  auto *node = new SelectStatement();
  node->loc = locationOf(ctx->getStart());
  node->distinct = ctx->K_DISTINCT() != nullptr;
  for (auto *item : ctx->selectItem()) {
    SelectItem select_item;
    select_item.loc = locationOf(item->getStart());
    if (item->STAR()) {
      select_item.star = true;
      if (item->identifier() && item->DOT())
        select_item.star_table = name(item->identifier());
    } else {
      select_item.expr = expression(item->expression());
      if (item->identifier())
        select_item.alias = name(item->identifier());
    }
    node->items.push_back(std::move(select_item));
  }
  if (auto *from = ctx->fromClause()) {
    node->has_from = true;
    node->from = tableReference(from->tableReference());
    for (auto *join : from->joinClause()) {
      JoinClause clause;
      clause.table = tableReference(join->tableReference());
      clause.condition = expression(join->expression());
      node->joins.push_back(std::move(clause));
    }
  }
  if (ctx->whereClause())
    node->where = expression(ctx->whereClause()->expression());
  if (ctx->groupByClause()) {
    for (auto *expr : ctx->groupByClause()->expression())
      node->group_by.push_back(expression(expr));
  }
  if (ctx->havingClause())
    node->having = expression(ctx->havingClause()->expression());
  if (ctx->orderByClause()) {
    for (auto *item : ctx->orderByClause()->orderItem()) {
      OrderItem order;
      order.expr = expression(item->expression());
      order.descending = item->K_DESC() != nullptr;
      node->order_by.push_back(std::move(order));
    }
  }
  if (ctx->limitClause()) {
    try {
      node->limit = std::stoll(ctx->limitClause()->INT_LITERAL()->getText());
    } catch (...) {
      error("SEM018", "El valor de LIMIT está fuera de rango.",
            ctx->limitClause()->INT_LITERAL()->getSymbol());
    }
  }
  // Cuando la consulta es una sentencia completa la envuelve visitStatement.
  if (dynamic_cast<SQLParser::StatementContext *>(ctx->parent)) {
    auto *statement = new Statement();
    statement->kind = StatementKind::Select;
    statement->node = std::shared_ptr<SelectStatement>(node);
    return statement;
  }
  return node;
}

// ───────────────────────────── Expresiones ─────────────────────────────

std::any AstBuilder::visitPrimaryExpression(SQLParser::PrimaryExpressionContext *ctx) {
  return visit(ctx->primary());
}

std::any AstBuilder::visitUnaryExpression(SQLParser::UnaryExpressionContext *ctx) {
  ExprPtr operand = expression(ctx->expression());
  // -5 se pliega a un literal negativo; así el AST queda más limpio.
  if (operand->kind == ExprKind::Literal && operand->literal.isNumeric()) {
    if (ctx->op->getType() == SQLParser::MINUS) {
      if (operand->literal.kind == Value::Kind::Int)
        operand->literal.integer = -operand->literal.integer;
      else
        operand->literal.real = -operand->literal.real;
    }
    operand->text = originalText(ctx);
    operand->loc = locationOf(ctx->getStart());
    return operand.release();
  }
  ExprPtr expr = newExpr(ExprKind::Unary, ctx);
  expr->op = ctx->op->getText();
  expr->children.push_back(std::move(operand));
  return expr.release();
}

ExprPtr AstBuilder::binary(antlr4::ParserRuleContext *ctx, const std::string &op,
                           SQLParser::ExpressionContext *left,
                           SQLParser::ExpressionContext *right) {
  ExprPtr expr = newExpr(ExprKind::Binary, ctx);
  expr->op = op;
  expr->children.push_back(expression(left));
  expr->children.push_back(expression(right));
  return expr;
}

std::any AstBuilder::visitMultiplicativeExpression(
    SQLParser::MultiplicativeExpressionContext *ctx) {
  return binary(ctx, ctx->op->getText(), ctx->expression(0), ctx->expression(1))
      .release();
}

std::any AstBuilder::visitAdditiveExpression(SQLParser::AdditiveExpressionContext *ctx) {
  return binary(ctx, ctx->op->getText(), ctx->expression(0), ctx->expression(1))
      .release();
}

std::any AstBuilder::visitComparisonExpression(
    SQLParser::ComparisonExpressionContext *ctx) {
  std::string op = ctx->op->getText();
  if (op == "!=")
    op = "<>";
  return binary(ctx, op, ctx->expression(0), ctx->expression(1)).release();
}

std::any AstBuilder::visitAndExpression(SQLParser::AndExpressionContext *ctx) {
  return binary(ctx, "AND", ctx->expression(0), ctx->expression(1)).release();
}

std::any AstBuilder::visitOrExpression(SQLParser::OrExpressionContext *ctx) {
  return binary(ctx, "OR", ctx->expression(0), ctx->expression(1)).release();
}

std::any AstBuilder::visitNotExpression(SQLParser::NotExpressionContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::Unary, ctx);
  expr->op = "NOT";
  expr->children.push_back(expression(ctx->expression()));
  return expr.release();
}

std::any AstBuilder::visitIsNullExpression(SQLParser::IsNullExpressionContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::IsNull, ctx);
  expr->negated = ctx->K_NOT() != nullptr;
  expr->children.push_back(expression(ctx->expression()));
  return expr.release();
}

std::any AstBuilder::visitInSubqueryExpression(
    SQLParser::InSubqueryExpressionContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::InSubquery, ctx);
  expr->negated = ctx->K_NOT() != nullptr;
  expr->children.push_back(expression(ctx->expression()));
  expr->subquery = select(ctx->selectStatement());
  return expr.release();
}

std::any AstBuilder::visitInListExpression(SQLParser::InListExpressionContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::InList, ctx);
  expr->negated = ctx->K_NOT() != nullptr;
  // expression(0) es el operando; el resto son los elementos de la lista.
  for (auto *item : ctx->expression())
    expr->children.push_back(expression(item));
  return expr.release();
}

std::any AstBuilder::visitLiteralPrimary(SQLParser::LiteralPrimaryContext *ctx) {
  auto *literal = ctx->literal();
  ExprPtr expr = newExpr(ExprKind::Literal, ctx);
  if (auto *token = literal->INT_LITERAL()) {
    try {
      expr->literal = Value::makeInt(std::stoll(token->getText()));
    } catch (...) {
      error("SEM018", "El entero " + token->getText() + " está fuera de rango.",
            token->getSymbol());
    }
  } else if (auto *token = literal->FLOAT_LITERAL()) {
    const auto value = parseFloat(token->getText());
    if (value)
      expr->literal = Value::makeFloat(*value);
    else
      error("SEM018", "El decimal está fuera del rango de FLOAT.", token->getSymbol());
  } else if (literal->K_DATE()) {
    const std::string text = unquote(literal->STRING_LITERAL()->getText());
    if (!isValidDate(text))
      error("SEM009", "'" + text + "' no es una fecha válida con formato YYYY-MM-DD.",
            literal->STRING_LITERAL()->getSymbol());
    expr->literal = Value::makeDate(text);
  } else if (auto *token = literal->STRING_LITERAL()) {
    expr->literal = Value::makeText(unquote(token->getText()));
  } else if (literal->K_TRUE()) {
    expr->literal = Value::makeBool(true);
  } else if (literal->K_FALSE()) {
    expr->literal = Value::makeBool(false);
  }
  return expr.release();
}

std::any AstBuilder::visitAggregatePrimary(SQLParser::AggregatePrimaryContext *ctx) {
  auto *aggregate = ctx->aggregateFunction();
  ExprPtr expr = newExpr(ExprKind::Aggregate, ctx);
  if (aggregate->STAR()) {
    expr->function = "COUNT";
    expr->star = true;
  } else {
    expr->function = upper(aggregate->name->getText());
    expr->distinct = aggregate->K_DISTINCT() != nullptr;
    expr->children.push_back(expression(aggregate->expression()));
  }
  return expr.release();
}

std::any AstBuilder::visitColumnPrimary(SQLParser::ColumnPrimaryContext *ctx) {
  auto *reference = ctx->columnReference();
  ExprPtr expr = newExpr(ExprKind::Column, ctx);
  auto identifiers = reference->identifier();
  if (identifiers.size() == 2) {
    expr->table = name(identifiers[0]);
    expr->column = name(identifiers[1]);
  } else {
    expr->column = name(identifiers[0]);
  }
  return expr.release();
}

std::any AstBuilder::visitExistsPrimary(SQLParser::ExistsPrimaryContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::Exists, ctx);
  expr->subquery = select(ctx->selectStatement());
  return expr.release();
}

std::any AstBuilder::visitSubqueryPrimary(SQLParser::SubqueryPrimaryContext *ctx) {
  ExprPtr expr = newExpr(ExprKind::ScalarSubquery, ctx);
  expr->subquery = select(ctx->selectStatement());
  return expr.release();
}

std::any AstBuilder::visitParenthesizedPrimary(
    SQLParser::ParenthesizedPrimaryContext *ctx) {
  // Los paréntesis solo agrupan: el AST conserva la expresión interior.
  return visit(ctx->expression());
}

std::string statementKindName(StatementKind kind) {
  switch (kind) {
  case StatementKind::CreateTable:
    return "CREATE TABLE";
  case StatementKind::DropTable:
    return "DROP TABLE";
  case StatementKind::CreateIndex:
    return "CREATE INDEX";
  case StatementKind::DropIndex:
    return "DROP INDEX";
  case StatementKind::Insert:
    return "INSERT";
  case StatementKind::Update:
    return "UPDATE";
  case StatementKind::Delete:
    return "DELETE";
  case StatementKind::Select:
    return "SELECT";
  }
  return "";
}

} // namespace sql
