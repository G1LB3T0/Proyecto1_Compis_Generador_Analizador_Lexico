// Backend MIPS32. Cada valor ocupa un par (payload, etiqueta) de palabras.
// Los descriptores de registros se vacían en fronteras de bloque y llamadas;
// todos los valores vivos tienen un slot estable en su registro de activación.
#include "compiscript/mips_generator.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace compiscript {
namespace {

std::string safe(const std::string &s) {
  std::string r;
  for (unsigned char c : s) r += std::isalnum(c) || c == '_' ? c : '_';
  return r;
}
int symbolId(const std::string &s) {
  auto p = s.rfind('#');
  if (p == std::string::npos) return -1;
  try { return std::stoi(s.substr(p + 1)); } catch (...) { return -1; }
}
bool temporary(const std::string &s) {
  return s.size() > 2 && s[0] == '%' && s[1] == 't' && std::isdigit(s[2]);
}
bool address(const std::string &s) {
  return !s.empty() && (s[0] == '%' || s[0] == '@' || s.rfind("env[", 0) == 0);
}
std::string decodeString(const std::string &s) {
  std::string r;
  for (size_t i = 1; i + 1 < s.size(); ++i) {
    char c = s[i];
    if (c == '\\' && i + 2 < s.size()) {
      c = s[++i];
      if (c == 'n') c = '\n'; else if (c == 't') c = '\t';
      else if (c == 'r') c = '\r'; else if (c == '0') c = '\0';
    }
    r += c;
  }
  return r;
}

struct Routine {
  int symbol = -1;
  int parent = -1;
  std::string label = "main";
  std::string class_name;
  bool method = false;
  bool initializer = false;
  std::vector<TacInstruction> code;
  std::map<std::string, int> slots;
  int frame_size = 64;
  std::map<std::string, int> label_depth;
};
struct Cache {
  std::string value;
  bool dirty = false;
  unsigned long age = 0;
};

class Generator {
public:
  Generator(const SemanticResult &s, const TacResult &t) : sem(s), tac(t) {}
  MipsResult run();
private:
  const SemanticResult &sem;
  const TacResult &tac;
  MipsResult result;
  std::vector<Routine> routines;
  std::map<int, size_t> by_symbol;
  std::map<std::string, size_t> by_label;
  std::map<std::string, size_t> initializers;
  std::map<std::string, std::string> strings;
  std::map<std::string, std::string> doubles;
  std::ostringstream text;
  Routine *current = nullptr;
  std::array<Cache, 3> cache;
  unsigned long age = 0;
  int unique = 0;
  const TacInstruction *source = nullptr;

