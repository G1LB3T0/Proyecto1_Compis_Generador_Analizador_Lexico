// Visitor principal: recorre el árbol de ANTLR y aplica todas las reglas
// semánticas exigidas para tipos, ámbitos, funciones, flujo, clases y listas.
#include "compiscript/semantic_analyzer.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace compiscript {

namespace {

size_t lineOf(antlr4::ParserRuleContext *ctx) {
  return ctx && ctx->getStart() ? ctx->getStart()->getLine() : 0;
}

size_t columnOf(antlr4::ParserRuleContext *ctx) {
  return ctx && ctx->getStart() ? ctx->getStart()->getCharPositionInLine() + 1
                                : 0;
}

} // namespace

// PUNTO DE ENTRADA SEMÁNTICO
// Todo análisis comienza con un entorno global y termina reuniendo las tablas y
// diagnósticos que consumirá el frontend.
SemanticAnalyzer::SemanticAnalyzer() { newScope("global", "global", -1, 1); }

SemanticResult
SemanticAnalyzer::analyze(CompiscriptParser::ProgramContext *program) {
  visit(program);
  SemanticResult result;
  result.ok = diagnostics_.empty();
  result.diagnostics = diagnostics_;
  result.scopes = scopes_;
  result.symbols = symbols_;
  for (const auto &entry : classes_)
    result.classes.push_back(entry.second);
  return result;
}

// MANEJO DE ÁMBITOS Y TABLA DE SÍMBOLOS
// Los ámbitos forman una jerarquía padre-hijo. Las declaraciones se guardan en
// el ámbito actual y las búsquedas avanzan hacia los ámbitos exteriores.
int SemanticAnalyzer::newScope(const std::string &name, const std::string &kind,
                               int parent_id, size_t line,
                               int owner_symbol_id) {
  Scope scope;
  scope.id = static_cast<int>(scopes_.size());
  scope.name = name;
  scope.kind = kind;
  scope.parent_id = parent_id;
  scope.owner_symbol_id = owner_symbol_id;
  scope.line = line;
  scopes_.push_back(scope);
  if (parent_id >= 0)
    scopes_[parent_id].children.push_back(scope.id);
  return scope.id;
}

void SemanticAnalyzer::error(const std::string &code,
                             const std::string &category,
                             const std::string &message,
                             antlr4::ParserRuleContext *ctx) {
  diagnostics_.push_back(
      {code, category, "error", message, lineOf(ctx), columnOf(ctx)});
}

int SemanticAnalyzer::declareSymbol(const std::string &name,
                                    const std::string &kind, const Type &type,
                                    antlr4::ParserRuleContext *ctx,
                                    bool mutable_value, bool initialized,
                                    int scope_id) {
  const int destination = scope_id >= 0 ? scope_id : current_scope_;
  auto existing = scopes_[destination].symbols.find(name);
  if (existing != scopes_[destination].symbols.end()) {
    const Symbol &previous = symbols_[existing->second];
    error("SEM_SCOPE_DUPLICATE", "ámbito",
          "El identificador '" + name +
              "' ya fue declarado en este ámbito (línea " +
              std::to_string(previous.line) + ").",
          ctx);
    return -1;
  }
  Symbol symbol;
  symbol.id = static_cast<int>(symbols_.size());
  symbol.name = name;
  symbol.kind = kind;
  symbol.type = type;
  symbol.scope_id = destination;
  symbol.line = lineOf(ctx);
  symbol.column = columnOf(ctx);
  symbol.mutable_value = mutable_value;
  symbol.initialized = initialized;
  symbols_.push_back(symbol);
  scopes_[destination].symbols[name] = symbol.id;
  return symbol.id;
}

bool SemanticAnalyzer::isDescendantScope(int scope_id, int ancestor_id) const {
  while (scope_id >= 0) {
    if (scope_id == ancestor_id)
      return true;
    scope_id = scopes_[scope_id].parent_id;
  }
  return false;
}

void SemanticAnalyzer::recordCapture(int symbol_id) {
  if (function_stack_.empty())
    return;
  Symbol &captured = symbols_[symbol_id];
  if (captured.kind != "variable" && captured.kind != "constant" &&
      captured.kind != "parameter") {
    return;
  }
  Symbol &function = symbols_[function_stack_.back()];
  if (function.function_scope_id < 0)
    return;
  if (!isDescendantScope(captured.scope_id, function.function_scope_id)) {
    function.captures.insert(captured.id);
    captured.captured_by.insert(function.id);
  }
}

int SemanticAnalyzer::resolveSymbol(const std::string &name,
                                    antlr4::ParserRuleContext *ctx,
                                    bool report) {
  int scope_id = current_scope_;
  while (scope_id >= 0) {
    auto found = scopes_[scope_id].symbols.find(name);
    if (found != scopes_[scope_id].symbols.end()) {
      recordCapture(found->second);
      return found->second;
    }
    scope_id = scopes_[scope_id].parent_id;
  }
  if (report) {
    error("SEM_SCOPE_UNDECLARED", "ámbito",
          "El identificador '" + name +
              "' no está declarado en un ámbito accesible.",
          ctx);
  }
  return -1;
}

// SISTEMA DE TIPOS
// Estas ayudas validan tipos conocidos y compatibilidad, incluyendo promoción
// de integer a float, listas y relaciones de herencia entre clases.
std::set<std::string> SemanticAnalyzer::classNames() const {
  std::set<std::string> names;
  for (const auto &item : classes_)
    names.insert(item.first);
  return names;
}

bool SemanticAnalyzer::compatible(const Type &target, const Type &value) const {
  if (assignable(target, value, classNames()))
    return true;
  if (target.isList() && value.isList()) {
    return compatible(target.element(), value.element());
  }
  if (classes_.count(target.name) == 0 || classes_.count(value.name) == 0) {
    return false;
  }
  std::set<std::string> visited;
  std::string current = value.name;
  while (classes_.count(current) && !visited.count(current)) {
    if (current == target.name)
      return true;
    visited.insert(current);
    current = classes_.at(current).base;
  }
  return false;
}

