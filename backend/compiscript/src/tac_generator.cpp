// Generación de código intermedio en cuádruplos y representación TAC legible.
#include "compiscript/tac_generator.h"

#include <algorithm>
#include <cctype>
#include <functional>
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

std::string safeName(const std::string &value) {
  std::string result;
  for (unsigned char character : value) {
    result += std::isalnum(character) || character == '_' ? character : '_';
  }
  return result.empty() ? "anonymous" : result;
}

bool isTemporary(const std::string &value) {
  return value.size() > 2 && value[0] == '%' && value[1] == 't';
}

} // namespace

TacGenerator::TacGenerator(SemanticResult &semantic,
                           const SemanticBindings &bindings)
    : semantic_(semantic), bindings_(bindings) {
  pushTempPool();
  layoutSymbols();
}

TacResult TacGenerator::generate(CompiscriptParser::ProgramContext *program) {
  TacResult result;
  if (!semantic_.ok) {
    result.skipped = true;
    result.reason =
        "La generación TAC se omitió porque existen errores semánticos.";
    return result;
  }

  visit(program);
  result.ok = diagnostics_.empty();
  result.diagnostics = diagnostics_;
  result.instructions = instructions_;
  result.activation_records = activation_records_;
  result.temporaries_created = total_temporaries_created_;
  result.temporaries_reused = total_temporaries_reused_;
  result.max_live_temporaries = max_live_temporaries_;

  std::ostringstream text;
  for (const auto &instruction : instructions_)
    text << instruction.text << '\n';
  result.text = text.str();
  return result;
}

TacValue TacGenerator::valueOf(const std::any &value) const {
  if (value.has_value() && value.type() == typeid(TacValue))
    return std::any_cast<TacValue>(value);
  return {};
}

TacValue TacGenerator::generateExpression(antlr4::tree::ParseTree *node) {
  return node ? valueOf(visit(node)) : TacValue{};
}

Type TacGenerator::expressionType(const void *node) const {
  const auto found = bindings_.expressions.find(node);
  return found == bindings_.expressions.end() ? TYPE_UNKNOWN
                                              : found->second.type;
}

int TacGenerator::boundSymbol(const void *node) const {
  const auto found = bindings_.symbols.find(node);
  return found == bindings_.symbols.end() ? -1 : found->second;
}

const Symbol *TacGenerator::symbol(int id) const {
  return id >= 0 && static_cast<size_t>(id) < semantic_.symbols.size()
             ? &semantic_.symbols[id]
             : nullptr;
}

Symbol *TacGenerator::symbol(int id) {
  return id >= 0 && static_cast<size_t>(id) < semantic_.symbols.size()
             ? &semantic_.symbols[id]
             : nullptr;
}

std::vector<std::string>
TacGenerator::operators(antlr4::ParserRuleContext *ctx,
                        const std::set<std::string> &accepted) const {
  std::vector<std::string> result;
  for (auto *child : ctx->children) {
    const std::string text = child->getText();
    if (accepted.count(text))
      result.push_back(text);
  }
  return result;
}

size_t TacGenerator::typeSize(const Type &type) const {
  if (type == TYPE_BOOLEAN)
    return 1;
  if (type == TYPE_INTEGER)
    return 4;
  if (type == TYPE_FLOAT)
    return 8;
  if (type == TYPE_VOID)
    return 0;
  // Strings, listas, objetos, funciones y valores aún desconocidos se modelan
  // como referencias de una palabra.
  return 8;
}

size_t TacGenerator::aligned(size_t value, size_t alignment) const {
  return value == 0 ? 0 : ((value + alignment - 1) / alignment) * alignment;
}

int TacGenerator::enclosingFunction(int scope_id) const {
  while (scope_id >= 0 &&
         static_cast<size_t>(scope_id) < semantic_.scopes.size()) {
    const Scope &scope = semantic_.scopes[scope_id];
    if (scope.kind == "function")
      return scope.owner_symbol_id;
    scope_id = scope.parent_id;
  }
  return -1;
}

std::string TacGenerator::functionLabel(int symbol_id) const {
  const Symbol *item = symbol(symbol_id);
  return item ? item->label : "fn_invalida";
}

std::string TacGenerator::storageName(int symbol_id) const {
  const Symbol *item = symbol(symbol_id);
  if (!item)
    return "%simbolo_invalido";
  if (item->kind == "function" || item->kind == "method" ||
      item->kind == "class")
    return item->label;
  if (item->kind == "field")
    return "%this." + item->name;

  const std::string prefix = item->storage == "global" ? "@" : "%";
  const std::string base =
      prefix + safeName(item->name) + "#" + std::to_string(item->id);
  if (!function_stack_.empty()) {
    const Symbol *function = symbol(function_stack_.back());
    if (function && function->captures.count(symbol_id))
      return "env[" + base + "]";
  }
  return base;
}

int TacGenerator::findClassMember(const std::string &class_name,
                                  const std::string &name, bool methods) const {
  std::set<std::string> visited;
  std::string current = class_name;
  while (!current.empty() && !visited.count(current)) {
    visited.insert(current);
    auto info = std::find_if(
        semantic_.classes.begin(), semantic_.classes.end(),
        [&](const ClassInfo &item) { return item.name == current; });
    if (info == semantic_.classes.end())
      break;
    const auto &members = methods ? info->methods : info->fields;
    auto found = members.find(name);
    if (found != members.end())
      return found->second;
    current = info->base;
  }
  return -1;
}