  void line(const std::string &s) {
    text << s << '\n';
    if (!s.empty() && s[0] == ' ' && s.find('#') != 2) ++result.instruction_count;
  }
  std::string label(const std::string &prefix) { return "__cg_" + prefix + std::to_string(unique++); }
  void problem(const std::string &code, const std::string &message) {
    result.diagnostics.push_back({code, "generación MIPS", "error", message,
                                 source ? source->line : 0, source ? source->column : 0});
  }
  const Symbol *symbol(int id) const {
    return id >= 0 && static_cast<size_t>(id) < sem.symbols.size() ? &sem.symbols[id] : nullptr;
  }
  int owner(int scope) const {
    while (scope >= 0 && static_cast<size_t>(scope) < sem.scopes.size()) {
      const auto &s = sem.scopes[scope];
      if (s.kind == "function") return s.owner_symbol_id;
      scope = s.parent_id;
    }
    return -1;
  }
  int parentOf(int id) const { const Symbol *s = symbol(id); return s ? owner(s->scope_id) : -1; }
  std::string symbolName(const Symbol &s) const {
    return (s.storage == "global" ? "@" : "%") + safe(s.name) + "#" + std::to_string(s.id);
  }
  std::string stringLabel(const std::string &s) {
    auto it = strings.find(s);
    if (it != strings.end()) return it->second;
    return strings[s] = "__str_" + std::to_string(strings.size());
  }
  std::string doubleLabel(const std::string &s) {
    auto it = doubles.find(s);
    if (it != doubles.end()) return it->second;
    return doubles[s] = "__flt_" + std::to_string(doubles.size());
  }
  void extract();
  void prepare(Routine &r);
  void routine(Routine &r);
  void instruction(const TacInstruction &i, int &depth, std::vector<TacInstruction> &params);
  std::string p(int n) const { return "$s" + std::to_string(1 + n * 2); }
  std::string k(int n) const { return "$s" + std::to_string(2 + n * 2); }
  void ancestor(int target, const std::string &reg);
  void location(const std::string &value, const std::string &reg);
  void load(const std::string &value, const std::string &payload, const std::string &kind);
  void store(const std::string &value, const std::string &payload, const std::string &kind);
  void spill(int n);
  void flush();
  int getReg(const std::string &value, bool reading, int avoid = -1);
  void args(const std::string &a, const std::string &b = {});
  void write(const std::string &target, const std::string &type = {});
  void invoke(const std::string &name) { flush(); line("  jal " + name); line("  nop"); }
  void coerce(const std::string &type);
  const ClassInfo *classInfo(const std::string &name) const;
  const Symbol *field(const std::string &klass, const std::string &name) const;
  std::string typeOf(const std::string &value) const;
  int objectSize(const std::string &klass) const;
  void popHandlers(int count);
  void runtime();
};

void Generator::extract() {
  routines.emplace_back();
  std::vector<size_t> stack{0};
  for (const auto &i : tac.instructions) {
    if (i.op == "begin_func") {
      Routine r;
      r.label = i.result;
      for (const auto &s : sem.symbols) if (s.label == r.label) {
        r.symbol = s.id; r.parent = parentOf(s.id); r.method = s.kind == "method";
        r.class_name = s.owner_class;
        break;
      }
      size_t n = routines.size(); routines.push_back(std::move(r));
      by_symbol[routines[n].symbol] = n; by_label[i.result] = n;
      stack.push_back(n);
    } else if (i.op == "end_func") {
      if (stack.size() > 1) stack.pop_back();
    } else if (i.op == "begin_class") {
      Routine r; r.label = "__init_" + safe(i.result); r.initializer = true;
      r.class_name = i.result;
      if (const ClassInfo *info = classInfo(i.result)) {
        r.symbol = info->symbol_id;
        if (const Symbol *klass = symbol(r.symbol)) r.parent = owner(klass->scope_id);
      }
      size_t n = routines.size(); routines.push_back(std::move(r));
      by_symbol[routines[n].symbol] = n;
      initializers[i.result] = n; stack.push_back(n);
    } else if (i.op == "end_class") {
      if (stack.size() > 1) stack.pop_back();
    } else routines[stack.back()].code.push_back(i);
  }
  for (auto &r : routines) prepare(r);
}

void Generator::prepare(Routine &r) {
  auto add = [&](const std::string &v) {
    if (!address(v) || v == "%this" || v.rfind("env[", 0) == 0 || v[0] == '@') return;
    const Symbol *s = symbol(symbolId(v));
    if (s && owner(s->scope_id) != r.symbol) return;
    if (!r.slots.count(v)) r.slots[v] = 64 + static_cast<int>(r.slots.size()) * 8;
  };
  for (const auto &s : sem.symbols) if (s.storage != "global" &&
      s.kind != "field" && s.kind != "method" && s.kind != "function" &&
      s.kind != "class" && owner(s.scope_id) == r.symbol) add(symbolName(s));
  int depth = 0;
  for (const auto &i : r.code) {
    add(i.result); add(i.arg1); add(i.arg2);
    if (i.op == "try_begin") ++depth;
    if (i.op == "try_end") --depth;
    if (i.op == "label") r.label_depth[i.result] = depth;
  }
  r.frame_size = 64 + static_cast<int>(r.slots.size()) * 8;
  MipsFrame frame;
  frame.function_symbol_id = r.symbol; frame.parent_symbol_id = r.parent;
  frame.name = r.initializer ? r.class_name + ".<inicialización>" : (symbol(r.symbol) ? symbol(r.symbol)->name : "programa");
  frame.label = r.label; frame.frame_size = r.frame_size;
  for (const auto &s : r.slots) {
    if (temporary(s.first)) frame.temporary_bytes += 8;
    else frame.local_bytes += 8;
  }
  if (const Symbol *s = symbol(r.symbol)) frame.parameter_bytes = (s->parameter_types.size() + (r.method ? 1 : 0)) * 8;
  result.frames.push_back(frame);
}

void Generator::ancestor(int target, const std::string &reg) {
  if (target == -1) { line("  move " + reg + ", $zero"); return; }
  line("  move " + reg + ", $fp");
  int id = current->symbol;
  std::set<int> seen;
  while (id != target && id != -1 && !seen.count(id)) {
    seen.insert(id); line("  lw " + reg + ", 4(" + reg + ")"); id = parentOf(id);
  }
  if (id != target) problem("MIPS_STATIC_LINK", "No se pudo resolver el entorno léxico requerido.");
}

void Generator::location(const std::string &v, const std::string &reg) {
  if (v == "%this") { line("  addiu " + reg + ", $fp, 48"); return; }
  const Symbol *s = symbol(symbolId(v));
  if (s && s->storage == "global") { line("  la " + reg + ", __global_" + std::to_string(s->id)); return; }
  std::string name = s ? symbolName(*s) : v;
  int own = s ? owner(s->scope_id) : current->symbol;
  if (own == current->symbol) {
    auto it = current->slots.find(name);
    if (it == current->slots.end()) { problem("MIPS_STORAGE", "No existe almacenamiento para " + v); line("  move " + reg + ", $fp"); }
    else line("  addiu " + reg + ", $fp, " + std::to_string(it->second));
  } else {
    ancestor(own, reg);
    auto it = by_symbol.find(own);
    if (it == by_symbol.end() || !routines[it->second].slots.count(name)) problem("MIPS_CAPTURE", "No se encontró la variable capturada " + v);
    else line("  addiu " + reg + ", " + reg + ", " + std::to_string(routines[it->second].slots.at(name)));
  }
}

void Generator::load(const std::string &v, const std::string &payload, const std::string &kind) {
  if (address(v)) {
    location(v, "$t8"); line("  lw " + payload + ", 0($t8)"); line("  lw " + kind + ", 4($t8)");
  } else if (v.empty() || v == "null") {
    line("  move " + payload + ", $zero"); line("  move " + kind + ", $zero");
  } else if (v == "true" || v == "false") {
    line("  li " + payload + ", " + (v == "true" ? "1" : "0")); line("  li " + kind + ", 2");
  } else if (v.front() == '"') {
    line("  la " + payload + ", " + stringLabel(decodeString(v))); line("  li " + kind + ", 3");
  } else if (v.rfind("fn_", 0) == 0) {
    line("  la " + payload + ", " + v); line("  li " + kind + ", 8");
  } else if (v.find('.') != std::string::npos) {
    try {
      if (!std::isfinite(std::stod(v))) throw std::out_of_range("float");
      line("  la " + payload + ", " + doubleLabel(v)); line("  li " + kind + ", 4");
    } catch (...) { problem("MIPS_FLOAT_RANGE", "El literal flotante queda fuera del rango finito IEEE754 double."); line("  move " + payload + ", $zero"); line("  move " + kind + ", $zero"); }
  } else {
    try {
      size_t end; long long n = std::stoll(v, &end);
      if (end != v.size() || n < -2147483648LL || n > 2147483647LL) throw std::out_of_range("integer");
      line("  li " + payload + ", " + v); line("  li " + kind + ", 1");
    } catch (...) { problem("MIPS_INTEGER_RANGE", "El literal no se puede representar en MIPS32: " + v); line("  move " + payload + ", $zero"); line("  move " + kind + ", $zero"); }
  }
}
void Generator::store(const std::string &v, const std::string &payload, const std::string &kind) {
  location(v, "$t8"); line("  sw " + payload + ", 0($t8)"); line("  sw " + kind + ", 4($t8)");
}
void Generator::spill(int n) {
  if (cache[n].dirty) { store(cache[n].value, p(n), k(n)); ++result.spills; cache[n].dirty = false; }
}
void Generator::flush() {
  for (int n = 0; n < 3; ++n) { spill(n); cache[n] = {}; }
}
int Generator::getReg(const std::string &v, bool reading, int avoid) {
  for (int n = 0; n < 3; ++n) if (cache[n].value == v && !v.empty()) { cache[n].age = ++age; ++result.register_allocations; return n; }
  int n = -1;
  for (int j = 0; j < 3; ++j) if (j != avoid && cache[j].value.empty()) { n = j; break; }
  if (n < 0) for (int j = 0; j < 3; ++j) if (j != avoid && (n < 0 || cache[j].age < cache[n].age)) n = j;
  spill(n); cache[n] = {v, false, ++age}; ++result.register_allocations;
  if (reading) load(v, p(n), k(n));
  return n;
}
void Generator::args(const std::string &a, const std::string &b) {
  int x = getReg(a, true); line("  move $a0, " + p(x)); line("  move $a1, " + k(x));
  if (!b.empty()) { int y = getReg(b, true, x); line("  move $a2, " + p(y)); line("  move $a3, " + k(y)); }
}
void Generator::coerce(const std::string &type) {
  if (type == "float") { line("  move $a0, $v0"); line("  move $a1, $v1"); invoke("__to_float"); }
}
void Generator::write(const std::string &target, const std::string &type) {
  coerce(type);
  int n = getReg(target, false); line("  move " + p(n) + ", $v0"); line("  move " + k(n) + ", $v1"); cache[n].dirty = true;
}
const ClassInfo *Generator::classInfo(const std::string &name) const {
  for (const auto &c : sem.classes) if (c.name == name) return &c;
  return nullptr;
}
const Symbol *Generator::field(const std::string &klass, const std::string &name) const {
  std::string c = klass; std::set<std::string> seen;
  while (!c.empty() && !seen.count(c)) {
    seen.insert(c); const ClassInfo *info = classInfo(c); if (!info) return nullptr;
    auto it = info->fields.find(name); if (it != info->fields.end()) return symbol(it->second);
    c = info->base;
  }
  return nullptr;
}
std::string Generator::typeOf(const std::string &v) const {
  if (v == "%this") return current->class_name;
  if (const Symbol *s = symbol(symbolId(v))) return s->type.name;
  return {};
}
int Generator::objectSize(const std::string &klass) const {
  int size = 8; std::string c = klass; std::set<std::string> seen;
  while (!c.empty() && !seen.count(c)) {
    seen.insert(c); const ClassInfo *info = classInfo(c); if (!info) break;
    for (auto f : info->fields) if (const Symbol *s = symbol(f.second)) size = std::max(size, s->offset + 16);
    c = info->base;
  }
  return size;
}
void Generator::popHandlers(int count) {
  for (int n = 0; n < count; ++n) {
    line("  lw $t8, __handler"); line("  lw $t9, 0($t8)"); line("  sw $t9, __handler"); line("  lw $sp, 12($t8)");
  }
}

void Generator::instruction(const TacInstruction &i, int &depth, std::vector<TacInstruction> &params) {
  source = &i;
  line("  # TAC " + std::to_string(i.index) + ": " + i.text);
  const auto &op = i.op;
  static const std::set<std::string> binary{"add","sub","mul","div","mod","eq","ne","lt","le","gt","ge"};
  if (op == "begin_program" || op == "end_program" || op == "capture" || op == "static_link") return;
  if (op == "declare") {
    load(i.arg1 == "boolean" ? "false" : (i.arg1 == "integer" || i.arg1 == "float" ? "0" : "null"), "$v0", "$v1");
    write(i.result, i.arg1); return;
  }
  if (op == "assign") {
    args(i.arg1); line("  move $v0, $a0"); line("  move $v1, $a1");
    write(i.result, i.result_type.empty() ? typeOf(i.result) : i.result_type); return;
  }
  if (binary.count(op)) {
    args(i.arg1, i.arg2); invoke("__" + op); write(i.result, i.result_type); return;
  }
  if (op == "neg" || op == "not") {
    args(i.arg1); invoke("__" + op); write(i.result, i.result_type); return;
  }
  if (op == "label") { flush(); line(i.result + ":"); return; }
  if (op == "goto") {
    flush(); auto target = current->label_depth.find(i.result);
    if (target != current->label_depth.end() && depth > target->second) popHandlers(depth - target->second);
    line("  j " + i.result); line("  nop"); return;
  }
  if (op == "if_true" || op == "if_false") {
    args(i.arg1); flush();
    auto target = current->label_depth.find(i.result);
    if (target != current->label_depth.end() && depth > target->second) {
      std::string skip = label("keep_handler_");
      line(std::string("  ") + (op == "if_true" ? "beq" : "bne") + " $a0, $zero, " + skip); line("  nop");
      popHandlers(depth - target->second); line("  j " + i.result); line("  nop"); line(skip + ":");
    } else { line(std::string("  ") + (op == "if_true" ? "bne" : "beq") + " $a0, $zero, " + i.result); line("  nop"); }
    return;
  }
  if (op == "print") {
    args(i.arg1); invoke("__print"); line("  li $v0, 4"); line("  la $a0, __newline"); line("  syscall"); return;
  }
  if (op == "param") { params.push_back(i); return; }
  if (op == "param_in") {
    if (i.arg1 == "receiver") return;
    int n = std::stoi(i.arg1) + (current->method ? 1 : 0);
    int offset = current->frame_size + n * 8;
    line("  lw $v0, " + std::to_string(offset) + "($fp)");
    line("  lw $v1, " + std::to_string(offset + 4) + "($fp)");
    write(i.result, typeOf(i.result)); return;
  }
  if (op == "call" || op == "call_method") {
    flush(); int bytes = static_cast<int>(params.size()) * 8;
    if (bytes) line("  addiu $sp, $sp, -" + std::to_string(bytes));
    auto callee = by_label.find(i.arg1);
    const Routine *r = callee == by_label.end() ? nullptr : &routines[callee->second];
    const Symbol *fn = r ? symbol(r->symbol) : nullptr;
    for (size_t n = 0; n < params.size(); ++n) {
      load(params[n].arg1, "$v0", "$v1");
      int formal = static_cast<int>(n) - (r && r->method ? 1 : 0);
      if (fn && formal >= 0 && static_cast<size_t>(formal) < fn->parameter_types.size()) coerce(fn->parameter_types[formal].name);
      line("  sw $v0, " + std::to_string(n * 8) + "($sp)"); line("  sw $v1, " + std::to_string(n * 8 + 4) + "($sp)");
    }
    if (!r) problem("MIPS_CALL", "No existe la rutina " + i.arg1);
    ancestor(r ? r->parent : -1, "$a0"); line("  jal " + i.arg1); line("  nop");
    if (bytes) line("  addiu $sp, $sp, " + std::to_string(bytes));
    params.clear();
    if (!i.result.empty()) write(i.result, fn ? fn->return_type.name : i.result_type);
    return;
  }
  if (op == "return") {
    args(i.arg1); line("  move $v0, $a0"); line("  move $v1, $a1");
    const Symbol *fn = symbol(current->symbol); coerce(fn ? fn->return_type.name : std::string{});
    flush(); line("  j " + current->label + "__return"); line("  nop"); return;
  }
  if (op == "new_list") {
    args(i.arg1); invoke("__new_list"); write(i.result); return;
  }
  if (op == "length") {
    args(i.arg1); flush(); line("  beq $a0, $zero, __null_error"); line("  nop");
    line("  lw $v0, 0($a0)"); line("  li $v1, 1"); write(i.result); return;
  }
  if (op == "load_index") {
    args(i.arg1, i.arg2); invoke("__index");
    line("  lw $v1, 4($v0)"); line("  lw $v0, 0($v0)"); write(i.result, i.result_type); return;
  }
  if (op == "store_index") {
    args(i.result, i.arg1); invoke("__index");
    line("  addiu $sp, $sp, -8"); line("  sw $v0, 0($sp)");
    load(i.arg2, "$v0", "$v1"); coerce(i.result_type);
    line("  lw $t7, 0($sp)"); line("  addiu $sp, $sp, 8"); line("  sw $v0, 0($t7)"); line("  sw $v1, 4($t7)"); return;
  }
  if (op == "new_object") {
    flush(); line("  li $a0, " + std::to_string(objectSize(i.arg1))); invoke("__alloc_zero");
    line("  la $t9, " + stringLabel(i.arg1)); line("  sw $t9, 0($v0)"); line("  li $v1, 7"); write(i.result); flush();
    auto init = initializers.find(i.arg1);
    if (init != initializers.end()) {
      load(i.result, "$t0", "$t1"); line("  addiu $sp, $sp, -8"); line("  sw $t0, 0($sp)"); line("  sw $t1, 4($sp)");
      ancestor(routines[init->second].parent, "$a0"); line("  jal " + routines[init->second].label); line("  nop"); line("  addiu $sp, $sp, 8");
    }
    return;
  }
  if (op == "field_decl" || op == "field_init" || op == "load_field" || op == "store_field") {
    std::string base = (op == "field_decl" || op == "field_init") ? "%this" : (op == "load_field" ? i.arg1 : i.result);
    std::string name = (op == "field_decl" || op == "field_init") ? i.result : (op == "load_field" ? i.arg2 : i.arg1);
    std::string klass = (op == "field_decl" || op == "field_init") ? current->class_name : (i.arg1_type.empty() ? typeOf(base) : i.arg1_type);
    const Symbol *member = field(klass, name);
    if (!member) { problem("MIPS_FIELD", "No se pudo resolver el atributo " + klass + "." + name); return; }
    if (op == "field_decl") {
      load(member->type == TYPE_BOOLEAN ? "false" : (member->type.isNumeric() ? "0" : "null"), "$v0", "$v1"); coerce(member->type.name);
    } else if (op != "load_field") {
      args(op == "field_init" ? i.arg1 : i.arg2); line("  move $v0, $a0"); line("  move $v1, $a1"); coerce(member->type.name);
    }
    flush(); load(base, "$t0", "$t1"); line("  beq $t0, $zero, __null_error"); line("  nop");
    int offset = member->offset + 8;
    if (op == "load_field") {
      line("  lw $v1, " + std::to_string(offset + 4) + "($t0)"); line("  lw $v0, " + std::to_string(offset) + "($t0)"); write(i.result, member->type.name);
    } else { line("  sw $v0, " + std::to_string(offset) + "($t0)"); line("  sw $v1, " + std::to_string(offset + 4) + "($t0)"); }
    return;
  }
  if (op == "try_begin") {
    flush(); line("  move $t9, $sp"); line("  addiu $sp, $sp, -64");
    line("  lw $t8, __handler"); line("  sw $t8, 0($sp)"); line("  la $t8, " + i.result); line("  sw $t8, 4($sp)");
    line("  sw $fp, 8($sp)"); line("  sw $t9, 12($sp)");
    for (int n = 0; n < 8; ++n) line("  sw $s" + std::to_string(n) + ", " + std::to_string(16 + n * 4) + "($sp)");
    line("  sw $sp, __handler"); ++depth; return;
  }
  if (op == "try_end") { flush(); popHandlers(1); --depth; return; }
  if (op == "catch") { write(i.result); return; }
  problem("MIPS_UNSUPPORTED_TAC", "Operación TAC sin traducción: " + op);
}

void Generator::routine(Routine &r) {
  current = &r; cache = {}; age = 0;
  line(""); line(r.label + ":");
  if (r.label == "main") { line("  li $t8, -8"); line("  and $sp, $sp, $t8"); }
  line("  addiu $sp, $sp, -" + std::to_string(r.frame_size));
  line("  sw $fp, 0($sp)"); line("  sw $a0, 4($sp)"); line("  sw $ra, 8($sp)");
  for (int n = 0; n < 8; ++n) line("  sw $s" + std::to_string(n) + ", " + std::to_string(12 + n * 4) + "($sp)");
  line("  lw $t8, __handler"); line("  sw $t8, 44($sp)"); line("  move $fp, $sp");
  // Initially zero every slot, making defaults and unused return paths defined.
  for (auto slot : r.slots) { line("  sw $zero, " + std::to_string(slot.second) + "($fp)"); line("  sw $zero, " + std::to_string(slot.second + 4) + "($fp)"); }
  if (r.method || r.initializer) {
    line("  lw $s0, " + std::to_string(r.frame_size) + "($fp)");
    line("  sw $s0, 48($fp)"); line("  li $t8, 7"); line("  sw $t8, 52($fp)");
  } else {
    int method = r.parent;
    while (method >= 0 && symbol(method) && symbol(method)->kind != "method") method = parentOf(method);
    if (method >= 0) {
      ancestor(method, "$t8"); line("  lw $s0, 48($t8)"); line("  sw $s0, 48($fp)"); line("  li $t8, 7"); line("  sw $t8, 52($fp)");
      if (const Symbol *m = symbol(method)) r.class_name = m->owner_class;
    } else { line("  sw $zero, 48($fp)"); line("  sw $zero, 52($fp)"); }
  }
  if (r.initializer) {
    const ClassInfo *info = classInfo(r.class_name);
    if (info && !info->base.empty() && initializers.count(info->base)) {
      line("  addiu $sp, $sp, -8"); line("  sw $s0, 0($sp)"); line("  li $t8, 7"); line("  sw $t8, 4($sp)");
      ancestor(routines[initializers.at(info->base)].parent, "$a0"); line("  jal " + routines[initializers.at(info->base)].label); line("  nop"); line("  addiu $sp, $sp, 8");
    }
  }
  int depth = 0; std::vector<TacInstruction> params;
  for (const auto &i : r.code) instruction(i, depth, params);
  if (!params.empty()) problem("MIPS_ARGUMENTS", "Hay parámetros TAC sin llamada asociada.");
  flush(); line("  move $v0, $zero"); line("  move $v1, $zero");
  line(r.label + "__return:");
  line("  lw $t8, 44($fp)"); line("  sw $t8, __handler");
  line("  move $sp, $fp"); line("  lw $ra, 8($sp)");
  for (int n = 0; n < 8; ++n) line("  lw $s" + std::to_string(n) + ", " + std::to_string(12 + n * 4) + "($sp)");
  line("  lw $fp, 0($sp)"); line("  addiu $sp, $sp, " + std::to_string(r.frame_size));
  if (r.label == "main") { line("  li $v0, 10"); line("  syscall"); }
  else { line("  jr $ra"); line("  nop"); }
}

void Generator::runtime() {
  text << R"ASM(
# Runtime de valores: null=0, integer=1, boolean=2, string=3,
# float(boxed double)=4, list=6, object=7, function=8.
__alloc_zero:
  move $t6, $a0
  addiu $a0, $a0, 7
  li $v0, 9
  syscall
  addiu $v0, $v0, 7
  li $t9, -8
  and $v0, $v0, $t9
  move $t7, $v0
__alloc_zero_loop:
  blez $t6, __alloc_zero_done
  nop
  sb $zero, 0($t7)
  addiu $t7, $t7, 1
  addiu $t6, $t6, -1
  j __alloc_zero_loop
  nop
__alloc_zero_done:
  jr $ra
  nop
__new_list:
  addiu $sp, $sp, -16
  sw $ra, 0($sp)
  sw $a0, 4($sp)
  bltz $a0, __index_error
  nop
  sll $a0, $a0, 3
  addiu $a0, $a0, 8
  jal __alloc_zero
  nop
  lw $t0, 4($sp)
  sw $t0, 0($v0)
  li $v1, 6
  lw $ra, 0($sp)
  addiu $sp, $sp, 16
  jr $ra
  nop
__index:
  beq $a0, $zero, __null_error
  nop
  bltz $a2, __index_error
  nop
  lw $t0, 0($a0)
  bge $a2, $t0, __index_error
  nop
  sll $t0, $a2, 3
  addu $v0, $a0, $t0
  addiu $v0, $v0, 8
  jr $ra
  nop
__number_left:
  li $t9, 4
  beq $a1, $t9, __number_left_float
  nop
  mtc1 $a0, $f0
  cvt.d.w $f0, $f0
  jr $ra
  nop
__number_left_float:
  l.d $f0, 0($a0)
  jr $ra
  nop
__number_right:
  li $t9, 4
  beq $a3, $t9, __number_right_float
  nop
  mtc1 $a2, $f2
  cvt.d.w $f2, $f2
  jr $ra
  nop
__number_right_float:
  l.d $f2, 0($a2)
  jr $ra
  nop
__box_result:
  mfc1 $t9, $f1
  li $t8, 2146435072
  and $t9, $t9, $t8
  beq $t9, $t8, __numeric_error
  nop
  li $a0, 16
  li $v0, 9
  syscall
  addiu $v0, $v0, 7
  li $t9, -8
  and $v0, $v0, $t9
  s.d $f0, 0($v0)
  li $v1, 4
  jr $ra
  nop
__to_float:
  li $t9, 4
  beq $a1, $t9, __to_float_done
  nop
  addiu $sp, $sp, -8
  sw $ra, 0($sp)
  jal __number_left
  nop
  jal __box_result
  nop
  lw $ra, 0($sp)
  addiu $sp, $sp, 8
  jr $ra
  nop
__to_float_done:
  move $v0, $a0
  move $v1, $a1
  jr $ra
  nop
__not:
  seq $v0, $a0, $zero
  li $v1, 2
  jr $ra
  nop
__neg:
  li $t9, 4
  beq $a1, $t9, __neg_float
  nop
  subu $v0, $zero, $a0
  li $v1, 1
  jr $ra
  nop
__neg_float:
  l.d $f0, 0($a0)
  neg.d $f0, $f0
  j __box_result
  nop
)ASM";
  for (const auto &op : std::vector<std::string>{"add","sub","mul","div","mod"}) {
    line("__" + op + ":");
    if (op == "add") { line("  li $t9, 3"); line("  beq $a1, $t9, __concat"); line("  nop"); }
    if (op != "div") {
      line("  li $t9, 4"); line("  beq $a1, $t9, __" + op + "_float"); line("  nop");
      line("  beq $a3, $t9, __" + op + "_float"); line("  nop");
      if (op == "mod") {
        line("  beq $a2, $zero, __division_error"); line("  nop"); line("  div $a0, $a2"); line("  mfhi $v0");
      } else line("  " + std::string(op == "add" ? "addu" : op == "sub" ? "subu" : "mul") + " $v0, $a0, $a2");
      line("  li $v1, 1"); line("  jr $ra"); line("  nop");
    }
    line("__" + op + "_float:"); line("  addiu $sp, $sp, -8"); line("  sw $ra, 0($sp)");
    line("  jal __number_left"); line("  nop"); line("  jal __number_right"); line("  nop");
    if (op == "div" || op == "mod") {
      line("  mtc1 $zero, $f4"); line("  cvt.d.w $f4, $f4"); line("  c.eq.d $f2, $f4"); line("  bc1t __division_error"); line("  nop");
    }
    if (op == "mod") {
      // Resto por reducción binaria: no convierte el cociente a integer32.
      line("  c.lt.d $f0, $f4"); line("  li $t0, 0"); line("  bc1f __mod_abs"); line("  nop"); line("  li $t0, 1");
      line("__mod_abs:"); line("  abs.d $f0, $f0"); line("  abs.d $f2, $f2");
      line("__mod_reduce:"); line("  c.lt.d $f0, $f2"); line("  bc1t __mod_sign"); line("  nop"); line("  mov.d $f4, $f2");
      line("__mod_double:"); line("  add.d $f8, $f4, $f4"); line("  c.le.d $f8, $f0"); line("  bc1f __mod_subtract"); line("  nop"); line("  mov.d $f4, $f8"); line("  j __mod_double"); line("  nop");
      line("__mod_subtract:"); line("  sub.d $f0, $f0, $f4"); line("  j __mod_reduce"); line("  nop");
      line("__mod_sign:"); line("  beq $t0, $zero, __mod_box"); line("  nop"); line("  neg.d $f0, $f0"); line("__mod_box:");
    } else line("  " + op + ".d $f0, $f0, $f2");
    line("  jal __box_result"); line("  nop"); line("  lw $ra, 0($sp)"); line("  addiu $sp, $sp, 8"); line("  jr $ra"); line("  nop");
  }
  text << R"ASM(
__compare:
  li $t9, 3
  bne $a1, $t9, __compare_numeric_check
  nop
  bne $a3, $t9, __compare_integer
  nop
  beq $a0, $zero, __compare_integer
  nop
  beq $a2, $zero, __compare_integer
  nop
__compare_string_loop:
  lbu $t0, 0($a0)
  lbu $t1, 0($a2)
  bne $t0, $t1, __compare_string_different
  nop
  beq $t0, $zero, __compare_equal
  nop
  addiu $a0, $a0, 1
  addiu $a2, $a2, 1
  j __compare_string_loop
  nop
__compare_string_different:
  subu $v0, $t0, $t1
  jr $ra
  nop
__compare_numeric_check:
  li $t9, 4
  beq $a1, $t9, __compare_float
  nop
  beq $a3, $t9, __compare_float
  nop
__compare_integer:
  slt $t0, $a0, $a2
  slt $t1, $a2, $a0
  subu $v0, $t1, $t0
  jr $ra
  nop
__compare_equal:
  move $v0, $zero
  jr $ra
  nop
__compare_float:
  addiu $sp, $sp, -8
  sw $ra, 0($sp)
  jal __number_left
  nop
  jal __number_right
  nop
  c.eq.d $f0, $f2
  bc1t __compare_float_equal
  nop
  c.lt.d $f0, $f2
  bc1t __compare_float_less
  nop
  li $v0, 1
  j __compare_float_end
  nop
__compare_float_less:
  li $v0, -1
  j __compare_float_end
  nop
__compare_float_equal:
  move $v0, $zero
__compare_float_end:
  lw $ra, 0($sp)
  addiu $sp, $sp, 8
  jr $ra
  nop
)ASM";
  for (const auto &op : std::vector<std::string>{"eq","ne","lt","le","gt","ge"}) {
    line("__" + op + ":"); line("  addiu $sp, $sp, -8"); line("  sw $ra, 0($sp)");
    line("  jal __compare"); line("  nop");
    if (op == "eq") line("  seq $v0, $v0, $zero");
    if (op == "ne") line("  sne $v0, $v0, $zero");
    if (op == "lt") line("  slt $v0, $v0, $zero");
    if (op == "gt") line("  slt $v0, $zero, $v0");
    if (op == "le") { line("  slt $v0, $zero, $v0"); line("  xori $v0, $v0, 1"); }
    if (op == "ge") { line("  slt $v0, $v0, $zero"); line("  xori $v0, $v0, 1"); }
    line("  li $v1, 2"); line("  lw $ra, 0($sp)"); line("  addiu $sp, $sp, 8"); line("  jr $ra"); line("  nop");
  }
  text << R"ASM(