void SemanticAnalyzer::requireType(const Type &actual, const Type &expected,
                                   antlr4::ParserRuleContext *ctx,
                                   const std::string &message,
                                   const std::string &code) {
  if (!actual.isUnknown() && actual != expected) {
    error(code, "tipo", message + "; se recibió '" + actual.name + "'.", ctx);
  }
}

void SemanticAnalyzer::checkKnownType(const Type &type,
                                      antlr4::ParserRuleContext *ctx,
                                      bool allow_void) {
  std::string base = type.name;
  while (base.size() >= 2 && base.substr(base.size() - 2) == "[]") {
    base.resize(base.size() - 2);
  }
  std::set<std::string> known = {"integer", "float", "string", "boolean"};
  const auto class_names = classNames();
  known.insert(class_names.begin(), class_names.end());
  if (allow_void)
    known.insert("void");
  if (base != "unknown" && known.count(base) == 0) {
    error("SEM_TYPE_UNKNOWN", "tipo", "El tipo '" + base + "' no existe.", ctx);
  }
  if (base == "void" && type.isList()) {
    error("SEM_TYPE_VOID", "tipo", "No se permiten listas de void.", ctx);
  }
}

Type SemanticAnalyzer::typeOf(CompiscriptParser::TypeContext *ctx) const {
  return ctx ? Type{ctx->getText()} : TYPE_UNKNOWN;
}

ExprResult SemanticAnalyzer::expressionResult(const std::any &value) const {
  if (value.has_value() && value.type() == typeid(ExprResult)) {
    return std::any_cast<ExprResult>(value);
  }
  return {};
}

ExprResult
SemanticAnalyzer::visitExpressionNode(antlr4::tree::ParseTree *node) {
  return node ? expressionResult(visit(node)) : ExprResult{};
}

std::vector<std::string>
SemanticAnalyzer::operators(antlr4::ParserRuleContext *ctx,
                            const std::set<std::string> &accepted) const {
  std::vector<std::string> result;
  for (auto *child : ctx->children) {
    const std::string text = child->getText();
    if (accepted.count(text))
      result.push_back(text);
  }
  return result;
}

// FUNCIONES, PROCEDIMIENTOS Y CLASES
// Se registran antes de visitar sus cuerpos para permitir recursividad,
// referencias adelantadas y validación de miembros.
int SemanticAnalyzer::predeclareClass(
    CompiscriptParser::ClassDeclarationContext *ctx) {
  auto known = declaration_symbols_.find(ctx);
  if (known != declaration_symbols_.end())
    return known->second;
  const auto identifiers = ctx->Identifier();
  const std::string name = identifiers.front()->getText();
  const std::string base =
      identifiers.size() > 1 ? identifiers[1]->getText() : "";
  const int symbol_id = declareSymbol(name, "class", Type{name}, ctx, false);
  declaration_symbols_[ctx] = symbol_id;
  if (symbol_id >= 0) {
    Symbol &symbol = symbols_[symbol_id];
    symbol.base_class = base;
    ClassInfo info;
    info.name = name;
    info.base = base;
    info.symbol_id = symbol_id;
    info.scope_id = newScope("clase " + name, "class", current_scope_,
                             lineOf(ctx), symbol_id);
    classes_[name] = info;
  }
  return symbol_id;
}

int SemanticAnalyzer::predeclareFunction(
    CompiscriptParser::FunctionDeclarationContext *ctx, const std::string &kind,
    int scope_id) {
  auto known = declaration_symbols_.find(ctx);
  if (known != declaration_symbols_.end())
    return known->second;
  const int symbol_id =
      declareSymbol(ctx->Identifier()->getText(), kind, TYPE_FUNCTION, ctx,
                    false, true, scope_id);
  declaration_symbols_[ctx] = symbol_id;
  if (symbol_id < 0)
    return -1;
  Symbol &symbol = symbols_[symbol_id];
  if (ctx->parameters()) {
    for (auto *parameter : ctx->parameters()->parameter()) {
      symbol.parameter_names.push_back(parameter->Identifier()->getText());
      symbol.parameter_types.push_back(typeOf(parameter->type()));
    }
  }
  symbol.return_type = ctx->type() ? typeOf(ctx->type()) : TYPE_VOID;
  return symbol_id;
}

void SemanticAnalyzer::prepareClassMembers(
    CompiscriptParser::ClassDeclarationContext *ctx) {
  auto declaration = declaration_symbols_.find(ctx);
  if (declaration == declaration_symbols_.end() || declaration->second < 0)
    return;
  const std::string class_name = ctx->Identifier(0)->getText();
  ClassInfo &info = classes_[class_name];
  for (auto *member : ctx->classMember()) {
    if (auto *function = member->functionDeclaration()) {
      const int id = predeclareFunction(function, "method", info.scope_id);
      if (id >= 0) {
        symbols_[id].owner_class = class_name;
        info.methods.emplace(symbols_[id].name, id);
      }
      continue;
    }
    auto *variable = member->variableDeclaration();
    auto *constant = member->constantDeclaration();
    antlr4::ParserRuleContext *declaration_ctx =
        variable ? static_cast<antlr4::ParserRuleContext *>(variable)
                 : static_cast<antlr4::ParserRuleContext *>(constant);
    auto existing_declaration = declaration_symbols_.find(declaration_ctx);
    if (existing_declaration != declaration_symbols_.end()) {
      const int existing_id = existing_declaration->second;
      if (existing_id >= 0)
        info.fields.emplace(symbols_[existing_id].name, existing_id);
      continue;
    }
    const std::string name = variable ? variable->Identifier()->getText()
                                      : constant->Identifier()->getText();
    const Type type = variable
                          ? (variable->typeAnnotation()
                                 ? typeOf(variable->typeAnnotation()->type())
                                 : TYPE_UNKNOWN)
                          : (constant->typeAnnotation()
                                 ? typeOf(constant->typeAnnotation()->type())
                                 : TYPE_UNKNOWN);
    const int id =
        declareSymbol(name, "field", type, declaration_ctx, variable != nullptr,
                      constant != nullptr, info.scope_id);
    declaration_symbols_[declaration_ctx] = id;
    if (id >= 0)
      info.fields.emplace(name, id);
  }
}

