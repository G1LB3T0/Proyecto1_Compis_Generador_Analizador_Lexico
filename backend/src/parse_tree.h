#pragma once
#include "source_span.h"
#include <string>
#include <vector>
#include <memory>

// Árbol ligero usado por el parser LL(1) y la visualización histórica.
struct TreeNode {
    std::string label;
    std::string lexema;
    std::vector<std::shared_ptr<TreeNode>> children;

    TreeNode() = default;
    TreeNode(const std::string& lbl, const std::string& lex = "")
        : label(lbl), lexema(lex) {}
};

using TreeNodePtr = std::shared_ptr<TreeNode>;

// Árbol sintáctico concreto independiente del método LR utilizado.
// Un nodo terminal conserva el token completo; un nodo no terminal conserva
// la producción y sus hijos en el orden original de la gramática.
struct ParseTreeNode {
    std::string symbol;
    bool terminal = false;
    int production_index = -1;

    std::string lexeme;
    SourceSpan span;
    std::vector<std::shared_ptr<ParseTreeNode>> children;
};

std::shared_ptr<ParseTreeNode> makeTerminalNode(const Token& token);

std::shared_ptr<ParseTreeNode> makeNonTerminalNode(
    const std::string& symbol,
    int production_index,
    const std::vector<std::shared_ptr<ParseTreeNode>>& children);