__concat:
  beq $a0, $zero, __null_error
  nop
  beq $a2, $zero, __null_error
  nop
  addiu $sp, $sp, -24
  sw $ra, 0($sp)
  sw $a0, 4($sp)
  sw $a2, 8($sp)
  move $t0, $a0
  move $t1, $zero
__concat_len1:
  lbu $t2, 0($t0)
  beq $t2, $zero, __concat_len2_start
  nop
  addiu $t1, $t1, 1
  addiu $t0, $t0, 1
  j __concat_len1
  nop
__concat_len2_start:
  move $t0, $a2
__concat_len2:
  lbu $t2, 0($t0)
  beq $t2, $zero, __concat_alloc
  nop
  addiu $t1, $t1, 1
  addiu $t0, $t0, 1
  j __concat_len2
  nop
__concat_alloc:
  addiu $a0, $t1, 1
  jal __alloc_zero
  nop
  move $t0, $v0
  lw $t1, 4($sp)
__concat_copy1:
  lbu $t2, 0($t1)
  beq $t2, $zero, __concat_copy2_start
  nop
  sb $t2, 0($t0)
  addiu $t0, $t0, 1
  addiu $t1, $t1, 1
  j __concat_copy1
  nop
__concat_copy2_start:
  lw $t1, 8($sp)