void SemanticAnalyzer::predeclareStatements(
    const std::vector<CompiscriptParser::StatementContext *> &statements) {
  for (auto *statement : statements) {
    if (statement->classDeclaration())
      predeclareClass(statement->classDeclaration());
    if (statement->functionDeclaration())
      predeclareFunction(statement->functionDeclaration());
  }
}

// REGLAS GENERALES: CÓDIGO MUERTO
// Una sentencia posterior a return, break o continue se reporta como
// inalcanzable; un if termina solo cuando terminan sus dos ramas.
bool SemanticAnalyzer::bodyTerminates(
    CompiscriptParser::ControlBodyContext *body) const {
  if (body->statement())
    return terminates(body->statement());
  const auto statements = body->block()->statement();
  return !statements.empty() && terminates(statements.back());
}

bool SemanticAnalyzer::terminates(
    CompiscriptParser::StatementContext *statement) const {
  if (statement->returnStatement() || statement->breakStatement() ||
      statement->continueStatement()) {
    return true;
  }
  if (statement->block()) {
    const auto nested = statement->block()->statement();
    return !nested.empty() && terminates(nested.back());
  }
  if (statement->ifStatement()) {
    const auto bodies = statement->ifStatement()->controlBody();
    return bodies.size() == 2 && bodyTerminates(bodies[0]) &&
           bodyTerminates(bodies[1]);
  }
  return false;
}

bool SemanticAnalyzer::analyzeStatements(
    const std::vector<CompiscriptParser::StatementContext *> &statements) {
  predeclareStatements(statements);
  bool terminated = false;
  for (auto *statement : statements) {
    if (terminated) {
      error("SEM_FLOW_DEAD_CODE", "flujo",
            "Código inalcanzable después de una instrucción que termina el "
            "flujo.",
            statement);
    }
    visit(statement);
    terminated = terminated || terminates(statement);
  }
  return terminated;
}

// RECORRIDO DEL PROGRAMA Y CREACIÓN DE ÁMBITOS DE BLOQUE
std::any
SemanticAnalyzer::visitProgram(CompiscriptParser::ProgramContext *ctx) {
  predeclareStatements(ctx->statement());
  for (auto *statement : ctx->statement()) {
    if (statement->classDeclaration())
      prepareClassMembers(statement->classDeclaration());
  }
  analyzeStatements(ctx->statement());
  validateInheritance(ctx);
  return ExprResult{};
}

std::any SemanticAnalyzer::visitBlock(CompiscriptParser::BlockContext *ctx) {
  const int parent = current_scope_;
  current_scope_ = newScope("bloque@" + std::to_string(lineOf(ctx)), "block",
                            parent, lineOf(ctx));
  analyzeStatements(ctx->statement());
  current_scope_ = parent;
  return ExprResult{};
}

std::any
SemanticAnalyzer::visitControlBody(CompiscriptParser::ControlBodyContext *ctx) {
  return ctx->block() ? visit(ctx->block()) : visit(ctx->statement());
}

// SISTEMA DE TIPOS EN DECLARACIONES
// Se infiere o comprueba el tipo inicial y se impide usar void en variables o
// constantes. La gramática hace obligatoria la inicialización de const.
std::any SemanticAnalyzer::visitVariableDeclaration(
    CompiscriptParser::VariableDeclarationContext *ctx) {
  const std::string name = ctx->Identifier()->getText();
  const Type declared = ctx->typeAnnotation()
                            ? typeOf(ctx->typeAnnotation()->type())
                            : TYPE_UNKNOWN;
  if (ctx->typeAnnotation()) {
    checkKnownType(declared, ctx->typeAnnotation());
    if (declared == TYPE_VOID) {
      error("SEM_TYPE_VOID", "tipo", "Una variable no puede ser de tipo void.",
            ctx);
    }
  }
  const ExprResult value =
      ctx->initializer() ? visitExpressionNode(ctx->initializer()->expression())
                         : ExprResult{};
  const Type final_type = declared.isUnknown() ? value.type : declared;
  if (ctx->typeAnnotation() && ctx->initializer() &&
      !compatible(declared, value.type)) {
    error("SEM_ASSIGN_TYPE", "tipo",
          "No se puede inicializar '" + name + "' de tipo '" + declared.name +
              "' con '" + value.type.name + "'.",
          ctx);
  }

  int symbol_id = -1;
  auto predeclared = declaration_symbols_.find(ctx);
  if (predeclared != declaration_symbols_.end()) {
    symbol_id = predeclared->second;
    if (symbol_id >= 0) {
      symbols_[symbol_id].type = final_type;
      symbols_[symbol_id].initialized = ctx->initializer() != nullptr;
    }
  } else {
    symbol_id = declareSymbol(name, "variable", final_type, ctx, true,
                              ctx->initializer() != nullptr);
  }
  return ExprResult{};
}

std::any SemanticAnalyzer::visitConstantDeclaration(
    CompiscriptParser::ConstantDeclarationContext *ctx) {
  const std::string name = ctx->Identifier()->getText();
  const Type declared = ctx->typeAnnotation()
                            ? typeOf(ctx->typeAnnotation()->type())
                            : TYPE_UNKNOWN;
  if (ctx->typeAnnotation()) {
    checkKnownType(declared, ctx->typeAnnotation());
    if (declared == TYPE_VOID) {
      error("SEM_TYPE_VOID", "tipo", "Una constante no puede ser de tipo void.",
            ctx);
    }
  }
  const ExprResult value = visitExpressionNode(ctx->expression());
  const Type final_type = declared.isUnknown() ? value.type : declared;
  if (ctx->typeAnnotation() && !compatible(declared, value.type)) {
    error("SEM_ASSIGN_TYPE", "tipo",
          "No se puede inicializar la constante '" + name + "' de tipo '" +
              declared.name + "' con '" + value.type.name + "'.",
          ctx);
  }

  auto predeclared = declaration_symbols_.find(ctx);
  if (predeclared != declaration_symbols_.end()) {
    if (predeclared->second >= 0)
      symbols_[predeclared->second].type = final_type;
  } else {
    declareSymbol(name, "constant", final_type, ctx, false, true);
  }
  return ExprResult{};
}

