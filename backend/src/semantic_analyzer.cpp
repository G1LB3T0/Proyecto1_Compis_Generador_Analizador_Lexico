#include "semantic_analyzer.h"
#include <algorithm>
#include <sstream>

SemanticModel SemanticAnalyzer::analyze(const std::shared_ptr<ASTNode>& ast) {
    reset(ast);
    if (!ast || ast->kind != ASTNodeKind::PROGRAM) {
        model_.skipped = true;
        model_.reason = "No se recibió un AST de programa válido";
        return model_;
    }

    seedPrimitiveTypes();
    const int global_scope = createScope(ScopeKind::GLOBAL, "<global>", -1);

    // Dos pasadas: las clases se registran primero para admitir declaraciones
    // como `Cuenta cuenta;` aunque Cuenta aparezca después en el archivo.
    predeclareClasses(*ast, global_scope);
    analyzeGlobalVariables(*ast, global_scope);
    analyzeClasses(*ast, global_scope);

    std::sort(model_.diagnostics.begin(), model_.diagnostics.end(),
        [](const SemanticDiagnostic& a, const SemanticDiagnostic& b) {
            if (a.span.start_line != b.span.start_line)
                return a.span.start_line < b.span.start_line;
            if (a.span.start_column != b.span.start_column)
                return a.span.start_column < b.span.start_column;
            return a.code < b.code;
        });
    model_.ok = model_.diagnostics.empty();
    return model_;
}

void SemanticAnalyzer::reset(const std::shared_ptr<ASTNode>& ast) {
    model_ = SemanticModel{};
    model_.ast = ast;
    declared_type_ids_.clear();
    unknown_type_ids_.clear();
    scope_symbols_.clear();
    accepted_class_nodes_.clear();
    class_symbol_ids_.clear();
    class_type_ids_.clear();
}

int SemanticAnalyzer::addType(const std::string& name,
                              SemanticTypeKind kind) {
    SemanticTypeInfo type;
    type.id = static_cast<int>(model_.types.size());
    type.name = name;
    type.kind = kind;
    model_.types.push_back(type);
    return type.id;
}

int SemanticAnalyzer::addMethodType(
    const std::string& qualified_name,
    const std::vector<int>& parameter_types,
    int return_type) {
    SemanticTypeInfo type;
    type.id = static_cast<int>(model_.types.size());
    type.name = qualified_name;
    type.kind = SemanticTypeKind::METHOD;
    type.parameter_type_ids = parameter_types;
    type.return_type_id = return_type;
    model_.types.push_back(type);
    return type.id;
}

int SemanticAnalyzer::createScope(ScopeKind kind,
                                  const std::string& name,
                                  int parent_id,
                                  int owner_symbol_id) {
    ScopeInfo scope;
    scope.id = static_cast<int>(model_.scopes.size());
    scope.kind = kind;
    scope.name = name;
    scope.parent_id = parent_id;
    scope.owner_symbol_id = owner_symbol_id;
    model_.scopes.push_back(scope);
    scope_symbols_[scope.id] = {};
    return scope.id;
}

int SemanticAnalyzer::declareSymbol(
    int scope_id,
    const std::string& name,
    SymbolKind kind,
    int type_id,
    const std::string& type_name,
    const SourceSpan& span,
    const std::string& duplicate_code) {
    auto& local = scope_symbols_[scope_id];
    auto existing = local.find(name);
    if (existing != local.end()) {
        const SourceSpan& previous = model_.symbols[existing->second].span;
        addDiagnostic(duplicate_code,
            "El nombre '" + name + "' ya fue declarado en este ámbito",
            span, &previous);
        return -1;
    }

    SymbolInfo symbol;
    symbol.id = static_cast<int>(model_.symbols.size());
    symbol.name = name;
    symbol.kind = kind;
    symbol.type_id = type_id;
    symbol.type_name = type_name;
    symbol.scope_id = scope_id;
    symbol.span = span;
    model_.symbols.push_back(symbol);

    local[name] = symbol.id;
    model_.scopes[scope_id].symbol_ids.push_back(symbol.id);
    return symbol.id;
}