__concat_copy2:
  lbu $t2, 0($t1)
  sb $t2, 0($t0)
  beq $t2, $zero, __concat_done
  nop
  addiu $t0, $t0, 1
  addiu $t1, $t1, 1
  j __concat_copy2
  nop
__concat_done:
  li $v1, 3
  lw $ra, 0($sp)
  addiu $sp, $sp, 24
  jr $ra
  nop
__print:
  beq $a1, $zero, __print_null
  nop
  li $t9, 2
  beq $a1, $t9, __print_boolean
  nop
  li $t9, 3
  beq $a1, $t9, __print_string
  nop
  li $t9, 4
  beq $a1, $t9, __print_float
  nop
  li $t9, 6
  beq $a1, $t9, __print_list
  nop
  li $t9, 7
  beq $a1, $t9, __print_object
  nop
  li $t9, 8
  beq $a1, $t9, __print_function
  nop
  li $v0, 1
  syscall
  jr $ra
  nop
__print_float:
  l.d $f12, 0($a0)
  li $v0, 3
  syscall
  jr $ra
  nop
__print_boolean:
  beq $a0, $zero, __print_false
  nop
  la $a0, __true
  j __print_string
  nop
__print_false:
  la $a0, __false
  j __print_string
  nop
__print_null:
  la $a0, __null
  j __print_string
  nop