// FUNCIONES Y PROCEDIMIENTOS
// Crea el entorno de la función, declara parámetros y valida el tipo y la
// presencia de return. Un procedimiento se representa con retorno void.
std::any SemanticAnalyzer::visitFunctionDeclaration(
    CompiscriptParser::FunctionDeclarationContext *ctx) {
  const std::string kind = class_stack_.empty() ? "function" : "method";
  const int symbol_id = predeclareFunction(ctx, kind);
  if (symbol_id < 0 || visited_declarations_.count(ctx))
    return ExprResult{};
  visited_declarations_.insert(ctx);
  if (!class_stack_.empty())
    symbols_[symbol_id].owner_class = class_stack_.back();
  const std::string function_name = symbols_[symbol_id].name;
  const Type return_type = symbols_[symbol_id].return_type;
  const std::vector<std::string> parameter_names =
      symbols_[symbol_id].parameter_names;
  const std::vector<Type> parameter_types = symbols_[symbol_id].parameter_types;
  antlr4::ParserRuleContext *return_context =
      ctx->type() ? static_cast<antlr4::ParserRuleContext *>(ctx->type())
                  : static_cast<antlr4::ParserRuleContext *>(ctx);
  checkKnownType(return_type, return_context, true);

  const int parent = current_scope_;
  current_scope_ = newScope("función " + function_name, "function", parent,
                            lineOf(ctx), symbol_id);
  symbols_[symbol_id].function_scope_id = current_scope_;
  function_stack_.push_back(symbol_id);

  const auto parameters =
      ctx->parameters() ? ctx->parameters()->parameter()
                        : std::vector<CompiscriptParser::ParameterContext *>{};
  for (size_t index = 0; index < parameters.size(); ++index) {
    auto *parameter = parameters[index];
    const Type type = parameter_types[index];
    checkKnownType(type, parameter);
    if (type == TYPE_VOID) {
      error("SEM_TYPE_VOID", "tipo", "Un parámetro no puede ser void.",
            parameter);
    }
    declareSymbol(parameter_names[index], "parameter", type, parameter);
  }
  visit(ctx->block());
  if (return_type != TYPE_VOID) {
    const auto statements = ctx->block()->statement();
    if (statements.empty() || !terminates(statements.back())) {
      error("SEM_FUNCTION_MISSING_RETURN", "función",
            "La función '" + function_name +
                "' debe retornar un valor de tipo '" + return_type.name +
                "' en todos sus caminos.",
            ctx);
    }
  }
  function_stack_.pop_back();
  current_scope_ = parent;
  return ExprResult{};
}

// CLASES Y OBJETOS
// Activa el entorno de clase para analizar sus atributos, métodos y uso de this.
std::any SemanticAnalyzer::visitClassDeclaration(
    CompiscriptParser::ClassDeclarationContext *ctx) {
  const int symbol_id = predeclareClass(ctx);
  if (symbol_id < 0 || visited_declarations_.count(ctx))
    return ExprResult{};
  visited_declarations_.insert(ctx);
  const std::string name = ctx->Identifier(0)->getText();
  auto found = classes_.find(name);
  if (found == classes_.end())
    return ExprResult{};
  prepareClassMembers(ctx);
  const int parent = current_scope_;
  current_scope_ = found->second.scope_id;
  class_stack_.push_back(name);
  for (auto *member : ctx->classMember())
    visit(member);
  class_stack_.pop_back();
  current_scope_ = parent;
  return ExprResult{};
}

std::any
SemanticAnalyzer::visitClassMember(CompiscriptParser::ClassMemberContext *ctx) {
  if (ctx->functionDeclaration())
    return visit(ctx->functionDeclaration());
  if (ctx->variableDeclaration())
    return visit(ctx->variableDeclaration());
  return visit(ctx->constantDeclaration());
}

// SISTEMA DE TIPOS EN ASIGNACIONES
// Comprueba que el destino sea modificable y que el valor sea compatible con
// el tipo declarado; también rechaza la reasignación de constantes.
ExprResult
SemanticAnalyzer::validateAssignment(const ExprResult &target,
                                     const ExprResult &value,
                                     antlr4::ParserRuleContext *ctx) {
  if (!target.assignable || target.symbol_id < 0) {
    error("SEM_ASSIGN_TARGET", "tipo",
          "El lado izquierdo de la asignación no es asignable.", ctx);
    return {TYPE_ERROR};
  }
  Symbol &symbol = symbols_[target.symbol_id];
  if (!symbol.mutable_value) {
    error("SEM_ASSIGN_CONST", "tipo",
          "No se puede reasignar la constante '" + symbol.name + "'.", ctx);
  }
  if (!compatible(target.type, value.type)) {
    error("SEM_ASSIGN_TYPE", "tipo",
          "No se puede asignar '" + value.type.name + "' a '" +
              target.type.name + "'.",
          ctx);
  }
  symbol.initialized = true;
  return {target.type};
}

std::any SemanticAnalyzer::visitAssignmentStatement(
    CompiscriptParser::AssignmentStatementContext *ctx) {
  validateAssignment(visitExpressionNode(ctx->leftHandSide()),
                     visitExpressionNode(ctx->expression()), ctx);
  return ExprResult{};
}

std::any SemanticAnalyzer::visitExpressionStatement(
    CompiscriptParser::ExpressionStatementContext *ctx) {
  return visit(ctx->expression());
}

std::any SemanticAnalyzer::visitPrintStatement(
    CompiscriptParser::PrintStatementContext *ctx) {
  visit(ctx->expression());
  return ExprResult{};
}

