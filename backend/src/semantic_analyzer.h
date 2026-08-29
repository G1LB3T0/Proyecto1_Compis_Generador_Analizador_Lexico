#pragma once

#include "semantic_model.h"
#include <map>
#include <set>

// Primera versión del analizador semántico.
// Responsabilidades incluidas: catálogo de tipos, árbol de ámbitos, tabla de
// símbolos, clases y diagnósticos por duplicados/tipos desconocidos.
class SemanticAnalyzer {
public:
    SemanticModel analyze(const std::shared_ptr<ASTNode>& ast);

private:
    SemanticModel model_;
    std::map<std::string, int> declared_type_ids_;
    std::map<std::string, int> unknown_type_ids_;
    std::map<int, std::map<std::string, int>> scope_symbols_;
    std::map<std::string, const ASTNode*> accepted_class_nodes_;
    std::map<std::string, int> class_symbol_ids_;
    std::map<std::string, int> class_type_ids_;

    void reset(const std::shared_ptr<ASTNode>& ast);
    int addType(const std::string& name, SemanticTypeKind kind);
    int addMethodType(const std::string& qualified_name,
                      const std::vector<int>& parameter_types,
                      int return_type);
    int createScope(ScopeKind kind, const std::string& name,
                    int parent_id, int owner_symbol_id = -1);
    int declareSymbol(int scope_id, const std::string& name,
                      SymbolKind kind, int type_id,
                      const std::string& type_name,
                      const SourceSpan& span,
                      const std::string& duplicate_code = "SEM002");
    int resolveType(const std::string& name, const SourceSpan& span);
    void addDiagnostic(const std::string& code,
                       const std::string& message,
                       const SourceSpan& span,
                       const SourceSpan* related = nullptr);

    void seedPrimitiveTypes();
    void predeclareClasses(const ASTNode& program, int global_scope);
    void analyzeGlobalVariables(const ASTNode& program, int global_scope);
    void analyzeClasses(const ASTNode& program, int global_scope);
    void analyzeClass(const ASTNode& class_node, int global_scope);
    void analyzeMethod(const ASTNode& method_node,
                       const std::string& class_name,
                       int class_scope,
                       ClassInfo& class_info);
    void analyzeBlock(const ASTNode& block_node, int parent_scope,
                      int owner_symbol_id);
};
