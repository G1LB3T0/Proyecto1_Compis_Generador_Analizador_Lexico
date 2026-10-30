
// Generated from SQL.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SQLVisitor.h"


/**
 * This class provides an empty implementation of SQLVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SQLBaseVisitor : public SQLVisitor {
public:

  virtual std::any visitScript(SQLParser::ScriptContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(SQLParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreateTableStatement(SQLParser::CreateTableStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTableElement(SQLParser::TableElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumnDefinition(SQLParser::ColumnDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDataType(SQLParser::DataTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumnConstraint(SQLParser::ColumnConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTableConstraint(SQLParser::TableConstraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDropTableStatement(SQLParser::DropTableStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreateIndexStatement(SQLParser::CreateIndexStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDropIndexStatement(SQLParser::DropIndexStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertStatement(SQLParser::InsertStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValuesRow(SQLParser::ValuesRowContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdateStatement(SQLParser::UpdateStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignment(SQLParser::AssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeleteStatement(SQLParser::DeleteStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectStatement(SQLParser::SelectStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectItem(SQLParser::SelectItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFromClause(SQLParser::FromClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTableReference(SQLParser::TableReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoinClause(SQLParser::JoinClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereClause(SQLParser::WhereClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGroupByClause(SQLParser::GroupByClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitHavingClause(SQLParser::HavingClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderByClause(SQLParser::OrderByClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderItem(SQLParser::OrderItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLimitClause(SQLParser::LimitClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpression(SQLParser::PrimaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrExpression(SQLParser::OrExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAndExpression(SQLParser::AndExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInSubqueryExpression(SQLParser::InSubqueryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpression(SQLParser::AdditiveExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInListExpression(SQLParser::InListExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNotExpression(SQLParser::NotExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComparisonExpression(SQLParser::ComparisonExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpression(SQLParser::MultiplicativeExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpression(SQLParser::UnaryExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIsNullExpression(SQLParser::IsNullExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteralPrimary(SQLParser::LiteralPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregatePrimary(SQLParser::AggregatePrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumnPrimary(SQLParser::ColumnPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExistsPrimary(SQLParser::ExistsPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSubqueryPrimary(SQLParser::SubqueryPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenthesizedPrimary(SQLParser::ParenthesizedPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregateFunction(SQLParser::AggregateFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumnReference(SQLParser::ColumnReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(SQLParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifier(SQLParser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }


};

