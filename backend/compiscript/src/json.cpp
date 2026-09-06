// Serializa de forma segura todos los resultados para consumirlos desde el IDE.
#include "compiscript/json.h"

#include <iomanip>
#include <sstream>

namespace compiscript {

std::string quoteJson(const std::string &value) {
  std::ostringstream output;
  output << '"';
  for (unsigned char character : value) {
    switch (character) {
    case '"':
      output << "\\\"";
      break;
    case '\\':
      output << "\\\\";
      break;
    case '\b':
      output << "\\b";
      break;
    case '\f':
      output << "\\f";
      break;
    case '\n':
      output << "\\n";
      break;
    case '\r':
      output << "\\r";
      break;
    case '\t':
      output << "\\t";
      break;
    default:
      if (character < 0x20) {
        output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
               << static_cast<int>(character) << std::dec;
      } else {
        output << character;
      }
    }
  }
  output << '"';
  return output.str();
}

namespace {

const char *boolean(bool value) { return value ? "true" : "false"; }

std::string integerArray(const std::vector<int> &values) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < values.size(); ++index) {
    if (index)
      output << ',';
    output << values[index];
  }
  output << ']';
  return output.str();
}

template <typename Set> std::string setArray(const Set &values) {
  std::ostringstream output;
  output << '[';
  bool first = true;
  for (int value : values) {
    if (!first)
      output << ',';
    first = false;
    output << value;
  }
  output << ']';
  return output.str();
}

std::string stringArray(const std::vector<std::string> &values) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < values.size(); ++index) {
    if (index)
      output << ',';
    output << quoteJson(values[index]);
  }
  output << ']';
  return output.str();
}

std::string typeArray(const std::vector<Type> &values) {
  std::vector<std::string> names;
  for (const Type &value : values)
    names.push_back(value.name);
  return stringArray(names);
}

std::string diagnosticJson(const Diagnostic &diagnostic) {
  return "{\"code\":" + quoteJson(diagnostic.code) +
         ",\"category\":" + quoteJson(diagnostic.category) +
         ",\"severity\":" + quoteJson(diagnostic.severity) +
         ",\"message\":" + quoteJson(diagnostic.message) +
         ",\"line\":" + std::to_string(diagnostic.line) +
         ",\"column\":" + std::to_string(diagnostic.column) + "}";
}

std::string diagnosticsJson(const std::vector<Diagnostic> &diagnostics) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < diagnostics.size(); ++index) {
    if (index)
      output << ',';
    output << diagnosticJson(diagnostics[index]);
  }
  output << ']';
  return output.str();
}

std::string mapJson(const std::map<std::string, int> &values) {
  std::ostringstream output;
  output << '{';
  bool first = true;
  for (const auto &item : values) {
    if (!first)
      output << ',';
    first = false;
    output << quoteJson(item.first) << ':' << item.second;
  }
  output << '}';
  return output.str();
}

std::string semanticJson(const SemanticResult &semantic) {
  std::ostringstream output;
  output << "{\"ok\":" << boolean(semantic.ok)
         << ",\"skipped\":" << boolean(semantic.skipped)
         << ",\"reason\":" << quoteJson(semantic.reason)
         << ",\"diagnostics\":" << diagnosticsJson(semantic.diagnostics)
         << ",\"scopes\":[";
  for (size_t index = 0; index < semantic.scopes.size(); ++index) {
    if (index)
      output << ',';
    const Scope &scope = semantic.scopes[index];
    std::vector<int> symbol_ids;
    for (const auto &item : scope.symbols)
      symbol_ids.push_back(item.second);
    output << "{\"id\":" << scope.id << ",\"name\":" << quoteJson(scope.name)
           << ",\"kind\":" << quoteJson(scope.kind)
           << ",\"parent_id\":" << scope.parent_id
           << ",\"owner_symbol_id\":" << scope.owner_symbol_id
           << ",\"line\":" << scope.line
           << ",\"symbol_ids\":" << integerArray(symbol_ids)
           << ",\"children\":" << integerArray(scope.children) << '}';
  }
  output << "],\"symbols\":[";
  for (size_t index = 0; index < semantic.symbols.size(); ++index) {
    if (index)
      output << ',';
    const Symbol &symbol = semantic.symbols[index];
    output << "{\"id\":" << symbol.id << ",\"name\":" << quoteJson(symbol.name)
           << ",\"kind\":" << quoteJson(symbol.kind)
           << ",\"type\":" << quoteJson(symbol.type.name)
           << ",\"scope_id\":" << symbol.scope_id << ",\"line\":" << symbol.line
           << ",\"column\":" << symbol.column
           << ",\"mutable\":" << boolean(symbol.mutable_value)
           << ",\"initialized\":" << boolean(symbol.initialized)
           << ",\"parameters\":" << typeArray(symbol.parameter_types)
           << ",\"parameter_names\":" << stringArray(symbol.parameter_names)
           << ",\"return_type\":" << quoteJson(symbol.return_type.name)
           << ",\"owner_class\":" << quoteJson(symbol.owner_class)
           << ",\"base_class\":" << quoteJson(symbol.base_class)
           << ",\"function_scope_id\":" << symbol.function_scope_id
           << ",\"captures\":" << setArray(symbol.captures)
           << ",\"captured_by\":" << setArray(symbol.captured_by) << '}';
  }
  output << "],\"classes\":[";
  for (size_t index = 0; index < semantic.classes.size(); ++index) {
    if (index)
      output << ',';
    const ClassInfo &item = semantic.classes[index];
    output << "{\"name\":" << quoteJson(item.name)
           << ",\"base\":" << quoteJson(item.base)
           << ",\"symbol_id\":" << item.symbol_id
           << ",\"scope_id\":" << item.scope_id
           << ",\"fields\":" << mapJson(item.fields)
           << ",\"methods\":" << mapJson(item.methods) << '}';
  }
  output << "]}";
  return output.str();
}

} // namespace

std::string analysisToJson(const AnalysisResult &result) {
  std::vector<Diagnostic> all = result.syntax_diagnostics;
  all.insert(all.end(), result.semantic.diagnostics.begin(),
             result.semantic.diagnostics.end());
  std::ostringstream output;
  output << "{\"ok\":" << boolean(result.ok)
         << ",\"filename\":" << quoteJson(result.filename)
         << ",\"syntax_ok\":" << boolean(result.syntax_ok)
         << ",\"semantic_ok\":" << boolean(result.semantic_ok)
         << ",\"tokens\":[";
  for (size_t index = 0; index < result.tokens.size(); ++index) {
    if (index)
      output << ',';
    const TokenInfo &token = result.tokens[index];
    output << "{\"type\":" << quoteJson(token.type)
           << ",\"category\":" << quoteJson(token.category)
           << ",\"text\":" << quoteJson(token.text)
           << ",\"line\":" << token.line << ",\"column\":" << token.column
           << '}';
  }
  output << "],\"tree\":" << result.tree_json << ",\"syntax_diagnostics\":"
         << diagnosticsJson(result.syntax_diagnostics)
         << ",\"semantic\":" << semanticJson(result.semantic)
         << ",\"diagnostics\":" << diagnosticsJson(all)
         << ",\"summary\":{\"tokens\":" << result.tokens.size()
         << ",\"syntax_errors\":" << result.syntax_diagnostics.size()
         << ",\"semantic_errors\":" << result.semantic.diagnostics.size()
         << ",\"scopes\":" << result.semantic.scopes.size()
         << ",\"symbols\":" << result.semantic.symbols.size() << "}}";
  return output.str();
}

} // namespace compiscript
