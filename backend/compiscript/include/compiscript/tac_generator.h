// Segunda pasada del compilador: parse tree validado -> código de tres
// direcciones (TAC) y diseño de registros de activación.
#pragma once

#include "CompiscriptBaseVisitor.h"
#include "compiscript/model.h"
#include "compiscript/semantic_analyzer.h"

#include <any>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace compiscript {

enum class TacLocation { None, Variable, Index, Field };

struct TacValue {
  Type type = TYPE_UNKNOWN;
  std::string place;
  int symbol_id = -1;
  TacLocation location = TacLocation::None;
  std::string base;
  std::string index;
  std::string member;
  std::string receiver;

  TacValue() = default;
  TacValue(Type value_type, std::string value_place = {}, int value_symbol = -1,
           TacLocation value_location = TacLocation::None)
      : type(std::move(value_type)), place(std::move(value_place)),
        symbol_id(value_symbol), location(value_location) {}
};

class TacGenerator final : public CompiscriptBaseVisitor {
public:
  TacGenerator(SemanticResult &semantic, const SemanticBindings &bindings);
  TacResult generate(CompiscriptParser::ProgramContext *program);

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
  struct TempPool {
    int next_id = 0;
    std::vector<std::string> free;
    std::unordered_set<std::string> live;
    size_t created = 0;
    size_t reused = 0;
    size_t max_live = 0;
  };

  SemanticResult &semantic_;
  const SemanticBindings &bindings_;
  std::vector<TacInstruction> instructions_;
  std::vector<Diagnostic> diagnostics_;
  std::vector<ActivationRecord> activation_records_;
  std::unordered_map<int, size_t> activation_by_function_;
  std::vector<int> function_stack_;
  std::vector<std::string> class_stack_;
  std::vector<std::string> break_targets_;
  std::vector<std::string> continue_targets_;
  std::vector<TempPool> temp_pools_;
  int label_counter_ = 0;
  size_t total_temporaries_created_ = 0;
  size_t total_temporaries_reused_ = 0;
  size_t max_live_temporaries_ = 0;

  TacValue valueOf(const std::any &value) const;
  TacValue generateExpression(antlr4::tree::ParseTree *node);
  Type expressionType(const void *node) const;
  int boundSymbol(const void *node) const;
  const Symbol *symbol(int id) const;
  Symbol *symbol(int id);
  std::vector<std::string>
  operators(antlr4::ParserRuleContext *ctx,
            const std::set<std::string> &accepted) const;

  void layoutSymbols();
  size_t typeSize(const Type &type) const;
  size_t aligned(size_t value, size_t alignment = 8) const;
  int enclosingFunction(int scope_id) const;
  std::string storageName(int symbol_id) const;
  std::string functionLabel(int symbol_id) const;
  int findClassMember(const std::string &class_name, const std::string &name,
                      bool methods) const;

  std::string newTemp();
  void releaseTemp(const std::string &name);
  void releaseValue(const TacValue &value);
  std::string newLabel(const std::string &prefix = "L");
  void pushTempPool();
  void popTempPool(int function_symbol_id);

  void emit(const std::string &op, const std::string &result = {},
            const std::string &arg1 = {}, const std::string &arg2 = {},
            antlr4::ParserRuleContext *ctx = nullptr);
  std::string format(const TacInstruction &instruction) const;
  void report(const std::string &code, const std::string &message,
              antlr4::ParserRuleContext *ctx);
  void store(const TacValue &target, const TacValue &value,
             antlr4::ParserRuleContext *ctx);
  TacValue emitBinary(const std::string &op, const TacValue &left,
                      const TacValue &right, antlr4::ParserRuleContext *ctx);
  std::vector<TacValue>
  generateArguments(CompiscriptParser::ArgumentsContext *ctx);
  TacValue emitCall(const TacValue &callee,
                    CompiscriptParser::CallExprContext *ctx);
};

} // namespace compiscript
