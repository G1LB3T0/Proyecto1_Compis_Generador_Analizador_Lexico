#include "parse_tree.h"

std::shared_ptr<ParseTreeNode> makeTerminalNode(const Token& token) {
    auto node = std::make_shared<ParseTreeNode>();
    node->symbol = token.tipo;
    node->terminal = true;
    node->lexeme = token.lexema;
    node->span = SourceSpan::fromToken(token);
    return node;
}
std::shared_ptr<ParseTreeNode> makeNonTerminalNode(
    const std::string& symbol,
    int production_index,
    const std::vector<std::shared_ptr<ParseTreeNode>>& children) {
    auto node = std::make_shared<ParseTreeNode>();
    node->symbol = symbol;
    node->production_index = production_index;
    node->children = children;

    const ParseTreeNode* first = nullptr;
    const ParseTreeNode* last = nullptr;
    for (const auto& child : children) {
        if (!child || !child->span.valid()) continue;
        if (!first) first = child.get();
        last = child.get();
    }
    if (first && last)
        node->span = SourceSpan::covering(first->span, last->span);

    return node;
}
