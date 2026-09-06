#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace compiscript {

struct Type {
  std::string name = "unknown";

  bool operator==(const Type &other) const { return name == other.name; }
  bool operator!=(const Type &other) const { return !(*this == other); }
  bool isNumeric() const { return name == "integer" || name == "float"; }
  bool isList() const {
    return name.size() >= 2 && name.substr(name.size() - 2) == "[]";
  }
  bool isUnknown() const { return name == "unknown" || name == "error"; }
  Type element() const {
    return isList() ? Type{name.substr(0, name.size() - 2)} : Type{};
  }
};

inline const Type TYPE_INTEGER{"integer"};
inline const Type TYPE_FLOAT{"float"};
inline const Type TYPE_STRING{"string"};
inline const Type TYPE_BOOLEAN{"boolean"};
inline const Type TYPE_NULL{"null"};
inline const Type TYPE_VOID{"void"};
inline const Type TYPE_UNKNOWN{"unknown"};
inline const Type TYPE_ERROR{"error"};
inline const Type TYPE_FUNCTION{"function"};
inline const Type TYPE_CLASS{"class"};

struct Diagnostic {
  std::string code;
  std::string category;
  std::string severity = "error";
  std::string message;
  size_t line = 0;
  size_t column = 0;
};

struct Symbol {
  int id = -1;
  std::string name;
  std::string kind;
  Type type;
  int scope_id = -1;
  size_t line = 0;
  size_t column = 0;
  bool mutable_value = true;
  bool initialized = true;
  std::vector<Type> parameter_types;
  std::vector<std::string> parameter_names;
  Type return_type = TYPE_VOID;
  std::string owner_class;
  std::string base_class;
  int function_scope_id = -1;
  std::set<int> captures;
  std::set<int> captured_by;
};

struct Scope {
  int id = -1;
  std::string name;
  std::string kind;
  int parent_id = -1;
  int owner_symbol_id = -1;
  size_t line = 0;
  std::map<std::string, int> symbols;
  std::vector<int> children;
};

struct ClassInfo {
  std::string name;
  std::string base;
  int symbol_id = -1;
  int scope_id = -1;
  std::map<std::string, int> fields;
  std::map<std::string, int> methods;
};

struct SemanticResult {
  bool ok = false;
  bool skipped = false;
  std::string reason;
  std::vector<Diagnostic> diagnostics;
  std::vector<Scope> scopes;
  std::vector<Symbol> symbols;
  std::vector<ClassInfo> classes;
};

struct TokenInfo {
  std::string type;
  std::string category;
  std::string text;
  size_t line = 0;
  size_t column = 0;
};

struct AnalysisResult {
  bool ok = false;
  bool syntax_ok = false;
  bool semantic_ok = false;
  std::string filename;
  std::vector<TokenInfo> tokens;
  std::vector<Diagnostic> syntax_diagnostics;
  SemanticResult semantic;
  std::string tree_json = "null";
};

Type listOf(const Type &element);
Type commonType(const Type &left, const Type &right);
bool assignable(const Type &target, const Type &value,
                const std::set<std::string> &class_names);

} // namespace compiscript
