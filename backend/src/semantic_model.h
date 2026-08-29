#pragma once

#include "ast.h"
#include <memory>
#include <string>
#include <vector>

enum class SemanticTypeKind { PRIMITIVE, CLASS, METHOD, UNKNOWN };
enum class ScopeKind { GLOBAL, CLASS, METHOD, BLOCK };
enum class SymbolKind { CLASS, FIELD, METHOD, PARAMETER, VARIABLE };

struct SemanticTypeInfo {
    int id = -1;
    std::string name;
    SemanticTypeKind kind = SemanticTypeKind::UNKNOWN;
    std::vector<int> parameter_type_ids;
    int return_type_id = -1;
};

struct ScopeInfo {
    int id = -1;
    ScopeKind kind = ScopeKind::GLOBAL;
    std::string name;
    int parent_id = -1;
    int owner_symbol_id = -1;
    std::vector<int> symbol_ids;
};

struct SymbolInfo {
    int id = -1;
    std::string name;
    SymbolKind kind = SymbolKind::VARIABLE;
    int type_id = -1;
    std::string type_name;
    int scope_id = -1;
    SourceSpan span;
};

struct ClassInfo {
    int id = -1;
    std::string name;
    int type_id = -1;
    int symbol_id = -1;
    int scope_id = -1;
    std::vector<int> field_symbol_ids;
    std::vector<int> method_symbol_ids;
};

struct SemanticDiagnostic {
    std::string code;
    std::string severity = "error";
    std::string message;
    SourceSpan span;
    bool has_related_span = false;
    SourceSpan related_span;
};

// SemanticModel es una instantánea autocontenida del avance. Sus relaciones
// se expresan mediante IDs estables para que el JSON no dependa de punteros.
struct SemanticModel {
    bool ok = false;
    bool skipped = false;
    std::string reason;
    std::shared_ptr<ASTNode> ast;
    std::vector<SemanticTypeInfo> types;
    std::vector<ScopeInfo> scopes;
    std::vector<SymbolInfo> symbols;
    std::vector<ClassInfo> classes;
    std::vector<SemanticDiagnostic> diagnostics;
};

const char* semanticTypeKindName(SemanticTypeKind kind);
const char* scopeKindName(ScopeKind kind);
const char* symbolKindName(SymbolKind kind);