void TacGenerator::layoutSymbols() {
  for (Scope &scope : semantic_.scopes) {
    int depth = 0;
    int parent = scope.parent_id;
    while (parent >= 0 &&
           static_cast<size_t>(parent) < semantic_.scopes.size()) {
      ++depth;
      parent = semantic_.scopes[parent].parent_id;
    }
    scope.lexical_depth = depth;
  }

  for (Symbol &item : semantic_.symbols) {
    item.size = typeSize(item.type);
    item.lexical_depth =
        item.scope_id >= 0 &&
                static_cast<size_t>(item.scope_id) < semantic_.scopes.size()
            ? semantic_.scopes[item.scope_id].lexical_depth
            : 0;
    if (item.kind == "function" || item.kind == "method") {
      item.storage = "code";
      const std::string owner =
          item.owner_class.empty() ? "" : safeName(item.owner_class) + "_";
      item.label =
          "fn_" + owner + safeName(item.name) + "_" + std::to_string(item.id);
      ActivationRecord frame;
      frame.function_symbol_id = item.id;
      frame.scope_id = item.function_scope_id;
      frame.name = item.owner_class.empty()
                       ? item.name
                       : item.owner_class + "." + item.name;
      frame.label = item.label;
      frame.lexical_depth =
          item.function_scope_id >= 0 &&
                  static_cast<size_t>(item.function_scope_id) <
                      semantic_.scopes.size()
              ? semantic_.scopes[item.function_scope_id].lexical_depth
              : 0;
      frame.captured_symbols.assign(item.captures.begin(), item.captures.end());
      activation_by_function_[item.id] = activation_records_.size();
      activation_records_.push_back(frame);
    } else if (item.kind == "class") {
      item.storage = "type";
      item.label =
          "class_" + safeName(item.name) + "_" + std::to_string(item.id);
    }
  }

  // Campos: el desplazamiento de una clase derivada comienza después de la
  // porción heredada del objeto.
  std::unordered_map<std::string, size_t> class_sizes;
  std::function<size_t(const std::string &)> layoutClass =
      [&](const std::string &name) -> size_t {
    auto known = class_sizes.find(name);
    if (known != class_sizes.end())
      return known->second;
    auto info =
        std::find_if(semantic_.classes.begin(), semantic_.classes.end(),
                     [&](const ClassInfo &item) { return item.name == name; });
    if (info == semantic_.classes.end())
      return 0;
    // Marca temporal que también protege frente a ciclos ya descartados por
    // el análisis semántico.
    class_sizes[name] = 0;
    size_t offset = info->base.empty() ? 0 : layoutClass(info->base);
    std::vector<int> fields;
    for (const auto &entry : info->fields)
      fields.push_back(entry.second);
    std::sort(fields.begin(), fields.end());
    for (int id : fields) {
      Symbol *field = symbol(id);
      if (!field)
        continue;
      field->storage = "field";
      field->offset = static_cast<int>(offset);
      offset += aligned(field->size);
    }
    class_sizes[name] = offset;
    return offset;
  };
  for (const ClassInfo &info : semantic_.classes)
    layoutClass(info.name);

  size_t global_offset = 0;
  std::unordered_map<int, size_t> parameter_offsets;
  std::unordered_map<int, size_t> local_offsets;
  for (Symbol &item : semantic_.symbols) {
    if (item.storage == "code" || item.storage == "type" ||
        item.storage == "field")
      continue;
    const int owner = enclosingFunction(item.scope_id);
    if (owner < 0) {
      item.storage = "global";
      item.offset = static_cast<int>(global_offset);
      global_offset += aligned(item.size);
      continue;
    }
    auto frame_position = activation_by_function_.find(owner);
    if (frame_position == activation_by_function_.end())
      continue;
    ActivationRecord &frame = activation_records_[frame_position->second];
    if (item.kind == "parameter") {
      item.storage = "parameter";
      item.offset = static_cast<int>(32 + parameter_offsets[owner]);
      parameter_offsets[owner] += aligned(item.size);
      frame.parameter_symbols.push_back(item.id);
      frame.parameter_bytes = parameter_offsets[owner];
    } else {
      item.storage = item.captured_by.empty() ? "local" : "captured-local";
      local_offsets[owner] += aligned(item.size);
      item.offset = -static_cast<int>(local_offsets[owner]);
      frame.local_symbols.push_back(item.id);
      frame.local_bytes = local_offsets[owner];
    }
  }

  for (ActivationRecord &frame : activation_records_) {
    frame.frame_size = 32 + frame.parameter_bytes + frame.local_bytes;
    if (frame.scope_id >= 0 &&
        static_cast<size_t>(frame.scope_id) < semantic_.scopes.size())
      semantic_.scopes[frame.scope_id].frame_size = frame.frame_size;
  }
}

void TacGenerator::pushTempPool() { temp_pools_.push_back(TempPool{}); }

void TacGenerator::popTempPool(int function_symbol_id) {
  if (temp_pools_.empty())
    return;
  const TempPool pool = temp_pools_.back();
  temp_pools_.pop_back();
  auto found = activation_by_function_.find(function_symbol_id);
  if (found != activation_by_function_.end()) {
    ActivationRecord &frame = activation_records_[found->second];
    frame.temporary_bytes = pool.max_live * 8;
    frame.frame_size =
        32 + frame.parameter_bytes + frame.local_bytes + frame.temporary_bytes;
    if (frame.scope_id >= 0 &&
        static_cast<size_t>(frame.scope_id) < semantic_.scopes.size())
      semantic_.scopes[frame.scope_id].frame_size = frame.frame_size;
  }
}

