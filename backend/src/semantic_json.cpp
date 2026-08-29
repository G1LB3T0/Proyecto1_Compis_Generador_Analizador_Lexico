#include "semantic_json.h"

namespace {

std::string quote(const std::string& value) {
    std::string result = "\"";
    for (unsigned char c : value) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else if (c < 0x20) result += "?";
        else result += static_cast<char>(c);
    }
    return result + "\"";
}

std::string integerArray(const std::vector<int>& values) {
    std::string result = "[";
    for (size_t i = 0; i < values.size(); i++) {
        if (i) result += ",";
        result += std::to_string(values[i]);
    }
    return result + "]";
}

std::string spanJson(const SourceSpan& span) {
    return "{\"start\":{\"line\":" + std::to_string(span.start_line) +
        ",\"column\":" + std::to_string(span.start_column) +
        "},\"end\":{\"line\":" + std::to_string(span.end_line) +
        ",\"column\":" + std::to_string(span.end_column) + "}}";
}

std::string astJson(const std::shared_ptr<ASTNode>& node) {
    if (!node) return "null";
    std::string result = "{\"kind\":" +
        quote(astNodeKindName(node->kind)) +
        ",\"name\":" + quote(node->name) +
        ",\"type\":" + quote(node->type_name) +
        ",\"span\":" + spanJson(node->span) +
        ",\"children\":[";
    for (size_t i = 0; i < node->children.size(); i++) {
        if (i) result += ",";
        result += astJson(node->children[i]);
    }
    return result + "]}";
}

} // namespace

std::string semanticSkippedToJson(const std::string& reason) {
    return "{\"ok\":false,\"skipped\":true,\"reason\":" +
           quote(reason) + "}";
}

std::string semanticModelToJson(const SemanticModel& model) {
    if (model.skipped) return semanticSkippedToJson(model.reason);

    std::string result = "{\"ok\":";
    result += model.ok ? "true" : "false";
    result += ",\"skipped\":false,\"ast\":" + astJson(model.ast);

    result += ",\"types\":[";
    for (size_t i = 0; i < model.types.size(); i++) {
        if (i) result += ",";
        const auto& type = model.types[i];
        result += "{\"id\":" + std::to_string(type.id) +
            ",\"name\":" + quote(type.name) +
            ",\"kind\":" + quote(semanticTypeKindName(type.kind)) +
            ",\"parameter_type_ids\":" + integerArray(type.parameter_type_ids) +
            ",\"return_type_id\":" + std::to_string(type.return_type_id) + "}";
    }
    result += "]";

    result += ",\"scopes\":[";
    for (size_t i = 0; i < model.scopes.size(); i++) {
        if (i) result += ",";
        const auto& scope = model.scopes[i];
        result += "{\"id\":" + std::to_string(scope.id) +
            ",\"kind\":" + quote(scopeKindName(scope.kind)) +
            ",\"name\":" + quote(scope.name) +
            ",\"parent_id\":" + std::to_string(scope.parent_id) +
            ",\"owner_symbol_id\":" + std::to_string(scope.owner_symbol_id) +
            ",\"symbol_ids\":" + integerArray(scope.symbol_ids) + "}";
    }
    result += "]";

    result += ",\"symbols\":[";
    for (size_t i = 0; i < model.symbols.size(); i++) {
        if (i) result += ",";
        const auto& symbol = model.symbols[i];
        result += "{\"id\":" + std::to_string(symbol.id) +
            ",\"name\":" + quote(symbol.name) +
            ",\"kind\":" + quote(symbolKindName(symbol.kind)) +
            ",\"type_id\":" + std::to_string(symbol.type_id) +
            ",\"type\":" + quote(symbol.type_name) +
            ",\"scope_id\":" + std::to_string(symbol.scope_id) +
            ",\"span\":" + spanJson(symbol.span) + "}";
    }
    result += "]";

    result += ",\"classes\":[";
    for (size_t i = 0; i < model.classes.size(); i++) {
        if (i) result += ",";
        const auto& cls = model.classes[i];
        result += "{\"id\":" + std::to_string(cls.id) +
            ",\"name\":" + quote(cls.name) +
            ",\"type_id\":" + std::to_string(cls.type_id) +
            ",\"symbol_id\":" + std::to_string(cls.symbol_id) +
            ",\"scope_id\":" + std::to_string(cls.scope_id) +
            ",\"field_symbol_ids\":" + integerArray(cls.field_symbol_ids) +
            ",\"method_symbol_ids\":" + integerArray(cls.method_symbol_ids) + "}";
    }
    result += "]";

    result += ",\"diagnostics\":[";
    for (size_t i = 0; i < model.diagnostics.size(); i++) {
        if (i) result += ",";
        const auto& diagnostic = model.diagnostics[i];
        result += "{\"code\":" + quote(diagnostic.code) +
            ",\"severity\":" + quote(diagnostic.severity) +
            ",\"message\":" + quote(diagnostic.message) +
            ",\"span\":" + spanJson(diagnostic.span) +
            ",\"related_span\":" +
            (diagnostic.has_related_span
                ? spanJson(diagnostic.related_span) : "null") + "}";
    }
    result += "]}";
    return result;
}