// CONTROL DE FLUJO
// Exige condiciones booleanas y usa loop_depth_ y function_stack_ para limitar
// correctamente break, continue y return.
std::any
SemanticAnalyzer::visitIfStatement(CompiscriptParser::IfStatementContext *ctx) {
  const ExprResult condition = visitExpressionNode(ctx->expression());
  requireType(condition.type, TYPE_BOOLEAN, ctx->expression(),
              "La condición de if debe ser boolean");
  for (auto *body : ctx->controlBody())
    visit(body);
  return ExprResult{};
}

std::any SemanticAnalyzer::visitWhileStatement(
    CompiscriptParser::WhileStatementContext *ctx) {
  const ExprResult condition = visitExpressionNode(ctx->expression());
  requireType(condition.type, TYPE_BOOLEAN, ctx->expression(),
              "La condición de while debe ser boolean");
  ++loop_depth_;
  visit(ctx->controlBody());
  --loop_depth_;
  return ExprResult{};
}

std::any SemanticAnalyzer::visitDoWhileStatement(
    CompiscriptParser::DoWhileStatementContext *ctx) {
  ++loop_depth_;
  visit(ctx->controlBody());
  --loop_depth_;
  const ExprResult condition = visitExpressionNode(ctx->expression());
  requireType(condition.type, TYPE_BOOLEAN, ctx->expression(),
              "La condición de do-while debe ser boolean");
  return ExprResult{};
}

std::any SemanticAnalyzer::visitForStatement(
    CompiscriptParser::ForStatementContext *ctx) {
  const int parent = current_scope_;
  current_scope_ = newScope("for@" + std::to_string(lineOf(ctx)), "block",
                            parent, lineOf(ctx));
  if (ctx->variableDeclaration())
    visit(ctx->variableDeclaration());
  if (ctx->expressionStatement())
    visit(ctx->expressionStatement());
  const auto expressions = ctx->expression();
  if (!expressions.empty()) {
    const ExprResult condition = visitExpressionNode(expressions[0]);
    requireType(condition.type, TYPE_BOOLEAN, expressions[0],
                "La condición de for debe ser boolean");
  }
  if (expressions.size() > 1)
    visit(expressions[1]);
  ++loop_depth_;
  visit(ctx->controlBody());
  --loop_depth_;
  current_scope_ = parent;
  return ExprResult{};
}

std::any SemanticAnalyzer::visitForeachStatement(
    CompiscriptParser::ForeachStatementContext *ctx) {
  const ExprResult collection = visitExpressionNode(ctx->expression());
  const int parent = current_scope_;
  current_scope_ = newScope("foreach@" + std::to_string(lineOf(ctx)), "block",
                            parent, lineOf(ctx));
  if (!collection.type.isList() && !collection.type.isUnknown()) {
    error("SEM_LIST_FOREACH", "lista", "foreach requiere una lista.",
          ctx->expression());
  }
  declareSymbol(ctx->Identifier()->getText(), "variable",
                collection.type.element(), ctx);
  ++loop_depth_;
  visit(ctx->controlBody());
  --loop_depth_;
  current_scope_ = parent;
  return ExprResult{};
}

std::any SemanticAnalyzer::visitBreakStatement(
    CompiscriptParser::BreakStatementContext *ctx) {
  if (loop_depth_ == 0) {
    error("SEM_FLOW_BREAK", "flujo",
          "break solo se permite dentro de un bucle.", ctx);
  }
  return ExprResult{};
}

std::any SemanticAnalyzer::visitContinueStatement(
    CompiscriptParser::ContinueStatementContext *ctx) {
  if (loop_depth_ == 0) {
    error("SEM_FLOW_CONTINUE", "flujo",
          "continue solo se permite dentro de un bucle.", ctx);
  }
  return ExprResult{};
}

std::any SemanticAnalyzer::visitReturnStatement(
    CompiscriptParser::ReturnStatementContext *ctx) {
  if (function_stack_.empty()) {
    error("SEM_FLOW_RETURN", "flujo",
          "return solo se permite dentro de una función.", ctx);
    if (ctx->expression())
      visit(ctx->expression());
    return ExprResult{};
  }
  const Symbol &function = symbols_[function_stack_.back()];
  const ExprResult value = ctx->expression()
                               ? visitExpressionNode(ctx->expression())
                               : ExprResult{TYPE_VOID};
  if (!compatible(function.return_type, value.type)) {
    error("SEM_FUNCTION_RETURN", "función",
          "La función '" + function.name + "' retorna '" + value.type.name +
              "', pero declaró '" + function.return_type.name + "'.",
          ctx);
  }
  return ExprResult{};
}

std::any SemanticAnalyzer::visitTryCatchStatement(
    CompiscriptParser::TryCatchStatementContext *ctx) {
  visit(ctx->block(0));
  const int parent = current_scope_;
  current_scope_ = newScope("catch@" + std::to_string(lineOf(ctx)), "block",
                            parent, lineOf(ctx));
  declareSymbol(ctx->Identifier()->getText(), "variable", TYPE_UNKNOWN, ctx);
  analyzeStatements(ctx->block(1)->statement());
  current_scope_ = parent;
  return ExprResult{};
}

std::any SemanticAnalyzer::visitSwitchStatement(
    CompiscriptParser::SwitchStatementContext *ctx) {
  const ExprResult selector = visitExpressionNode(ctx->expression());
  // El enunciado exige explícitamente condición booleana para switch.
  requireType(selector.type, TYPE_BOOLEAN, ctx->expression(),
              "La condición de switch debe ser boolean");
  for (auto *switch_case : ctx->switchCase()) {
    const ExprResult value = visitExpressionNode(switch_case->expression());
    if (!compatible(selector.type, value.type) &&
        !compatible(value.type, selector.type)) {
      error("SEM_SWITCH_CASE", "flujo",
            "El case de tipo '" + value.type.name +
                "' no coincide con el selector '" + selector.type.name + "'.",
            switch_case);
    }
    analyzeStatements(switch_case->statement());
  }
  if (ctx->defaultCase())
    analyzeStatements(ctx->defaultCase()->statement());
  return ExprResult{};
}