std::string TacGenerator::newTemp() {
  TempPool &pool = temp_pools_.back();
  std::string name;
  if (!pool.free.empty()) {
    name = pool.free.back();
    pool.free.pop_back();
    ++pool.reused;
    ++total_temporaries_reused_;
  } else {
    name = "%t" + std::to_string(pool.next_id++);
    ++pool.created;
    ++total_temporaries_created_;
  }
  pool.live.insert(name);
  pool.max_live = std::max(pool.max_live, pool.live.size());
  max_live_temporaries_ = std::max(max_live_temporaries_, pool.live.size());
  return name;
}

void TacGenerator::releaseTemp(const std::string &name) {
  if (!isTemporary(name) || temp_pools_.empty())
    return;
  TempPool &pool = temp_pools_.back();
  if (pool.live.erase(name))
    pool.free.push_back(name);
}

void TacGenerator::releaseValue(const TacValue &value) {
  releaseTemp(value.place);
  releaseTemp(value.base);
  releaseTemp(value.index);
  releaseTemp(value.receiver);
}

std::string TacGenerator::newLabel(const std::string &prefix) {
  return prefix + std::to_string(label_counter_++);
}

void TacGenerator::report(const std::string &code, const std::string &message,
                          antlr4::ParserRuleContext *ctx) {
  diagnostics_.push_back({code, "código intermedio", "error", message,
                          lineOf(ctx), columnOf(ctx)});
}

void TacGenerator::emit(const std::string &op, const std::string &result,
                        const std::string &arg1, const std::string &arg2,
                        antlr4::ParserRuleContext *ctx) {
  TacInstruction instruction;
  instruction.index = instructions_.size();
  instruction.op = op;
  instruction.arg1 = arg1;
  instruction.arg2 = arg2;
  instruction.result = result;
  instruction.line = lineOf(ctx);
  instruction.column = columnOf(ctx);
  instruction.text = format(instruction);
  instructions_.push_back(std::move(instruction));
}

std::string TacGenerator::format(const TacInstruction &item) const {
  static const std::unordered_map<std::string, std::string> symbols = {
      {"add", "+"}, {"sub", "-"}, {"mul", "*"}, {"div", "/"},
      {"mod", "%"}, {"eq", "=="}, {"ne", "!="}, {"lt", "<"},
      {"le", "<="}, {"gt", ">"},  {"ge", ">="}};
  auto binary = symbols.find(item.op);
  if (binary != symbols.end())
    return item.result + " = " + item.arg1 + " " + binary->second + " " +
           item.arg2;
  if (item.op == "assign")
    return item.result + " = " + item.arg1;
  if (item.op == "neg" || item.op == "not")
    return item.result + " = " + (item.op == "neg" ? "-" : "!") + item.arg1;
  if (item.op == "label")
    return item.result + ":";
  if (item.op == "goto")
    return "goto " + item.result;
  if (item.op == "if_true" || item.op == "if_false")
    return (item.op == "if_true" ? "if " : "ifFalse ") + item.arg1 + " goto " +
           item.result;
  if (item.op == "param")
    return "param " + item.arg1;
  if (item.op == "param_in")
    return "param_in " + item.result + ", " + item.arg1;
  if (item.op == "call" || item.op == "call_method") {
    const std::string call =
        (item.op == "call_method" ? "call_method " : "call ") + item.arg1 +
        ", " + item.arg2;
    return item.result.empty() ? call : item.result + " = " + call;
  }
  if (item.op == "return")
    return item.arg1.empty() ? "return" : "return " + item.arg1;
  if (item.op == "print")
    return "print " + item.arg1;
  if (item.op == "declare")
    return "declare " + item.result + " : " + item.arg1;
  if (item.op == "begin_program" || item.op == "end_program")
    return item.op;
  if (item.op == "begin_func")
    return "func " + item.result + ":";
  if (item.op == "end_func")
    return "endfunc " + item.result;
  if (item.op == "begin_class")
    return "class " + item.result +
           (item.arg1.empty() ? "" : " extends " + item.arg1) + ":";
  if (item.op == "end_class")
    return "endclass " + item.result;
  if (item.op == "capture")
    return "capture " + item.result + " from " + item.arg1;
  if (item.op == "static_link")
    return "static_link " + item.result + " = " + item.arg1;
  if (item.op == "new_list")
    return item.result + " = newlist " + item.arg1;
  if (item.op == "load_index")
    return item.result + " = " + item.arg1 + "[" + item.arg2 + "]";
  if (item.op == "store_index")
    return item.result + "[" + item.arg1 + "] = " + item.arg2;
  if (item.op == "length")
    return item.result + " = length " + item.arg1;
  if (item.op == "new_object")
    return item.result + " = new " + item.arg1;
  if (item.op == "load_field")
    return item.result + " = " + item.arg1 + "." + item.arg2;
  if (item.op == "store_field")
    return item.result + "." + item.arg1 + " = " + item.arg2;
  if (item.op == "field_decl")
    return "field " + item.result + " : " + item.arg1;
  if (item.op == "field_init")
    return "field " + item.result + " = " + item.arg1;
  if (item.op == "try_begin")
    return "try_begin " + item.result;
  if (item.op == "catch")
    return "catch " + item.result;
  if (item.op == "try_end")
    return "try_end";
  return item.op + " " + item.result + " " + item.arg1 + " " + item.arg2;
}

