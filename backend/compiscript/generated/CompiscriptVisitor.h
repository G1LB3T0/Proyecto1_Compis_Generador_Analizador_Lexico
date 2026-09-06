
// Generated from Compiscript.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "CompiscriptParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by CompiscriptParser.
 */
class  CompiscriptVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by CompiscriptParser.
   */
    virtual std::any visitProgram(CompiscriptParser::ProgramContext *context) = 0;

    virtual std::any visitStatement(CompiscriptParser::StatementContext *context) = 0;

    virtual std::any visitBlock(CompiscriptParser::BlockContext *context) = 0;

    virtual std::any visitControlBody(CompiscriptParser::ControlBodyContext *context) = 0;

    virtual std::any visitVariableDeclaration(CompiscriptParser::VariableDeclarationContext *context) = 0;

    virtual std::any visitConstantDeclaration(CompiscriptParser::ConstantDeclarationContext *context) = 0;

    virtual std::any visitTypeAnnotation(CompiscriptParser::TypeAnnotationContext *context) = 0;

    virtual std::any visitInitializer(CompiscriptParser::InitializerContext *context) = 0;

    virtual std::any visitAssignmentStatement(CompiscriptParser::AssignmentStatementContext *context) = 0;

    virtual std::any visitExpressionStatement(CompiscriptParser::ExpressionStatementContext *context) = 0;

    virtual std::any visitPrintStatement(CompiscriptParser::PrintStatementContext *context) = 0;

    virtual std::any visitIfStatement(CompiscriptParser::IfStatementContext *context) = 0;

    virtual std::any visitWhileStatement(CompiscriptParser::WhileStatementContext *context) = 0;

    virtual std::any visitDoWhileStatement(CompiscriptParser::DoWhileStatementContext *context) = 0;

    virtual std::any visitForStatement(CompiscriptParser::ForStatementContext *context) = 0;

    virtual std::any visitForeachStatement(CompiscriptParser::ForeachStatementContext *context) = 0;

    virtual std::any visitBreakStatement(CompiscriptParser::BreakStatementContext *context) = 0;

    virtual std::any visitContinueStatement(CompiscriptParser::ContinueStatementContext *context) = 0;

    virtual std::any visitReturnStatement(CompiscriptParser::ReturnStatementContext *context) = 0;

    virtual std::any visitTryCatchStatement(CompiscriptParser::TryCatchStatementContext *context) = 0;

    virtual std::any visitSwitchStatement(CompiscriptParser::SwitchStatementContext *context) = 0;

    virtual std::any visitSwitchCase(CompiscriptParser::SwitchCaseContext *context) = 0;

    virtual std::any visitDefaultCase(CompiscriptParser::DefaultCaseContext *context) = 0;

    virtual std::any visitFunctionDeclaration(CompiscriptParser::FunctionDeclarationContext *context) = 0;

    virtual std::any visitParameters(CompiscriptParser::ParametersContext *context) = 0;

    virtual std::any visitParameter(CompiscriptParser::ParameterContext *context) = 0;

    virtual std::any visitClassDeclaration(CompiscriptParser::ClassDeclarationContext *context) = 0;

    virtual std::any visitClassMember(CompiscriptParser::ClassMemberContext *context) = 0;

    virtual std::any visitExpression(CompiscriptParser::ExpressionContext *context) = 0;

    virtual std::any visitAssignExpr(CompiscriptParser::AssignExprContext *context) = 0;

    virtual std::any visitExprNoAssign(CompiscriptParser::ExprNoAssignContext *context) = 0;

    virtual std::any visitConditionalExpr(CompiscriptParser::ConditionalExprContext *context) = 0;

    virtual std::any visitLogicalOrExpr(CompiscriptParser::LogicalOrExprContext *context) = 0;

    virtual std::any visitLogicalAndExpr(CompiscriptParser::LogicalAndExprContext *context) = 0;

    virtual std::any visitEqualityExpr(CompiscriptParser::EqualityExprContext *context) = 0;

    virtual std::any visitRelationalExpr(CompiscriptParser::RelationalExprContext *context) = 0;

    virtual std::any visitAdditiveExpr(CompiscriptParser::AdditiveExprContext *context) = 0;

    virtual std::any visitMultiplicativeExpr(CompiscriptParser::MultiplicativeExprContext *context) = 0;

    virtual std::any visitUnaryExpr(CompiscriptParser::UnaryExprContext *context) = 0;

    virtual std::any visitPrimaryExpr(CompiscriptParser::PrimaryExprContext *context) = 0;

    virtual std::any visitLiteralExpr(CompiscriptParser::LiteralExprContext *context) = 0;

    virtual std::any visitLeftHandSide(CompiscriptParser::LeftHandSideContext *context) = 0;

    virtual std::any visitIdentifierExpr(CompiscriptParser::IdentifierExprContext *context) = 0;

    virtual std::any visitNewExpr(CompiscriptParser::NewExprContext *context) = 0;

    virtual std::any visitThisExpr(CompiscriptParser::ThisExprContext *context) = 0;

    virtual std::any visitCallExpr(CompiscriptParser::CallExprContext *context) = 0;

    virtual std::any visitIndexExpr(CompiscriptParser::IndexExprContext *context) = 0;

    virtual std::any visitPropertyAccessExpr(CompiscriptParser::PropertyAccessExprContext *context) = 0;

    virtual std::any visitArguments(CompiscriptParser::ArgumentsContext *context) = 0;

    virtual std::any visitArrayLiteral(CompiscriptParser::ArrayLiteralContext *context) = 0;

    virtual std::any visitType(CompiscriptParser::TypeContext *context) = 0;

    virtual std::any visitBaseType(CompiscriptParser::BaseTypeContext *context) = 0;


};

