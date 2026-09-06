
// Generated from Compiscript.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "CompiscriptVisitor.h"


/**
 * This class provides an empty implementation of CompiscriptVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  CompiscriptBaseVisitor : public CompiscriptVisitor {
public:

  virtual std::any visitProgram(CompiscriptParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(CompiscriptParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(CompiscriptParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitControlBody(CompiscriptParser::ControlBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVariableDeclaration(CompiscriptParser::VariableDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConstantDeclaration(CompiscriptParser::ConstantDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeAnnotation(CompiscriptParser::TypeAnnotationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitializer(CompiscriptParser::InitializerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignmentStatement(CompiscriptParser::AssignmentStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpressionStatement(CompiscriptParser::ExpressionStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrintStatement(CompiscriptParser::PrintStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStatement(CompiscriptParser::IfStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStatement(CompiscriptParser::WhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDoWhileStatement(CompiscriptParser::DoWhileStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForStatement(CompiscriptParser::ForStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeachStatement(CompiscriptParser::ForeachStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStatement(CompiscriptParser::BreakStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStatement(CompiscriptParser::ContinueStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStatement(CompiscriptParser::ReturnStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTryCatchStatement(CompiscriptParser::TryCatchStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwitchStatement(CompiscriptParser::SwitchStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwitchCase(CompiscriptParser::SwitchCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefaultCase(CompiscriptParser::DefaultCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionDeclaration(CompiscriptParser::FunctionDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameters(CompiscriptParser::ParametersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameter(CompiscriptParser::ParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassDeclaration(CompiscriptParser::ClassDeclarationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitClassMember(CompiscriptParser::ClassMemberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(CompiscriptParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignExpr(CompiscriptParser::AssignExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprNoAssign(CompiscriptParser::ExprNoAssignContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalExpr(CompiscriptParser::ConditionalExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpr(CompiscriptParser::LogicalOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpr(CompiscriptParser::LogicalAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpr(CompiscriptParser::EqualityExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelationalExpr(CompiscriptParser::RelationalExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpr(CompiscriptParser::AdditiveExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpr(CompiscriptParser::MultiplicativeExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpr(CompiscriptParser::UnaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpr(CompiscriptParser::PrimaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteralExpr(CompiscriptParser::LiteralExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLeftHandSide(CompiscriptParser::LeftHandSideContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifierExpr(CompiscriptParser::IdentifierExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNewExpr(CompiscriptParser::NewExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitThisExpr(CompiscriptParser::ThisExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallExpr(CompiscriptParser::CallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexExpr(CompiscriptParser::IndexExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPropertyAccessExpr(CompiscriptParser::PropertyAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArguments(CompiscriptParser::ArgumentsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayLiteral(CompiscriptParser::ArrayLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(CompiscriptParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBaseType(CompiscriptParser::BaseTypeContext *ctx) override {
    return visitChildren(ctx);
  }


};