__print_function:
  la $a0, __function
  j __print_string
  nop
__print_object:
  beq $a0, $zero, __print_null
  nop
  lw $a0, 0($a0)
  j __print_string
  nop
__print_string:
  beq $a0, $zero, __print_null
  nop
__write_string:
  move $t0, $a0
  move $t1, $zero
__write_string_length:
  lbu $t2, 0($t0)
  beq $t2, $zero, __write_string_output
  nop
  addiu $t0, $t0, 1
  addiu $t1, $t1, 1
  j __write_string_length
  nop
__write_string_output:
  move $a1, $a0
  move $a2, $t1
  li $a0, 1
  li $v0, 15
  syscall
  jr $ra
  nop
__print_list:
  beq $a0, $zero, __print_null
  nop
  addiu $sp, $sp, -24
  sw $ra, 0($sp)
  sw $a0, 4($sp)
  sw $zero, 8($sp)
  li $v0, 4
  la $a0, __open_list
  syscall
__print_list_loop:
  lw $t0, 4($sp)
  lw $t1, 8($sp)
  lw $t2, 0($t0)
  bge $t1, $t2, __print_list_done
  nop
  beq $t1, $zero, __print_list_item
  nop
  li $v0, 4
  la $a0, __comma
  syscall
__print_list_item:
  sll $t2, $t1, 3
  addu $t0, $t0, $t2
  lw $a0, 8($t0)
  lw $a1, 12($t0)
  jal __print
  nop
  lw $t1, 8($sp)
  addiu $t1, $t1, 1
  sw $t1, 8($sp)
  j __print_list_loop
  nop
