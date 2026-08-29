#include "semantic_model.h"

const char* semanticTypeKindName(SemanticTypeKind kind) {
    switch (kind) {
        case SemanticTypeKind::PRIMITIVE: return "primitive";
        case SemanticTypeKind::CLASS:     return "class";
        case SemanticTypeKind::METHOD:    return "method";
        case SemanticTypeKind::UNKNOWN:   return "unknown";
    }
    return "unknown";
}
const char* scopeKindName(ScopeKind kind) {
    switch (kind) {
        case ScopeKind::GLOBAL: return "global";
        case ScopeKind::CLASS:  return "class";
        case ScopeKind::METHOD: return "method";
        case ScopeKind::BLOCK:  return "block";
    }
    return "unknown";
}

const char* symbolKindName(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::CLASS:     return "class";
        case SymbolKind::FIELD:     return "field";
        case SymbolKind::METHOD:    return "method";
        case SymbolKind::PARAMETER: return "parameter";
        case SymbolKind::VARIABLE:  return "variable";
    }
    return "unknown";
}