TacValue TacGenerator::emitBinary(const std::string &op, const TacValue &left,
                                  const TacValue &right,
                                  antlr4::ParserRuleContext *ctx) {
  const std::string temporary = newTemp();
  emit(op, temporary, left.place, right.place, ctx);
  Type type = expressionType(ctx);
  if (type.isUnknown()) {
    if (op == "eq" || op == "ne" || op == "lt" || op == "le" || op == "gt" ||
        op == "ge")
      type = TYPE_BOOLEAN;
    else if (op == "div")
      type = TYPE_FLOAT;
    else
      type = commonType(left.type, right.type);
  }
  releaseValue(left);
  releaseValue(right);
  return {type, temporary};
}

void TacGenerator::store(const TacValue &target, const TacValue &value,
                         antlr4::ParserRuleContext *ctx) {
  switch (target.location) {
  case TacLocation::Variable:
    emit("assign", target.place, value.place, {}, ctx);
    break;
  case TacLocation::Index:
    emit("store_index", target.base, target.index, value.place, ctx);
    break;
  case TacLocation::Field:
    emit("store_field", target.base, target.member, value.place, ctx);
    break;
  default:
    report("TAC_INVALID_TARGET",
           "No se pudo determinar el destino de una asignación válida.", ctx);
  }
}

std::vector<TacValue>
TacGenerator::generateArguments(CompiscriptParser::ArgumentsContext *ctx) {
  std::vector<TacValue> values;
  if (!ctx)
    return values;
  for (auto *expression : ctx->expression())
    values.push_back(generateExpression(expression));
  return values;
}

TacValue TacGenerator::emitCall(const TacValue &callee,
                                CompiscriptParser::CallExprContext *ctx) {
  const Symbol *function = symbol(callee.symbol_id);
  if (!function) {
    report("TAC_UNRESOLVED_CALL",
           "La llamada no conserva el símbolo resuelto por la semántica.", ctx);
    return {};
  }
  std::vector<TacValue> arguments = generateArguments(ctx->arguments());
  if (!callee.receiver.empty())
    emit("param", {}, callee.receiver, {}, ctx);
  for (const TacValue &argument : arguments)
    emit("param", {}, argument.place, {}, ctx);
  if (!function->captures.empty())
    emit("static_link", function->label, "current_env", {}, ctx);

  std::string result;
  if (function->return_type != TYPE_VOID)
    result = newTemp();
  emit(function->kind == "method" ? "call_method" : "call", result,
       function->label, std::to_string(arguments.size()), ctx);
  for (const TacValue &argument : arguments)
    releaseValue(argument);
  releaseTemp(callee.receiver);
  return {function->return_type, result};
}

