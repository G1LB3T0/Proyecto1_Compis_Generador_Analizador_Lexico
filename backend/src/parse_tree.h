#pragma once

#include "source_span.h"
#include <memory>
#include <string>
#include <vector>

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