__print_list_done:
  li $v0, 4
  la $a0, __close_list
  syscall
  lw $ra, 0($sp)
  addiu $sp, $sp, 24
  jr $ra
  nop
__numeric_error:
  la $a0, __error_numeric
  j __raise
  nop
__division_error:
  la $a0, __error_division
  j __raise
  nop
__index_error:
  la $a0, __error_index
  j __raise
  nop
__null_error:
  la $a0, __error_null
  j __raise
  nop
__raise:
  lw $t9, __handler
  beq $t9, $zero, __uncaught
  nop
  move $t7, $a0
  lw $t8, 0($t9)
  sw $t8, __handler
  lw $t8, 4($t9)
  lw $fp, 8($t9)
  lw $sp, 12($t9)
  lw $s0, 16($t9)
  lw $s1, 20($t9)
  lw $s2, 24($t9)
  lw $s3, 28($t9)
  lw $s4, 32($t9)
  lw $s5, 36($t9)
  lw $s6, 40($t9)
  lw $s7, 44($t9)
  move $v0, $t7
  li $v1, 3
  jr $t8
  nop
__uncaught:
  move $s7, $a0
  la $a0, __error_prefix
  jal __write_string
  nop
  move $a0, $s7
  jal __write_string
  nop
  la $a0, __newline
  li $v0, 4
  syscall
  li $a0, 2
  li $v0, 17
  syscall
)ASM";
}