int SemanticAnalyzer::resolveType(const std::string& name,
                                  const SourceSpan& span) {
    auto declared = declared_type_ids_.find(name);
    if (declared != declared_type_ids_.end()) return declared->second;

    int id;
    auto unknown = unknown_type_ids_.find(name);
    if (unknown == unknown_type_ids_.end()) {
        id = addType(name, SemanticTypeKind::UNKNOWN);
        unknown_type_ids_[name] = id;
    } else {
        id = unknown->second;
    }

    addDiagnostic("SEM003", "El tipo '" + name + "' no está declarado", span);
    return id;
}

void SemanticAnalyzer::addDiagnostic(const std::string& code,
                                     const std::string& message,
                                     const SourceSpan& span,
                                     const SourceSpan* related) {
    SemanticDiagnostic diagnostic;
    diagnostic.code = code;
    diagnostic.message = message;
    diagnostic.span = span;
    if (related) {
        diagnostic.has_related_span = true;
        diagnostic.related_span = *related;
    }
    model_.diagnostics.push_back(diagnostic);
}

void SemanticAnalyzer::seedPrimitiveTypes() {
    static const char* primitives[] = {
        "int", "float", "double", "char", "bool", "string", "void"
    };
    for (const char* name : primitives) {
        const int id = addType(name, SemanticTypeKind::PRIMITIVE);
        declared_type_ids_[name] = id;
    }
}

void SemanticAnalyzer::predeclareClasses(const ASTNode& program,
                                         int global_scope) {
    for (const auto& child : program.children) {
        if (!child || child->kind != ASTNodeKind::CLASS_DECL) continue;

        auto existing = accepted_class_nodes_.find(child->name);
        if (existing != accepted_class_nodes_.end()) {
            addDiagnostic("SEM010",
                "La clase '" + child->name + "' ya fue declarada",
                child->span, &existing->second->span);
            continue;
        }

        const int type_id = addType(child->name, SemanticTypeKind::CLASS);
        const int symbol_id = declareSymbol(global_scope, child->name,
            SymbolKind::CLASS, type_id, child->name, child->span, "SEM010");
        if (symbol_id < 0) continue;

        declared_type_ids_[child->name] = type_id;
        accepted_class_nodes_[child->name] = child.get();
        class_symbol_ids_[child->name] = symbol_id;
        class_type_ids_[child->name] = type_id;
    }
}

void SemanticAnalyzer::analyzeGlobalVariables(const ASTNode& program,
                                              int global_scope) {
    for (const auto& child : program.children) {
        if (!child || child->kind != ASTNodeKind::VARIABLE_DECL) continue;
        const int type_id = resolveType(child->type_name, child->span);
        declareSymbol(global_scope, child->name, SymbolKind::VARIABLE,
                      type_id, child->type_name, child->span);
    }
}

void SemanticAnalyzer::analyzeClasses(const ASTNode& program,
                                      int global_scope) {
    std::set<std::string> analyzed;
    for (const auto& child : program.children) {
        if (!child || child->kind != ASTNodeKind::CLASS_DECL) continue;
        if (!accepted_class_nodes_.count(child->name)) continue;
        if (!analyzed.insert(child->name).second) continue;
        analyzeClass(*child, global_scope);
    }
}

