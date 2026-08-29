#pragma once
#include <string>
#include <vector>
#include <memory>

struct TreeNode {
    std::string label;
    std::string lexema;
    std::vector<std::shared_ptr<TreeNode>> children;

    TreeNode() = default;
    TreeNode(const std::string& lbl, const std::string& lex = "")
        : label(lbl), lexema(lex) {}
};

using TreeNodePtr = std::shared_ptr<TreeNode>;
