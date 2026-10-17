// Modelos compartidos: tipos, símbolos, ámbitos, clases y diagnósticos.
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace compiscript {

// Tipo semántico completo; los sufijos [] representan listas anidadas.
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

// Error listo para mostrar, con categoría y ubicación en el código fuente.
struct Diagnostic {
  std::string code;
  std::string category;
  std::string severity = "error";
  std::string message;
  size_t line = 0;
  size_t column = 0;
};

// Entrada de la tabla de símbolos: declaración, firma y relaciones de closure.
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
  // Información añadida por la fase de código intermedio.
  size_t size = 0;
  int offset = -1;
  int lexical_depth = 0;
  std::string storage;
  std::string label;
};

// Entorno anidado con referencia a su padre, símbolos propios e hijos.
struct Scope {
  int id = -1;
  std::string name;
  std::string kind;
  int parent_id = -1;
  int owner_symbol_id = -1;
  size_t line = 0;
  std::map<std::string, int> symbols;
  std::vector<int> children;
  int lexical_depth = 0;
  size_t frame_size = 0;
};

// Índice de atributos y métodos declarados por cada clase.
struct ClassInfo {
  std::string name;
  std::string base;
  int symbol_id = -1;
  int scope_id = -1;
  std::map<std::string, int> fields;
  std::map<std::string, int> methods;
};

// Datos producidos exclusivamente por el recorrido semántico.
struct SemanticResult {
  bool ok = false;
  bool skipped = false;
  std::string reason;
  std::vector<Diagnostic> diagnostics;
  std::vector<Scope> scopes;
  std::vector<Symbol> symbols;
  std::vector<ClassInfo> classes;
};

// Cuádruplo de código de tres direcciones. Los campos vacíos no participan en
// la instrucción; `text` es exclusivamente la representación legible del
// cuádruplo estructurado.
struct TacInstruction {
  size_t index = 0;
  std::string op;
  std::string arg1;
  std::string arg2;
  std::string result;
  std::string text;
  size_t line = 0;
  size_t column = 0;
};

// Diseño de un registro de activación relativo al frame pointer. Los enlaces y
// direcciones ocupan una palabra de ocho bytes en el modelo abstracto.
struct ActivationRecord {
  int function_symbol_id = -1;
  int scope_id = -1;
  std::string name;
  std::string label;
  int lexical_depth = 0;
  int control_link_offset = 0;
  int access_link_offset = 8;
  int return_address_offset = 16;
  int return_value_offset = 24;
  size_t parameter_bytes = 0;
  size_t local_bytes = 0;
  size_t temporary_bytes = 0;
  size_t frame_size = 0;
  std::vector<int> parameter_symbols;
  std::vector<int> local_symbols;
  std::vector<int> captured_symbols;
};

// Producto de la fase de generación intermedia.
struct TacResult {
  bool ok = false;
  bool skipped = false;
  std::string reason;
  std::vector<Diagnostic> diagnostics;
  std::vector<TacInstruction> instructions;
  std::vector<ActivationRecord> activation_records;
  std::string text;
  size_t temporaries_created = 0;
  size_t temporaries_reused = 0;
  size_t max_live_temporaries = 0;
};

struct TokenInfo {
  std::string type;
  std::string category;
  std::string text;
  size_t line = 0;
  size_t column = 0;
};

// Resultado unificado de las fases léxica, sintáctica, semántica y TAC.
struct AnalysisResult {
  bool ok = false;
  bool syntax_ok = false;
  bool semantic_ok = false;
  bool tac_ok = false;
  std::string filename;
  std::vector<TokenInfo> tokens;
  std::vector<Diagnostic> syntax_diagnostics;
  SemanticResult semantic;
  TacResult tac;
  std::string tree_json = "null";
};

Type listOf(const Type &element);
Type commonType(const Type &left, const Type &right);
bool assignable(const Type &target, const Type &value,
                const std::set<std::string> &class_names);

} // namespace compiscript