// SISTEMA DE TIPOS EN EXPRESIONES
// Cada visita obtiene los tipos de sus hijos y sintetiza el tipo resultante;
// aquí se validan operadores ternarios, lógicos, comparativos y aritméticos.
std::any
SemanticAnalyzer::visitExpression(CompiscriptParser::ExpressionContext *ctx) {
  return visit(ctx->assignmentExpr());
}

std::any
SemanticAnalyzer::visitAssignExpr(CompiscriptParser::AssignExprContext *ctx) {
  return validateAssignment(visitExpressionNode(ctx->leftHandSide()),
                            visitExpressionNode(ctx->assignmentExpr()), ctx);
}

std::any SemanticAnalyzer::visitExprNoAssign(
    CompiscriptParser::ExprNoAssignContext *ctx) {
  return visit(ctx->conditionalExpr());
}

std::any SemanticAnalyzer::visitConditionalExpr(
    CompiscriptParser::ConditionalExprContext *ctx) {
  const ExprResult condition = visitExpressionNode(ctx->logicalOrExpr());
  const auto branches = ctx->expression();
  if (branches.empty())
    return condition;
  requireType(condition.type, TYPE_BOOLEAN, ctx,
              "La condición ternaria debe ser boolean");
  const ExprResult left = visitExpressionNode(branches[0]);
  const ExprResult right = visitExpressionNode(branches[1]);
  const Type result = commonType(left.type, right.type);
  if (result == TYPE_ERROR) {
    error("SEM_TERNARY_TYPE", "tipo",
          "Las ramas ternarias '" + left.type.name + "' y '" + right.type.name +
              "' son incompatibles.",
          ctx);
  }
  return ExprResult{result};
}

std::any SemanticAnalyzer::visitLogicalOrExpr(
    CompiscriptParser::LogicalOrExprContext *ctx) {
  const auto operands = ctx->logicalAndExpr();
  ExprResult first = visitExpressionNode(operands.front());
  if (operands.size() == 1)
    return first;
  requireType(first.type, TYPE_BOOLEAN, ctx,
              "Los operandos de || deben ser boolean", "SEM_LOGICAL_TYPE");
  for (size_t index = 1; index < operands.size(); ++index) {
    const ExprResult value = visitExpressionNode(operands[index]);
    requireType(value.type, TYPE_BOOLEAN, ctx,
                "Los operandos de || deben ser boolean", "SEM_LOGICAL_TYPE");
  }
  return ExprResult{TYPE_BOOLEAN};
}

std::any SemanticAnalyzer::visitLogicalAndExpr(
    CompiscriptParser::LogicalAndExprContext *ctx) {
  const auto operands = ctx->equalityExpr();
  ExprResult first = visitExpressionNode(operands.front());
  if (operands.size() == 1)
    return first;
  requireType(first.type, TYPE_BOOLEAN, ctx,
              "Los operandos de && deben ser boolean", "SEM_LOGICAL_TYPE");
  for (size_t index = 1; index < operands.size(); ++index) {
    const ExprResult value = visitExpressionNode(operands[index]);
    requireType(value.type, TYPE_BOOLEAN, ctx,
                "Los operandos de && deben ser boolean", "SEM_LOGICAL_TYPE");
  }
  return ExprResult{TYPE_BOOLEAN};
}

std::any SemanticAnalyzer::visitEqualityExpr(
    CompiscriptParser::EqualityExprContext *ctx) {
  const auto nodes = ctx->relationalExpr();
  std::vector<ExprResult> values;
  for (auto *node : nodes)
    values.push_back(visitExpressionNode(node));
  if (values.size() == 1)
    return values.front();
  for (size_t index = 1; index < values.size(); ++index) {
    const Type left = values[index - 1].type;
    const Type right = values[index].type;
    if (!compatible(left, right) && !compatible(right, left)) {
      error("SEM_COMPARE_TYPE", "tipo",
            "No se pueden comparar '" + left.name + "' y '" + right.name + "'.",
            ctx);
    }
  }
  return ExprResult{TYPE_BOOLEAN};
}

std::any SemanticAnalyzer::visitRelationalExpr(
    CompiscriptParser::RelationalExprContext *ctx) {
  const auto nodes = ctx->additiveExpr();
  std::vector<ExprResult> values;
  for (auto *node : nodes)
    values.push_back(visitExpressionNode(node));
  if (values.size() == 1)
    return values.front();
  for (size_t index = 1; index < values.size(); ++index) {
    const Type left = values[index - 1].type;
    const Type right = values[index].type;
    const bool valid = (left.isNumeric() && right.isNumeric()) ||
                       (left == TYPE_STRING && right == TYPE_STRING) ||
                       left.isUnknown() || right.isUnknown();
    if (!valid) {
      error("SEM_COMPARE_TYPE", "tipo",
            "No se pueden ordenar '" + left.name + "' y '" + right.name + "'.",
            ctx);
    }
  }
  return ExprResult{TYPE_BOOLEAN};
}

std::any SemanticAnalyzer::visitAdditiveExpr(
    CompiscriptParser::AdditiveExprContext *ctx) {
  const auto nodes = ctx->multiplicativeExpr();
  const auto ops = operators(ctx, {"+", "-"});
  ExprResult result = visitExpressionNode(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    const ExprResult right = visitExpressionNode(nodes[index]);
    const std::string &op = ops[index - 1];
    if (op == "+" && result.type == TYPE_STRING && right.type == TYPE_STRING) {
      result = {TYPE_STRING};
    } else if (result.type.isNumeric() && right.type.isNumeric()) {
      result = {commonType(result.type, right.type)};
    } else if (result.type.isUnknown() || right.type.isUnknown()) {
      result = {TYPE_UNKNOWN};
    } else {
      error("SEM_ARITHMETIC_TYPE", "tipo",
            "El operador '" + op +
                "' requiere números (o dos string para +), no '" +
                result.type.name + "' y '" + right.type.name + "'.",
            ctx);
      result = {TYPE_ERROR};
    }
  }
  return result;
}

