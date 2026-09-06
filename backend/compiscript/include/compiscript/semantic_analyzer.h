#pragma once

#include "CompiscriptBaseVisitor.h"
#include "compiscript/model.h"

#include <any>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace compiscript {

struct ExprResult {
  Type type = TYPE_UNKNOWN;
  int symbol_id = -1;
  bool assignable = false;
};

class SemanticAnalyzer final : public CompiscriptBaseVisitor {
public:
  SemanticAnalyzer();
  SemanticResult analyze(CompiscriptParser::ProgramContext *program);

  std::any visitProgram(CompiscriptParser::ProgramContext *ctx) override;
  std::any visitBlock(CompiscriptParser::BlockContext *ctx) override;
  std::any
  visitControlBody(CompiscriptParser::ControlBodyContext *ctx) override;
  std::any visitVariableDeclaration(
      CompiscriptParser::VariableDeclarationContext *ctx) override;
  std::any visitConstantDeclaration(
      CompiscriptParser::ConstantDeclarationContext *ctx) override;
  std::any visitAssignmentStatement(
      CompiscriptParser::AssignmentStatementContext *ctx) override;
  std::any visitExpressionStatement(
      CompiscriptParser::ExpressionStatementContext *ctx) override;
  std::any
  visitPrintStatement(CompiscriptParser::PrintStatementContext *ctx) override;
  std::any
  visitIfStatement(CompiscriptParser::IfStatementContext *ctx) override;
  std::any
  visitWhileStatement(CompiscriptParser::WhileStatementContext *ctx) override;
  std::any visitDoWhileStatement(
      CompiscriptParser::DoWhileStatementContext *ctx) override;
  std::any
  visitForStatement(CompiscriptParser::ForStatementContext *ctx) override;
  std::any visitForeachStatement(
      CompiscriptParser::ForeachStatementContext *ctx) override;
  std::any
  visitBreakStatement(CompiscriptParser::BreakStatementContext *ctx) override;
  std::any visitContinueStatement(
      CompiscriptParser::ContinueStatementContext *ctx) override;
  std::any
  visitReturnStatement(CompiscriptParser::ReturnStatementContext *ctx) override;
  std::any visitTryCatchStatement(
      CompiscriptParser::TryCatchStatementContext *ctx) override;
  std::any
  visitSwitchStatement(CompiscriptParser::SwitchStatementContext *ctx) override;
  std::any visitFunctionDeclaration(
      CompiscriptParser::FunctionDeclarationContext *ctx) override;
  std::any visitClassDeclaration(
      CompiscriptParser::ClassDeclarationContext *ctx) override;
  std::any
  visitClassMember(CompiscriptParser::ClassMemberContext *ctx) override;

  std::any visitExpression(CompiscriptParser::ExpressionContext *ctx) override;
  std::any visitAssignExpr(CompiscriptParser::AssignExprContext *ctx) override;
  std::any
  visitExprNoAssign(CompiscriptParser::ExprNoAssignContext *ctx) override;
  std::any
  visitConditionalExpr(CompiscriptParser::ConditionalExprContext *ctx) override;
  std::any
  visitLogicalOrExpr(CompiscriptParser::LogicalOrExprContext *ctx) override;
  std::any
  visitLogicalAndExpr(CompiscriptParser::LogicalAndExprContext *ctx) override;
  std::any
  visitEqualityExpr(CompiscriptParser::EqualityExprContext *ctx) override;
  std::any
  visitRelationalExpr(CompiscriptParser::RelationalExprContext *ctx) override;
  std::any
  visitAdditiveExpr(CompiscriptParser::AdditiveExprContext *ctx) override;
  std::any visitMultiplicativeExpr(
      CompiscriptParser::MultiplicativeExprContext *ctx) override;
  std::any visitUnaryExpr(CompiscriptParser::UnaryExprContext *ctx) override;
  std::any
  visitPrimaryExpr(CompiscriptParser::PrimaryExprContext *ctx) override;
  std::any
  visitLiteralExpr(CompiscriptParser::LiteralExprContext *ctx) override;
  std::any
  visitArrayLiteral(CompiscriptParser::ArrayLiteralContext *ctx) override;
  std::any
  visitLeftHandSide(CompiscriptParser::LeftHandSideContext *ctx) override;
  std::any
  visitIdentifierExpr(CompiscriptParser::IdentifierExprContext *ctx) override;
  std::any visitNewExpr(CompiscriptParser::NewExprContext *ctx) override;
  std::any visitThisExpr(CompiscriptParser::ThisExprContext *ctx) override;

private:
  std::vector<Scope> scopes_;
  std::vector<Symbol> symbols_;
  std::map<std::string, ClassInfo> classes_;
  std::vector<Diagnostic> diagnostics_;
  int current_scope_ = 0;
  std::vector<int> function_stack_;
  std::vector<std::string> class_stack_;
  int loop_depth_ = 0;
  std::unordered_map<const void *, int> declaration_symbols_;
  std::set<const void *> visited_declarations_;