void SemanticAnalyzer::analyzeClass(const ASTNode& class_node,
                                    int global_scope) {
    ClassInfo class_info;
    class_info.id = static_cast<int>(model_.classes.size());
    class_info.name = class_node.name;
    class_info.type_id = class_type_ids_[class_node.name];
    class_info.symbol_id = class_symbol_ids_[class_node.name];
    class_info.scope_id = createScope(ScopeKind::CLASS, class_node.name,
                                      global_scope, class_info.symbol_id);

    for (const auto& member : class_node.children) {
        if (!member) continue;
        if (member->kind == ASTNodeKind::FIELD_DECL) {
            const int type_id = resolveType(member->type_name, member->span);
            const int symbol_id = declareSymbol(class_info.scope_id,
                member->name, SymbolKind::FIELD, type_id,
                member->type_name, member->span);
            if (symbol_id >= 0) class_info.field_symbol_ids.push_back(symbol_id);
        } else if (member->kind == ASTNodeKind::METHOD_DECL) {
            analyzeMethod(*member, class_node.name,
                          class_info.scope_id, class_info);
        }
    }
    model_.classes.push_back(class_info);
}

void SemanticAnalyzer::analyzeMethod(const ASTNode& method_node,
                                     const std::string& class_name,
                                     int class_scope,
                                     ClassInfo& class_info) {
    // Primero se comprueba el nombre: si está duplicado no se crean tipos ni
    // ámbitos huérfanos para una declaración que no formará parte del modelo.
    auto existing = scope_symbols_[class_scope].find(method_node.name);
    if (existing != scope_symbols_[class_scope].end()) {
        const SourceSpan& previous = model_.symbols[existing->second].span;
        addDiagnostic("SEM002",
            "El nombre '" + method_node.name +
            "' ya fue declarado en este ámbito",
            method_node.span, &previous);
        return;
    }

    const int return_type = resolveType(method_node.type_name, method_node.span);
    std::vector<int> parameter_types;
    for (const auto& child : method_node.children) {
        if (child && child->kind == ASTNodeKind::PARAMETER_DECL)
            parameter_types.push_back(resolveType(child->type_name, child->span));
    }

    std::ostringstream signature;
    signature << class_name << "." << method_node.name << "(";
    for (size_t i = 0; i < parameter_types.size(); i++) {
        if (i) signature << ",";
        signature << model_.types[parameter_types[i]].name;
    }
    signature << "):" << method_node.type_name;

    const int method_type = addMethodType(
        signature.str(), parameter_types, return_type);
    const int method_symbol = declareSymbol(class_scope, method_node.name,
        SymbolKind::METHOD, method_type, signature.str(), method_node.span);
    if (method_symbol < 0) return;
    class_info.method_symbol_ids.push_back(method_symbol);

    const int method_scope = createScope(ScopeKind::METHOD,
        class_name + "." + method_node.name, class_scope, method_symbol);

    size_t parameter_index = 0;
    for (const auto& child : method_node.children) {
        if (!child) continue;
        if (child->kind == ASTNodeKind::PARAMETER_DECL) {
            const int type_id = parameter_types[parameter_index++];
            declareSymbol(method_scope, child->name, SymbolKind::PARAMETER,
                          type_id, child->type_name, child->span);
        } else if (child->kind == ASTNodeKind::BLOCK_STMT) {
            analyzeBlock(*child, method_scope, method_symbol);
        }
    }
}

void SemanticAnalyzer::analyzeBlock(const ASTNode& block_node,
                                    int parent_scope,
                                    int owner_symbol_id) {
    std::ostringstream name;
    name << "block@" << block_node.span.start_line
         << ":" << block_node.span.start_column;
    const int block_scope = createScope(ScopeKind::BLOCK, name.str(),
                                        parent_scope, owner_symbol_id);

    for (const auto& child : block_node.children) {
        if (!child) continue;
        if (child->kind == ASTNodeKind::VARIABLE_DECL) {
            const int type_id = resolveType(child->type_name, child->span);
            declareSymbol(block_scope, child->name, SymbolKind::VARIABLE,
                          type_id, child->type_name, child->span);
        } else if (child->kind == ASTNodeKind::BLOCK_STMT) {
            analyzeBlock(*child, block_scope, owner_symbol_id);
        }
    }
}
