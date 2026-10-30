
// Generated from SQL.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SQLParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SQLParser.
 */
class  SQLVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SQLParser.
   */
    virtual std::any visitScript(SQLParser::ScriptContext *context) = 0;

    virtual std::any visitStatement(SQLParser::StatementContext *context) = 0;

    virtual std::any visitCreateTableStatement(SQLParser::CreateTableStatementContext *context) = 0;

    virtual std::any visitTableElement(SQLParser::TableElementContext *context) = 0;

    virtual std::any visitColumnDefinition(SQLParser::ColumnDefinitionContext *context) = 0;

    virtual std::any visitDataType(SQLParser::DataTypeContext *context) = 0;

    virtual std::any visitColumnConstraint(SQLParser::ColumnConstraintContext *context) = 0;

    virtual std::any visitTableConstraint(SQLParser::TableConstraintContext *context) = 0;

    virtual std::any visitDropTableStatement(SQLParser::DropTableStatementContext *context) = 0;

    virtual std::any visitCreateIndexStatement(SQLParser::CreateIndexStatementContext *context) = 0;

    virtual std::any visitDropIndexStatement(SQLParser::DropIndexStatementContext *context) = 0;

    virtual std::any visitInsertStatement(SQLParser::InsertStatementContext *context) = 0;

    virtual std::any visitValuesRow(SQLParser::ValuesRowContext *context) = 0;

    virtual std::any visitUpdateStatement(SQLParser::UpdateStatementContext *context) = 0;

    virtual std::any visitAssignment(SQLParser::AssignmentContext *context) = 0;

    virtual std::any visitDeleteStatement(SQLParser::DeleteStatementContext *context) = 0;

    virtual std::any visitSelectStatement(SQLParser::SelectStatementContext *context) = 0;

    virtual std::any visitSelectItem(SQLParser::SelectItemContext *context) = 0;

    virtual std::any visitFromClause(SQLParser::FromClauseContext *context) = 0;

    virtual std::any visitTableReference(SQLParser::TableReferenceContext *context) = 0;

    virtual std::any visitJoinClause(SQLParser::JoinClauseContext *context) = 0;

    virtual std::any visitWhereClause(SQLParser::WhereClauseContext *context) = 0;

    virtual std::any visitGroupByClause(SQLParser::GroupByClauseContext *context) = 0;

    virtual std::any visitHavingClause(SQLParser::HavingClauseContext *context) = 0;

    virtual std::any visitOrderByClause(SQLParser::OrderByClauseContext *context) = 0;

    virtual std::any visitOrderItem(SQLParser::OrderItemContext *context) = 0;

    virtual std::any visitLimitClause(SQLParser::LimitClauseContext *context) = 0;

    virtual std::any visitPrimaryExpression(SQLParser::PrimaryExpressionContext *context) = 0;

    virtual std::any visitOrExpression(SQLParser::OrExpressionContext *context) = 0;

    virtual std::any visitAndExpression(SQLParser::AndExpressionContext *context) = 0;

    virtual std::any visitInSubqueryExpression(SQLParser::InSubqueryExpressionContext *context) = 0;

    virtual std::any visitAdditiveExpression(SQLParser::AdditiveExpressionContext *context) = 0;

    virtual std::any visitInListExpression(SQLParser::InListExpressionContext *context) = 0;

    virtual std::any visitNotExpression(SQLParser::NotExpressionContext *context) = 0;

    virtual std::any visitComparisonExpression(SQLParser::ComparisonExpressionContext *context) = 0;

    virtual std::any visitMultiplicativeExpression(SQLParser::MultiplicativeExpressionContext *context) = 0;

    virtual std::any visitUnaryExpression(SQLParser::UnaryExpressionContext *context) = 0;

    virtual std::any visitIsNullExpression(SQLParser::IsNullExpressionContext *context) = 0;

    virtual std::any visitLiteralPrimary(SQLParser::LiteralPrimaryContext *context) = 0;

    virtual std::any visitAggregatePrimary(SQLParser::AggregatePrimaryContext *context) = 0;

    virtual std::any visitColumnPrimary(SQLParser::ColumnPrimaryContext *context) = 0;

    virtual std::any visitExistsPrimary(SQLParser::ExistsPrimaryContext *context) = 0;

    virtual std::any visitSubqueryPrimary(SQLParser::SubqueryPrimaryContext *context) = 0;

    virtual std::any visitParenthesizedPrimary(SQLParser::ParenthesizedPrimaryContext *context) = 0;

    virtual std::any visitAggregateFunction(SQLParser::AggregateFunctionContext *context) = 0;

    virtual std::any visitColumnReference(SQLParser::ColumnReferenceContext *context) = 0;

    virtual std::any visitLiteral(SQLParser::LiteralContext *context) = 0;

    virtual std::any visitIdentifier(SQLParser::IdentifierContext *context) = 0;


};

