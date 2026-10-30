// Visitor de ANTLR que transforma el parse tree concreto en el AST de SQL.
#pragma once

#include "SQLBaseVisitor.h"
#include "sql/ast.h"
#include "sql/model.h"

#include <vector>

namespace sql {

// Cada visitXxx devuelve, dentro de std::any, un puntero recién creado
// (Expr*, SelectStatement* o Statement*) que quien llama adopta de inmediato
// en un unique_ptr. Se usa puntero crudo porque std::any exige tipos copiables.
class AstBuilder final : public SQLBaseVisitor {
public:
  Script build(SQLParser::ScriptContext *tree);
  const std::vector<Diagnostic> &diagnostics() const { return diagnostics_; }

  std::any visitStatement(SQLParser::StatementContext *ctx) override;
  std::any visitCreateTableStatement(SQLParser::CreateTableStatementContext *ctx) override;
  std::any visitDropTableStatement(SQLParser::DropTableStatementContext *ctx) override;
  std::any visitCreateIndexStatement(SQLParser::CreateIndexStatementContext *ctx) override;
  std::any visitDropIndexStatement(SQLParser::DropIndexStatementContext *ctx) override;
  std::any visitInsertStatement(SQLParser::InsertStatementContext *ctx) override;
  std::any visitUpdateStatement(SQLParser::UpdateStatementContext *ctx) override;
  std::any visitDeleteStatement(SQLParser::DeleteStatementContext *ctx) override;
  std::any visitSelectStatement(SQLParser::SelectStatementContext *ctx) override;

  std::any visitPrimaryExpression(SQLParser::PrimaryExpressionContext *ctx) override;
  std::any visitUnaryExpression(SQLParser::UnaryExpressionContext *ctx) override;
  std::any visitMultiplicativeExpression(SQLParser::MultiplicativeExpressionContext *ctx) override;
  std::any visitAdditiveExpression(SQLParser::AdditiveExpressionContext *ctx) override;
  std::any visitComparisonExpression(SQLParser::ComparisonExpressionContext *ctx) override;
  std::any visitIsNullExpression(SQLParser::IsNullExpressionContext *ctx) override;
  std::any visitInSubqueryExpression(SQLParser::InSubqueryExpressionContext *ctx) override;
  std::any visitInListExpression(SQLParser::InListExpressionContext *ctx) override;
  std::any visitNotExpression(SQLParser::NotExpressionContext *ctx) override;
  std::any visitAndExpression(SQLParser::AndExpressionContext *ctx) override;
  std::any visitOrExpression(SQLParser::OrExpressionContext *ctx) override;

  std::any visitLiteralPrimary(SQLParser::LiteralPrimaryContext *ctx) override;
  std::any visitAggregatePrimary(SQLParser::AggregatePrimaryContext *ctx) override;
  std::any visitColumnPrimary(SQLParser::ColumnPrimaryContext *ctx) override;
  std::any visitExistsPrimary(SQLParser::ExistsPrimaryContext *ctx) override;
  std::any visitSubqueryPrimary(SQLParser::SubqueryPrimaryContext *ctx) override;
  std::any visitParenthesizedPrimary(SQLParser::ParenthesizedPrimaryContext *ctx) override;

private:
  ExprPtr expression(antlr4::tree::ParseTree *ctx);
  std::shared_ptr<SelectStatement> select(SQLParser::SelectStatementContext *ctx);
  ExprPtr binary(antlr4::ParserRuleContext *ctx, const std::string &op,
                 SQLParser::ExpressionContext *left,
                 SQLParser::ExpressionContext *right);
  ExprPtr newExpr(ExprKind kind, antlr4::ParserRuleContext *ctx);
  std::string name(SQLParser::IdentifierContext *ctx) const;
  DataType dataType(SQLParser::DataTypeContext *ctx);
  TableReference tableReference(SQLParser::TableReferenceContext *ctx);
  void error(const std::string &code, const std::string &message,
             antlr4::Token *token);

  std::vector<Diagnostic> diagnostics_;
};

// Texto original (con espacios) que abarca un nodo del parse tree.
std::string originalText(antlr4::ParserRuleContext *ctx);
SourceLocation locationOf(antlr4::Token *token);

} // namespace sql