std::any TacGenerator::visitProgram(CompiscriptParser::ProgramContext *ctx) {
  emit("begin_program", {}, {}, {}, ctx);
  for (auto *statement : ctx->statement())
    visit(statement);
  emit("end_program", {}, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitBlock(CompiscriptParser::BlockContext *ctx) {
  for (auto *statement : ctx->statement())
    visit(statement);
  return TacValue{};
}

std::any
TacGenerator::visitControlBody(CompiscriptParser::ControlBodyContext *ctx) {
  return ctx->block() ? visit(ctx->block()) : visit(ctx->statement());
}

std::any TacGenerator::visitVariableDeclaration(
    CompiscriptParser::VariableDeclarationContext *ctx) {
  const int id = boundSymbol(ctx);
  const Symbol *item = symbol(id);
  if (!item) {
    report("TAC_UNRESOLVED_DECLARATION",
           "La variable no conserva su entrada de tabla de símbolos.", ctx);
    return TacValue{};
  }
  if (item->kind == "field") {
    if (ctx->initializer()) {
      TacValue value = generateExpression(ctx->initializer()->expression());
      emit("field_init", item->name, value.place, item->type.name, ctx);
      releaseValue(value);
    } else {
      emit("field_decl", item->name, item->type.name, {}, ctx);
    }
    return TacValue{};
  }
  emit("declare", storageName(id), item->type.name, {}, ctx);
  if (ctx->initializer()) {
    TacValue value = generateExpression(ctx->initializer()->expression());
    emit("assign", storageName(id), value.place, {}, ctx);
    releaseValue(value);
  }
  return TacValue{};
}

std::any TacGenerator::visitConstantDeclaration(
    CompiscriptParser::ConstantDeclarationContext *ctx) {
  const int id = boundSymbol(ctx);
  const Symbol *item = symbol(id);
  if (!item) {
    report("TAC_UNRESOLVED_DECLARATION",
           "La constante no conserva su entrada de tabla de símbolos.", ctx);
    return TacValue{};
  }
  TacValue value = generateExpression(ctx->expression());
  if (item->kind == "field")
    emit("field_init", item->name, value.place, item->type.name, ctx);
  else {
    emit("declare", storageName(id), item->type.name, {}, ctx);
    emit("assign", storageName(id), value.place, {}, ctx);
  }
  releaseValue(value);
  return TacValue{};
}

std::any TacGenerator::visitAssignmentStatement(
    CompiscriptParser::AssignmentStatementContext *ctx) {
  TacValue target = generateExpression(ctx->leftHandSide());
  TacValue value = generateExpression(ctx->expression());
  store(target, value, ctx);
  releaseValue(target);
  releaseValue(value);
  return TacValue{};
}

std::any TacGenerator::visitExpressionStatement(
    CompiscriptParser::ExpressionStatementContext *ctx) {
  TacValue value = generateExpression(ctx->expression());
  releaseValue(value);
  return TacValue{};
}

std::any TacGenerator::visitPrintStatement(
    CompiscriptParser::PrintStatementContext *ctx) {
  TacValue value = generateExpression(ctx->expression());
  emit("print", {}, value.place, {}, ctx);
  releaseValue(value);
  return TacValue{};
}

std::any
TacGenerator::visitIfStatement(CompiscriptParser::IfStatementContext *ctx) {
  const auto bodies = ctx->controlBody();
  const std::string else_label = newLabel("L_if_else_");
  const std::string end_label = newLabel("L_if_end_");
  TacValue condition = generateExpression(ctx->expression());
  emit("if_false", bodies.size() == 2 ? else_label : end_label, condition.place,
       {}, ctx);
  releaseValue(condition);
  visit(bodies.front());
  if (bodies.size() == 2) {
    emit("goto", end_label, {}, {}, ctx);
    emit("label", else_label, {}, {}, ctx);
    visit(bodies[1]);
  }
  emit("label", end_label, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitWhileStatement(
    CompiscriptParser::WhileStatementContext *ctx) {
  const std::string condition_label = newLabel("L_while_cond_");
  const std::string end_label = newLabel("L_while_end_");
  emit("label", condition_label, {}, {}, ctx);
  TacValue condition = generateExpression(ctx->expression());
  emit("if_false", end_label, condition.place, {}, ctx);
  releaseValue(condition);
  break_targets_.push_back(end_label);
  continue_targets_.push_back(condition_label);
  visit(ctx->controlBody());
  continue_targets_.pop_back();
  break_targets_.pop_back();
  emit("goto", condition_label, {}, {}, ctx);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitDoWhileStatement(
    CompiscriptParser::DoWhileStatementContext *ctx) {
  const std::string body_label = newLabel("L_do_body_");
  const std::string condition_label = newLabel("L_do_cond_");
  const std::string end_label = newLabel("L_do_end_");
  emit("label", body_label, {}, {}, ctx);
  break_targets_.push_back(end_label);
  continue_targets_.push_back(condition_label);
  visit(ctx->controlBody());
  continue_targets_.pop_back();
  break_targets_.pop_back();
  emit("label", condition_label, {}, {}, ctx);
  TacValue condition = generateExpression(ctx->expression());
  emit("if_true", body_label, condition.place, {}, ctx);
  releaseValue(condition);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{};
}

std::any
TacGenerator::visitForStatement(CompiscriptParser::ForStatementContext *ctx) {
  if (ctx->variableDeclaration())
    visit(ctx->variableDeclaration());
  if (ctx->expressionStatement())
    visit(ctx->expressionStatement());
  const auto expressions = ctx->expression();
  const std::string condition_label = newLabel("L_for_cond_");
  const std::string increment_label = newLabel("L_for_inc_");
  const std::string end_label = newLabel("L_for_end_");
  emit("label", condition_label, {}, {}, ctx);
  if (!expressions.empty()) {
    TacValue condition = generateExpression(expressions[0]);
    emit("if_false", end_label, condition.place, {}, ctx);
    releaseValue(condition);
  }
  break_targets_.push_back(end_label);
  continue_targets_.push_back(increment_label);
  visit(ctx->controlBody());
  continue_targets_.pop_back();
  break_targets_.pop_back();
  emit("label", increment_label, {}, {}, ctx);
  if (expressions.size() > 1) {
    TacValue increment = generateExpression(expressions[1]);
    releaseValue(increment);
  }
  emit("goto", condition_label, {}, {}, ctx);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitForeachStatement(
    CompiscriptParser::ForeachStatementContext *ctx) {
  TacValue collection = generateExpression(ctx->expression());
  const int iteration_id = boundSymbol(ctx);
  const Symbol *iteration_symbol = symbol(iteration_id);
  const std::string index = newTemp();
  const std::string length = newTemp();
  const std::string condition_label = newLabel("L_foreach_cond_");
  const std::string increment_label = newLabel("L_foreach_inc_");
  const std::string end_label = newLabel("L_foreach_end_");
  emit("declare", storageName(iteration_id),
       iteration_symbol ? iteration_symbol->type.name : "unknown", {}, ctx);
  emit("assign", index, "0", {}, ctx);
  emit("length", length, collection.place, {}, ctx);
  emit("label", condition_label, {}, {}, ctx);
  const std::string comparison_place = newTemp();
  emit("lt", comparison_place, index, length, ctx);
  TacValue comparison{TYPE_BOOLEAN, comparison_place};
  emit("if_false", end_label, comparison.place, {}, ctx);
  releaseValue(comparison);
  const std::string element = newTemp();
  emit("load_index", element, collection.place, index, ctx);
  emit("assign", storageName(iteration_id), element, {}, ctx);
  releaseTemp(element);
  break_targets_.push_back(end_label);
  continue_targets_.push_back(increment_label);
  visit(ctx->controlBody());
  continue_targets_.pop_back();
  break_targets_.pop_back();
  emit("label", increment_label, {}, {}, ctx);
  const std::string next_place = newTemp();
  emit("add", next_place, index, "1", ctx);
  TacValue next{TYPE_INTEGER, next_place};
  emit("assign", index, next.place, {}, ctx);
  releaseValue(next);
  emit("goto", condition_label, {}, {}, ctx);
  emit("label", end_label, {}, {}, ctx);
  releaseTemp(index);
  releaseTemp(length);
  releaseValue(collection);
  return TacValue{};
}

std::any TacGenerator::visitBreakStatement(
    CompiscriptParser::BreakStatementContext *ctx) {
  if (break_targets_.empty())
    report("TAC_BREAK_TARGET", "No existe etiqueta de destino para break.",
           ctx);
  else
    emit("goto", break_targets_.back(), {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitContinueStatement(
    CompiscriptParser::ContinueStatementContext *ctx) {
  if (continue_targets_.empty())
    report("TAC_CONTINUE_TARGET",
           "No existe etiqueta de destino para continue.", ctx);
  else
    emit("goto", continue_targets_.back(), {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitReturnStatement(
    CompiscriptParser::ReturnStatementContext *ctx) {
  TacValue value = ctx->expression() ? generateExpression(ctx->expression())
                                     : TacValue{TYPE_VOID};
  emit("return", {}, value.place, {}, ctx);
  releaseValue(value);
  return TacValue{};
}

std::any TacGenerator::visitTryCatchStatement(
    CompiscriptParser::TryCatchStatementContext *ctx) {
  const std::string catch_label = newLabel("L_catch_");
  const std::string end_label = newLabel("L_try_end_");
  emit("try_begin", catch_label, {}, {}, ctx);
  visit(ctx->block(0));
  emit("goto", end_label, {}, {}, ctx);
  emit("label", catch_label, {}, {}, ctx);
  const int catch_id = boundSymbol(ctx);
  emit("catch", storageName(catch_id), {}, {}, ctx);
  visit(ctx->block(1));
  emit("label", end_label, {}, {}, ctx);
  emit("try_end", {}, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitSwitchStatement(
    CompiscriptParser::SwitchStatementContext *ctx) {
  TacValue selector = generateExpression(ctx->expression());
  const std::string end_label = newLabel("L_switch_end_");
  for (auto *switch_case : ctx->switchCase()) {
    const std::string next_label = newLabel("L_case_next_");
    TacValue value = generateExpression(switch_case->expression());
    const std::string comparison_place = newTemp();
    emit("eq", comparison_place, selector.place, value.place, switch_case);
    TacValue comparison{TYPE_BOOLEAN, comparison_place};
    releaseValue(value);
    emit("if_false", next_label, comparison.place, {}, switch_case);
    releaseValue(comparison);
    for (auto *statement : switch_case->statement())
      visit(statement);
    // Compiscript define cases sin caída implícita porque break está reservado
    // para bucles por la especificación semántica del proyecto.
    emit("goto", end_label, {}, {}, switch_case);
    emit("label", next_label, {}, {}, switch_case);
  }
  if (ctx->defaultCase()) {
    for (auto *statement : ctx->defaultCase()->statement())
      visit(statement);
  }
  emit("label", end_label, {}, {}, ctx);
  releaseValue(selector);
  return TacValue{};
}

std::any TacGenerator::visitFunctionDeclaration(
    CompiscriptParser::FunctionDeclarationContext *ctx) {
  const int id = boundSymbol(ctx);
  const Symbol *function = symbol(id);
  if (!function) {
    report("TAC_UNRESOLVED_FUNCTION",
           "La función no conserva su entrada de tabla de símbolos.", ctx);
    return TacValue{};
  }
  emit("begin_func", function->label, {}, {}, ctx);
  function_stack_.push_back(id);
  pushTempPool();
  if (function->kind == "method")
    emit("param_in", "%this", "receiver", {}, ctx);
  const auto parameters =
      ctx->parameters() ? ctx->parameters()->parameter()
                        : std::vector<CompiscriptParser::ParameterContext *>{};
  for (size_t index = 0; index < parameters.size(); ++index) {
    const int parameter_id = boundSymbol(parameters[index]);
    emit("param_in", storageName(parameter_id), std::to_string(index), {},
         parameters[index]);
  }
  for (int capture : function->captures)
    emit("capture", storageName(capture), "access_link", {}, ctx);
  visit(ctx->block());
  if (function->return_type == TYPE_VOID)
    emit("return", {}, {}, {}, ctx);
  popTempPool(id);
  function_stack_.pop_back();
  emit("end_func", function->label, {}, {}, ctx);
  return TacValue{};
}

std::any TacGenerator::visitClassDeclaration(
    CompiscriptParser::ClassDeclarationContext *ctx) {
  const int id = boundSymbol(ctx);
  const Symbol *item = symbol(id);
  if (!item) {
    report("TAC_UNRESOLVED_CLASS",
           "La clase no conserva su entrada de tabla de símbolos.", ctx);
    return TacValue{};
  }
  const std::string name = ctx->Identifier(0)->getText();
  const std::string base =
      ctx->Identifier().size() > 1 ? ctx->Identifier(1)->getText() : "";
  emit("begin_class", name, base, {}, ctx);
  class_stack_.push_back(name);
  for (auto *member : ctx->classMember())
    visit(member);
  class_stack_.pop_back();
  emit("end_class", name, {}, {}, ctx);
  return TacValue{};
}

std::any
TacGenerator::visitClassMember(CompiscriptParser::ClassMemberContext *ctx) {
  if (ctx->functionDeclaration())
    return visit(ctx->functionDeclaration());
  if (ctx->variableDeclaration())
    return visit(ctx->variableDeclaration());
  return visit(ctx->constantDeclaration());
}

std::any
TacGenerator::visitExpression(CompiscriptParser::ExpressionContext *ctx) {
  return visit(ctx->assignmentExpr());
}

std::any
TacGenerator::visitAssignExpr(CompiscriptParser::AssignExprContext *ctx) {
  TacValue target = generateExpression(ctx->leftHandSide());
  TacValue value = generateExpression(ctx->assignmentExpr());
  store(target, value, ctx);
  releaseValue(target);
  return value;
}

std::any
TacGenerator::visitExprNoAssign(CompiscriptParser::ExprNoAssignContext *ctx) {
  return visit(ctx->conditionalExpr());
}

std::any TacGenerator::visitConditionalExpr(
    CompiscriptParser::ConditionalExprContext *ctx) {
  TacValue condition = generateExpression(ctx->logicalOrExpr());
  const auto branches = ctx->expression();
  if (branches.empty())
    return condition;
  const std::string else_label = newLabel("L_ternary_else_");
  const std::string end_label = newLabel("L_ternary_end_");
  const std::string result = newTemp();
  emit("if_false", else_label, condition.place, {}, ctx);
  releaseValue(condition);
  TacValue left = generateExpression(branches[0]);
  emit("assign", result, left.place, {}, ctx);
  releaseValue(left);
  emit("goto", end_label, {}, {}, ctx);
  emit("label", else_label, {}, {}, ctx);
  TacValue right = generateExpression(branches[1]);
  emit("assign", result, right.place, {}, ctx);
  releaseValue(right);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{expressionType(ctx), result};
}

std::any
TacGenerator::visitLogicalOrExpr(CompiscriptParser::LogicalOrExprContext *ctx) {
  const auto operands = ctx->logicalAndExpr();
  if (operands.size() == 1)
    return visit(operands.front());
  const std::string result = newTemp();
  const std::string true_label = newLabel("L_or_true_");
  const std::string end_label = newLabel("L_or_end_");
  for (auto *operand : operands) {
    TacValue value = generateExpression(operand);
    emit("if_true", true_label, value.place, {}, ctx);
    releaseValue(value);
  }
  emit("assign", result, "false", {}, ctx);
  emit("goto", end_label, {}, {}, ctx);
  emit("label", true_label, {}, {}, ctx);
  emit("assign", result, "true", {}, ctx);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{TYPE_BOOLEAN, result};
}

std::any TacGenerator::visitLogicalAndExpr(
    CompiscriptParser::LogicalAndExprContext *ctx) {
  const auto operands = ctx->equalityExpr();
  if (operands.size() == 1)
    return visit(operands.front());
  const std::string result = newTemp();
  const std::string false_label = newLabel("L_and_false_");
  const std::string end_label = newLabel("L_and_end_");
  for (auto *operand : operands) {
    TacValue value = generateExpression(operand);
    emit("if_false", false_label, value.place, {}, ctx);
    releaseValue(value);
  }
  emit("assign", result, "true", {}, ctx);
  emit("goto", end_label, {}, {}, ctx);
  emit("label", false_label, {}, {}, ctx);
  emit("assign", result, "false", {}, ctx);
  emit("label", end_label, {}, {}, ctx);
  return TacValue{TYPE_BOOLEAN, result};
}

std::any
TacGenerator::visitEqualityExpr(CompiscriptParser::EqualityExprContext *ctx) {
  const auto nodes = ctx->relationalExpr();
  if (nodes.size() == 1)
    return visit(nodes.front());
  const auto ops = operators(ctx, {"==", "!="});
  TacValue result = generateExpression(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    TacValue right = generateExpression(nodes[index]);
    result =
        emitBinary(ops[index - 1] == "==" ? "eq" : "ne", result, right, ctx);
  }
  return result;
}

std::any TacGenerator::visitRelationalExpr(
    CompiscriptParser::RelationalExprContext *ctx) {
  const auto nodes = ctx->additiveExpr();
  if (nodes.size() == 1)
    return visit(nodes.front());
  const auto ops = operators(ctx, {"<", "<=", ">", ">="});
  static const std::unordered_map<std::string, std::string> names = {
      {"<", "lt"}, {"<=", "le"}, {">", "gt"}, {">=", "ge"}};
  TacValue result = generateExpression(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    TacValue right = generateExpression(nodes[index]);
    result = emitBinary(names.at(ops[index - 1]), result, right, ctx);
  }
  return result;
}

std::any
TacGenerator::visitAdditiveExpr(CompiscriptParser::AdditiveExprContext *ctx) {
  const auto nodes = ctx->multiplicativeExpr();
  if (nodes.size() == 1)
    return visit(nodes.front());
  const auto ops = operators(ctx, {"+", "-"});
  TacValue result = generateExpression(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    TacValue right = generateExpression(nodes[index]);
    result =
        emitBinary(ops[index - 1] == "+" ? "add" : "sub", result, right, ctx);
  }
  return result;
}

std::any TacGenerator::visitMultiplicativeExpr(
    CompiscriptParser::MultiplicativeExprContext *ctx) {
  const auto nodes = ctx->unaryExpr();
  if (nodes.size() == 1)
    return visit(nodes.front());
  const auto ops = operators(ctx, {"*", "/", "%"});
  static const std::unordered_map<std::string, std::string> names = {
      {"*", "mul"}, {"/", "div"}, {"%", "mod"}};
  TacValue result = generateExpression(nodes.front());
  for (size_t index = 1; index < nodes.size(); ++index) {
    TacValue right = generateExpression(nodes[index]);
    result = emitBinary(names.at(ops[index - 1]), result, right, ctx);
  }
  return result;
}

std::any
TacGenerator::visitUnaryExpr(CompiscriptParser::UnaryExprContext *ctx) {
  if (ctx->primaryExpr())
    return visit(ctx->primaryExpr());
  TacValue operand = generateExpression(ctx->unaryExpr());
  const std::string result = newTemp();
  emit(ctx->getStart()->getText() == "!" ? "not" : "neg", result, operand.place,
       {}, ctx);
  const Type type =
      ctx->getStart()->getText() == "!" ? TYPE_BOOLEAN : operand.type;
  releaseValue(operand);
  return TacValue{type, result};
}

std::any
TacGenerator::visitPrimaryExpr(CompiscriptParser::PrimaryExprContext *ctx) {
  if (ctx->literalExpr())
    return visit(ctx->literalExpr());
  if (ctx->leftHandSide())
    return visit(ctx->leftHandSide());
  return visit(ctx->expression());
}

std::any
TacGenerator::visitLiteralExpr(CompiscriptParser::LiteralExprContext *ctx) {
  if (ctx->arrayLiteral())
    return visit(ctx->arrayLiteral());
  Type type = TYPE_NULL;
  if (ctx->IntegerLiteral())
    type = TYPE_INTEGER;
  else if (ctx->FloatLiteral())
    type = TYPE_FLOAT;
  else if (ctx->StringLiteral())
    type = TYPE_STRING;
  else if (ctx->getText() == "true" || ctx->getText() == "false")
    type = TYPE_BOOLEAN;
  return TacValue{type, ctx->getText()};
}

std::any
TacGenerator::visitArrayLiteral(CompiscriptParser::ArrayLiteralContext *ctx) {
  const auto expressions = ctx->expression();
  const std::string result = newTemp();
  emit("new_list", result, std::to_string(expressions.size()), {}, ctx);
  Type element = TYPE_UNKNOWN;
  for (size_t index = 0; index < expressions.size(); ++index) {
    TacValue value = generateExpression(expressions[index]);
    emit("store_index", result, std::to_string(index), value.place, ctx);
    element = index == 0 ? value.type : commonType(element, value.type);
    releaseValue(value);
  }
  Type type = expressionType(ctx);
  if (type.isUnknown())
    type = listOf(element);
  return TacValue{type, result};
}

std::any TacGenerator::visitIdentifierExpr(
    CompiscriptParser::IdentifierExprContext *ctx) {
  const int id = boundSymbol(ctx);
  const Symbol *item = symbol(id);
  if (!item) {
    report("TAC_UNRESOLVED_IDENTIFIER",
           "El identificador no conserva el símbolo resuelto.", ctx);
    return TacValue{};
  }
  if (item->kind == "function" || item->kind == "method")
    return TacValue{TYPE_FUNCTION, item->label, id};
  if (item->kind == "class")
    return TacValue{TYPE_CLASS, item->label, id};
  if (item->kind == "field") {
    const std::string result = newTemp();
    emit("load_field", result, "%this", item->name, ctx);
    TacValue value{item->type, result, id, TacLocation::Field};
    value.base = "%this";
    value.member = item->name;
    return value;
  }
  return TacValue{item->type, storageName(id), id, TacLocation::Variable};
}

std::any TacGenerator::visitNewExpr(CompiscriptParser::NewExprContext *ctx) {
  const int class_id = boundSymbol(ctx);
  const Symbol *class_symbol = symbol(class_id);
  if (!class_symbol) {
    report("TAC_UNRESOLVED_CLASS",
           "La creación de objeto no conserva la clase resuelta.", ctx);
    return TacValue{};
  }
  const std::string result = newTemp();
  emit("new_object", result, class_symbol->name, {}, ctx);
  std::vector<TacValue> arguments = generateArguments(ctx->arguments());
  const int constructor =
      findClassMember(class_symbol->name, "constructor", true);
  if (constructor >= 0) {
    const Symbol *method = symbol(constructor);
    emit("param", {}, result, {}, ctx);
    for (const TacValue &argument : arguments)
      emit("param", {}, argument.place, {}, ctx);
    emit("call_method", {}, method ? method->label : "constructor",
         std::to_string(arguments.size()), ctx);
  }
  for (const TacValue &argument : arguments)
    releaseValue(argument);
  return TacValue{Type{class_symbol->name}, result};
}

std::any TacGenerator::visitThisExpr(CompiscriptParser::ThisExprContext *ctx) {
  (void)ctx;
  return TacValue{
      class_stack_.empty() ? TYPE_UNKNOWN : Type{class_stack_.back()}, "%this"};
}

std::any
TacGenerator::visitLeftHandSide(CompiscriptParser::LeftHandSideContext *ctx) {
  TacValue result = generateExpression(ctx->primaryAtom());
  for (auto *suffix : ctx->suffixOp()) {
    if (auto *call =
            dynamic_cast<CompiscriptParser::CallExprContext *>(suffix)) {
      result = emitCall(result, call);
      continue;
    }
    if (auto *index =
            dynamic_cast<CompiscriptParser::IndexExprContext *>(suffix)) {
      TacValue subscript = generateExpression(index->expression());
      const std::string loaded = newTemp();
      emit("load_index", loaded, result.place, subscript.place, index);
      TacValue indexed{expressionType(index), loaded, -1, TacLocation::Index};
      if (indexed.type.isUnknown() && result.type.isList())
        indexed.type = result.type.element();
      indexed.base = result.place;
      indexed.index = subscript.place;
      result = indexed;
      continue;
    }
    auto *property =
        dynamic_cast<CompiscriptParser::PropertyAccessExprContext *>(suffix);
    if (!property)
      continue;
    const int member_id = boundSymbol(property);
    const Symbol *member = symbol(member_id);
    if (!member) {
      report("TAC_UNRESOLVED_MEMBER",
             "El acceso no conserva el miembro resuelto.", property);
      return TacValue{};
    }
    if (member->kind == "method") {
      TacValue method{TYPE_FUNCTION, member->label, member_id};
      method.receiver = result.place;
      result = method;
      continue;
    }
    const std::string loaded = newTemp();
    emit("load_field", loaded, result.place, member->name, property);
    TacValue field{member->type, loaded, member_id, TacLocation::Field};
    field.base = result.place;
    field.member = member->name;
    result = field;
  }
  return result;
}

} // namespace compiscript