std::any SemanticAnalyzer::visitMultiplicativeExpr(
    CompiscriptParser::MultiplicativeExprContext *ctx) {
  const auto nodes = ctx->unaryExpr();
  const auto ops = operators(ctx, {"*", "/", "%"});
  ExprResult result = visitExpressionNode(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    const ExprResult right = visitExpressionNode(nodes[index]);
    const std::string &op = ops[index - 1];
    if (result.type.isNumeric() && right.type.isNumeric()) {
      result = {
          (op == "/" || result.type == TYPE_FLOAT || right.type == TYPE_FLOAT)
              ? TYPE_FLOAT
              : TYPE_INTEGER};
    } else if (result.type.isUnknown() || right.type.isUnknown()) {
      result = {TYPE_UNKNOWN};
    } else {
      error("SEM_ARITHMETIC_TYPE", "tipo",
            "El operador '" + op + "' requiere operandos numéricos, no '" +
                result.type.name + "' y '" + right.type.name + "'.",
            ctx);
      result = {TYPE_ERROR};
    }
  }
  return result;
}

std::any
SemanticAnalyzer::visitUnaryExpr(CompiscriptParser::UnaryExprContext *ctx) {
  if (ctx->primaryExpr())
    return visit(ctx->primaryExpr());
  const ExprResult operand = visitExpressionNode(ctx->unaryExpr());
  const std::string op = ctx->getStart()->getText();
  if (op == "!") {
    requireType(operand.type, TYPE_BOOLEAN, ctx,
                "El operador ! requiere boolean", "SEM_LOGICAL_TYPE");
    return ExprResult{TYPE_BOOLEAN};
  }
  if (!operand.type.isNumeric() && !operand.type.isUnknown()) {
    error("SEM_ARITHMETIC_TYPE", "tipo",
          "El operador - unario requiere un número.", ctx);
    return ExprResult{TYPE_ERROR};
  }
  return ExprResult{operand.type};
}

std::any
SemanticAnalyzer::visitPrimaryExpr(CompiscriptParser::PrimaryExprContext *ctx) {
  if (ctx->literalExpr())
    return visit(ctx->literalExpr());
  if (ctx->leftHandSide())
    return visit(ctx->leftHandSide());
  return visit(ctx->expression());
}

std::any
SemanticAnalyzer::visitLiteralExpr(CompiscriptParser::LiteralExprContext *ctx) {
  if (ctx->arrayLiteral())
    return visit(ctx->arrayLiteral());
  if (ctx->IntegerLiteral())
    return ExprResult{TYPE_INTEGER};
  if (ctx->FloatLiteral())
    return ExprResult{TYPE_FLOAT};
  if (ctx->StringLiteral())
    return ExprResult{TYPE_STRING};
  const std::string text = ctx->getText();
  if (text == "true" || text == "false")
    return ExprResult{TYPE_BOOLEAN};
  return ExprResult{TYPE_NULL};
}

// LISTAS Y ESTRUCTURAS DE DATOS
// Una lista literal debe tener elementos compatibles y sintetiza un tipo como
// integer[] o string[].
std::any SemanticAnalyzer::visitArrayLiteral(
    CompiscriptParser::ArrayLiteralContext *ctx) {
  const auto expressions = ctx->expression();
  if (expressions.empty())
    return ExprResult{listOf(TYPE_UNKNOWN)};
  Type element = visitExpressionNode(expressions.front()).type;
  for (size_t index = 1; index < expressions.size(); ++index) {
    const Type value = visitExpressionNode(expressions[index]).type;
    const Type merged = commonType(element, value);
    if (merged == TYPE_ERROR) {
      error("SEM_LIST_ELEMENT", "lista",
            "La lista mezcla elementos '" + element.name + "' y '" +
                value.name + "'.",
            ctx);
      return ExprResult{listOf(TYPE_ERROR)};
    }
    element = merged;
  }
  return ExprResult{listOf(element)};
}

std::any SemanticAnalyzer::visitIdentifierExpr(
    CompiscriptParser::IdentifierExprContext *ctx) {
  const int symbol_id = resolveSymbol(ctx->Identifier()->getText(), ctx);
  if (symbol_id < 0)
    return ExprResult{TYPE_ERROR};
  const Symbol &symbol = symbols_[symbol_id];
  if (symbol.kind == "function" || symbol.kind == "method") {
    return ExprResult{TYPE_FUNCTION, symbol_id, false};
  }
  if (symbol.kind == "class")
    return ExprResult{TYPE_CLASS, symbol_id, false};
  return ExprResult{symbol.type, symbol_id, true};
}

// FUNCIONES: LLAMADAS Y ARGUMENTOS
// Valida que el valor sea invocable y que cada argumento coincida en cantidad,
// posición y tipo con los parámetros declarados.
std::vector<ExprResult>
SemanticAnalyzer::arguments(CompiscriptParser::ArgumentsContext *ctx) {
  std::vector<ExprResult> result;
  if (!ctx)
    return result;
  for (auto *expression : ctx->expression()) {
    result.push_back(visitExpressionNode(expression));
  }
  return result;
}

void SemanticAnalyzer::validateCallArguments(
    int function_symbol, const std::vector<ExprResult> &values,
    antlr4::ParserRuleContext *ctx) {
  const Symbol &function = symbols_[function_symbol];
  if (values.size() != function.parameter_types.size()) {
    error("SEM_FUNCTION_ARITY", "función",
          "'" + function.name + "' espera " +
              std::to_string(function.parameter_types.size()) +
              " argumento(s), pero recibió " + std::to_string(values.size()) +
              ".",
          ctx);
    return;
  }
  for (size_t index = 0; index < values.size(); ++index) {
    const Type expected = function.parameter_types[index];
    if (!compatible(expected, values[index].type)) {
      error("SEM_FUNCTION_ARGUMENT", "función",
            "El argumento " + std::to_string(index + 1) + " de '" +
                function.name + "' debe ser '" + expected.name + "', no '" +
                values[index].type.name + "'.",
            ctx);
    }
  }
}