  int newScope(const std::string &name, const std::string &kind, int parent_id,
               size_t line, int owner_symbol_id = -1);
  int declareSymbol(const std::string &name, const std::string &kind,
                    const Type &type, antlr4::ParserRuleContext *ctx,
                    bool mutable_value = true, bool initialized = true,
                    int scope_id = -1);
  int resolveSymbol(const std::string &name, antlr4::ParserRuleContext *ctx,
                    bool report = true);
  void recordCapture(int symbol_id);
  bool isDescendantScope(int scope_id, int ancestor_id) const;
  void error(const std::string &code, const std::string &category,
             const std::string &message, antlr4::ParserRuleContext *ctx);
  void requireType(const Type &actual, const Type &expected,
                   antlr4::ParserRuleContext *ctx, const std::string &message,
                   const std::string &code = "SEM_TYPE_MISMATCH");
  void checkKnownType(const Type &type, antlr4::ParserRuleContext *ctx,
                      bool allow_void = false);
  std::set<std::string> classNames() const;
  bool compatible(const Type &target, const Type &value) const;

  Type typeOf(CompiscriptParser::TypeContext *ctx) const;
  ExprResult expressionResult(const std::any &value) const;
  ExprResult visitExpressionNode(antlr4::tree::ParseTree *node);
  std::vector<ExprResult> arguments(CompiscriptParser::ArgumentsContext *ctx);
  std::vector<std::string>
  operators(antlr4::ParserRuleContext *ctx,
            const std::set<std::string> &accepted) const;

  void predeclareStatements(
      const std::vector<CompiscriptParser::StatementContext *> &statements);
  int predeclareFunction(CompiscriptParser::FunctionDeclarationContext *ctx,
                         const std::string &kind = "function",
                         int scope_id = -1);
  int predeclareClass(CompiscriptParser::ClassDeclarationContext *ctx);
  void prepareClassMembers(CompiscriptParser::ClassDeclarationContext *ctx);
  bool analyzeStatements(
      const std::vector<CompiscriptParser::StatementContext *> &statements);
  bool terminates(CompiscriptParser::StatementContext *statement) const;
  bool bodyTerminates(CompiscriptParser::ControlBodyContext *body) const;

  ExprResult validateAssignment(const ExprResult &target,
                                const ExprResult &value,
                                antlr4::ParserRuleContext *ctx);
  void validateCallArguments(int function_symbol,
                             const std::vector<ExprResult> &values,
                             antlr4::ParserRuleContext *ctx);
  int findMember(const std::string &class_name, const std::string &name,
                 bool methods) const;
  ExprResult applyProperty(const ExprResult &owner, const std::string &name,
                           antlr4::ParserRuleContext *ctx);
  ExprResult applyCall(const ExprResult &callee,
                       CompiscriptParser::CallExprContext *ctx);
  void validateInheritance(antlr4::ParserRuleContext *ctx);
};

} // namespace compiscript