MipsResult Generator::run() {
  if (!sem.ok || !tac.ok || tac.skipped) {
    result.skipped = true; result.reason = "La generación MIPS requiere un análisis semántico y TAC válidos."; return result;
  }
  extract();
  line(".text"); line(".globl main");
  for (auto &r : routines) routine(r);
  runtime();
  std::ostringstream data;
  data << "# Compiscript - MIPS32 / MARS. Valores tagged y pila con slots de 8 bytes.\n.data\n.align 3\n__handler: .word 0\n";
  for (const auto &s : sem.symbols) if (s.storage == "global" && s.kind != "class" && s.kind != "function" && s.kind != "method")
    data << ".align 3\n__global_" << s.id << ": .word 0, 0\n";
  auto bytes = [&](const std::string &name, const std::string &s) {
    data << name << ": .byte ";
    for (unsigned char c : s) data << (c < 128 ? static_cast<int>(c) : static_cast<int>(c) - 256) << ',';
    data << "0\n";
  };
  for (const auto &entry : strings) bytes(entry.second, entry.first);
  for (const auto &entry : doubles) data << ".align 3\n" << entry.second << ": .double " << entry.first << '\n';
  bytes("__newline", "\n"); bytes("__true", "true"); bytes("__false", "false"); bytes("__null", "null");
  bytes("__function", "[function]"); bytes("__open_list", "["); bytes("__close_list", "]"); bytes("__comma", ", ");
  bytes("__error_prefix", "Error de ejecución: "); bytes("__error_division", "división por cero");
  bytes("__error_numeric", "resultado numérico no finito");
  bytes("__error_index", "índice fuera de rango"); bytes("__error_null", "referencia nula");
  result.ok = result.diagnostics.empty();
  if (result.ok) result.text = data.str() + text.str();
  // Cuenta final incluyendo las rutinas auxiliares emitidas como bloques.
  result.instruction_count = 0;
  std::istringstream assembly(result.text); std::string l;
  while (std::getline(assembly, l)) {
    auto first = l.find_first_not_of(" \t");
    if (first != std::string::npos && first > 0 && l[first] != '#' && l[first] != '.') ++result.instruction_count;
  }
  return result;
}

} // namespace

MipsResult MipsGenerator::generate() { return Generator(semantic_, tac_).run(); }

} // namespace compiscript