// CLASES Y OBJETOS: MIEMBROS, CONSTRUCTORES Y THIS
// Busca atributos y métodos también en clases base, y valida la creación de
// objetos y el contexto donde puede utilizarse this.
int SemanticAnalyzer::findMember(const std::string &class_name,
                                 const std::string &name, bool methods) const {
  std::set<std::string> visited;
  std::string current = class_name;
  while (!current.empty() && classes_.count(current) &&
         !visited.count(current)) {
    visited.insert(current);
    const ClassInfo &info = classes_.at(current);
    const auto &members = methods ? info.methods : info.fields;
    auto found = members.find(name);
    if (found != members.end())
      return found->second;
    current = info.base;
  }
  return -1;
}

ExprResult SemanticAnalyzer::applyProperty(const ExprResult &owner,
                                           const std::string &name,
                                           antlr4::ParserRuleContext *ctx) {
  auto info = classes_.find(owner.type.name);
  if (info == classes_.end()) {
    if (!owner.type.isUnknown()) {
      error("SEM_CLASS_PROPERTY", "clase",
            "El tipo '" + owner.type.name + "' no posee propiedades o métodos.",
            ctx);
    }
    return {TYPE_ERROR};
  }
  int symbol_id = findMember(owner.type.name, name, false);
  if (symbol_id < 0)
    symbol_id = findMember(owner.type.name, name, true);
  if (symbol_id < 0) {
    error("SEM_CLASS_MEMBER", "clase",
          "La clase '" + owner.type.name + "' no tiene un miembro llamado '" +
              name + "'.",
          ctx);
    return {TYPE_ERROR};
  }
  const Symbol &symbol = symbols_[symbol_id];
  if (symbol.kind == "method")
    return {TYPE_FUNCTION, symbol_id, false};
  return {symbol.type, symbol_id, true};
}

ExprResult
SemanticAnalyzer::applyCall(const ExprResult &callee,
                            CompiscriptParser::CallExprContext *ctx) {
  const auto values = arguments(ctx->arguments());
  if (callee.symbol_id < 0 || (symbols_[callee.symbol_id].kind != "function" &&
                               symbols_[callee.symbol_id].kind != "method")) {
    error("SEM_FUNCTION_NOT_CALLABLE", "función",
          "Una expresión de tipo '" + callee.type.name +
              "' no se puede invocar.",
          ctx);
    return {TYPE_ERROR};
  }
  validateCallArguments(callee.symbol_id, values, ctx);
  return {symbols_[callee.symbol_id].return_type};
}

std::any
SemanticAnalyzer::visitNewExpr(CompiscriptParser::NewExprContext *ctx) {
  const std::string name = ctx->Identifier()->getText();
  auto info = classes_.find(name);
  const auto values = arguments(ctx->arguments());
  if (info == classes_.end()) {
    error("SEM_CLASS_UNKNOWN", "clase", "La clase '" + name + "' no existe.",
          ctx);
    return ExprResult{TYPE_ERROR};
  }
  const int constructor = findMember(name, "constructor", true);
  if (constructor < 0 && !values.empty()) {
    error("SEM_CLASS_CONSTRUCTOR", "clase",
          "La clase '" + name +
              "' no declara constructor y no acepta argumentos.",
          ctx);
  } else if (constructor >= 0) {
    validateCallArguments(constructor, values, ctx);
  }
  return ExprResult{Type{name}};
}

std::any
SemanticAnalyzer::visitThisExpr(CompiscriptParser::ThisExprContext *ctx) {
  if (class_stack_.empty()) {
    error("SEM_CLASS_THIS", "clase",
          "this solo se permite dentro de una clase.", ctx);
    return ExprResult{TYPE_ERROR};
  }
  return ExprResult{Type{class_stack_.back()}};
}

// ACCESOS ENCADENADOS
// Recorre llamadas, accesos a miembros e índices. En listas exige índices
// integer y devuelve el tipo de sus elementos.
std::any SemanticAnalyzer::visitLeftHandSide(
    CompiscriptParser::LeftHandSideContext *ctx) {
  ExprResult result = visitExpressionNode(ctx->primaryAtom());
  for (auto *suffix : ctx->suffixOp()) {
    if (auto *call =
            dynamic_cast<CompiscriptParser::CallExprContext *>(suffix)) {
      result = applyCall(result, call);
    } else if (auto *index =
                   dynamic_cast<CompiscriptParser::IndexExprContext *>(
                       suffix)) {
      const ExprResult subscript = visitExpressionNode(index->expression());
      requireType(subscript.type, TYPE_INTEGER, index,
                  "El índice de una lista debe ser integer", "SEM_LIST_INDEX");
      if (!result.type.isList() && !result.type.isUnknown()) {
        error("SEM_LIST_ACCESS", "lista",
              "El tipo '" + result.type.name + "' no se puede indexar.", index);
        result = {TYPE_ERROR};
      } else {
        result.type = result.type.element();
      }
    } else if (auto *property =
                   dynamic_cast<CompiscriptParser::PropertyAccessExprContext *>(
                       suffix)) {
      result =
          applyProperty(result, property->Identifier()->getText(), property);
    }
  }
  return result;
}

// CLASES: HERENCIA
// Comprueba que la clase base exista y que la jerarquía no contenga ciclos.
void SemanticAnalyzer::validateInheritance(antlr4::ParserRuleContext *ctx) {
  for (auto &entry : classes_) {
    ClassInfo &info = entry.second;
    if (!info.base.empty() && classes_.count(info.base) == 0) {
      error("SEM_CLASS_BASE_UNKNOWN", "clase",
            "La clase base '" + info.base + "' de '" + info.name +
                "' no existe.",
            ctx);
    }
  }
  for (const auto &entry : classes_) {
    std::set<std::string> seen;
    std::string current = entry.first;
    while (classes_.count(current) && !classes_.at(current).base.empty()) {
      if (seen.count(current)) {
        error("SEM_CLASS_CYCLE", "clase",
              "La herencia de '" + entry.first + "' contiene un ciclo.", ctx);
        break;
      }
      seen.insert(current);
      current = classes_.at(current).base;
    }
  }
}

} // namespace compiscript
