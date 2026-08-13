#pragma once

#include "parse_tree.h"
#include <memory>
#include <string>
#include <vector>

// El AST elimina signos de puntuación y no terminales auxiliares. Este avance
// conserva únicamente las construcciones necesarias para extraer tipos,
// ámbitos y clases; las expresiones se incorporarán en un avance posterior.
enum class ASTNodeKind {
    PROGRAM,
    CLASS_DECL,
    FIELD_DECL,
    METHOD_DECL,
    PARAMETER_DECL,
    VARIABLE_DECL,
    BLOCK_STMT
};

struct ASTNode {
    ASTNodeKind kind = ASTNodeKind::PROGRAM;
    std::string name;
    std::string type_name;
    SourceSpan span;
    std::vector<std::shared_ptr<ASTNode>> children;
};

const char* astNodeKindName(ASTNodeKind kind);

class ASTBuilder {
public:
    // La gramática semántica documentada usa estos no terminales:
    // class_decl, field_decl, method_decl, parameter, var_decl y block.
    // El constructor recibe el CST, por lo que no depende de SLR o LALR.
    std::shared_ptr<ASTNode> build(
        const std::shared_ptr<ParseTreeNode>& parse_tree) const;

    // Estos constructores se exponen para los recolectores estructurales del
    // archivo de implementación; siguen siendo funciones puras sobre el CST.
    std::shared_ptr<ASTNode> buildClass(const ParseTreeNode& node) const;
    std::shared_ptr<ASTNode> buildField(const ParseTreeNode& node) const;
    std::shared_ptr<ASTNode> buildMethod(const ParseTreeNode& node) const;
    std::shared_ptr<ASTNode> buildParameter(const ParseTreeNode& node) const;
    std::shared_ptr<ASTNode> buildVariable(const ParseTreeNode& node) const;
    std::shared_ptr<ASTNode> buildBlock(const ParseTreeNode& node) const;
};
