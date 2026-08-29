#include "ast.h"

namespace {

const ParseTreeNode* directChild(const ParseTreeNode& node,
                                 const std::string& symbol) {
    for (const auto& child : node.children)
        if (child && child->symbol == symbol) return child.get();
    return nullptr;
}

const ParseTreeNode* firstTerminal(const ParseTreeNode& node) {
    if (node.terminal) return &node;
    for (const auto& child : node.children) {
        if (!child) continue;
        const ParseTreeNode* result = firstTerminal(*child);
        if (result) return result;
    }
    return nullptr;
}

std::string declaredType(const ParseTreeNode& node) {
    const ParseTreeNode* type = directChild(node, "type");
    if (!type) return "<unknown>";
    const ParseTreeNode* token = firstTerminal(*type);
    return token ? token->lexeme : "<unknown>";
}

std::string directIdentifier(const ParseTreeNode& node) {
    const ParseTreeNode* id = directChild(node, "ID");
    return id ? id->lexeme : "<missing>";
}

void collectTopLevel(const ParseTreeNode& node,
                     const ASTBuilder& builder,
                     std::vector<std::shared_ptr<ASTNode>>& output);
void collectClassMembers(const ParseTreeNode& node,
                         const ASTBuilder& builder,
                         std::vector<std::shared_ptr<ASTNode>>& output);
void collectParameters(const ParseTreeNode& node,
                       const ASTBuilder& builder,
                       std::vector<std::shared_ptr<ASTNode>>& output);
void collectBlockItems(const ParseTreeNode& node,
                       const ASTBuilder& builder,
                       std::vector<std::shared_ptr<ASTNode>>& output);

} // namespace
const char* astNodeKindName(ASTNodeKind kind) {
    switch (kind) {
        case ASTNodeKind::PROGRAM:        return "program";
        case ASTNodeKind::CLASS_DECL:     return "class";
        case ASTNodeKind::FIELD_DECL:     return "field";
        case ASTNodeKind::METHOD_DECL:    return "method";
        case ASTNodeKind::PARAMETER_DECL: return "parameter";
        case ASTNodeKind::VARIABLE_DECL:  return "variable";
        case ASTNodeKind::BLOCK_STMT:     return "block";
    }
    return "unknown";
}

std::shared_ptr<ASTNode> ASTBuilder::build(
    const std::shared_ptr<ParseTreeNode>& parse_tree) const {
    if (!parse_tree) return nullptr;

    auto program = std::make_shared<ASTNode>();
    program->kind = ASTNodeKind::PROGRAM;
    program->name = "<global>";
    program->span = parse_tree->span;
    collectTopLevel(*parse_tree, *this, program->children);
    return program;
}

std::shared_ptr<ASTNode> ASTBuilder::buildClass(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::CLASS_DECL;
    result->name = directIdentifier(node);
    result->type_name = result->name;
    result->span = node.span;
    collectClassMembers(node, *this, result->children);
    return result;
}

std::shared_ptr<ASTNode> ASTBuilder::buildField(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::FIELD_DECL;
    result->name = directIdentifier(node);
    result->type_name = declaredType(node);
    result->span = node.span;
    return result;
}

std::shared_ptr<ASTNode> ASTBuilder::buildMethod(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::METHOD_DECL;
    result->name = directIdentifier(node);
    result->type_name = declaredType(node);
    result->span = node.span;

    for (const auto& child : node.children) {
        if (!child) continue;
        if (child->symbol == "block")
            result->children.push_back(buildBlock(*child));
        else
            collectParameters(*child, *this, result->children);
    }
    return result;
}

std::shared_ptr<ASTNode> ASTBuilder::buildParameter(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::PARAMETER_DECL;
    result->name = directIdentifier(node);
    result->type_name = declaredType(node);
    result->span = node.span;
    return result;
}

std::shared_ptr<ASTNode> ASTBuilder::buildVariable(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::VARIABLE_DECL;
    result->name = directIdentifier(node);
    result->type_name = declaredType(node);
    result->span = node.span;
    return result;
}

std::shared_ptr<ASTNode> ASTBuilder::buildBlock(
    const ParseTreeNode& node) const {
    auto result = std::make_shared<ASTNode>();
    result->kind = ASTNodeKind::BLOCK_STMT;
    result->name = "block";
    result->span = node.span;
    for (const auto& child : node.children)
        if (child) collectBlockItems(*child, *this, result->children);
    return result;
}

namespace {

// Los recolectores detienen la recursión al reconocer una construcción. Así,
// una variable local de un método nunca se confunde con un atributo de clase.
void collectTopLevel(const ParseTreeNode& node,
                     const ASTBuilder& builder,
                     std::vector<std::shared_ptr<ASTNode>>& output) {
    if (node.symbol == "class_decl") {
        output.push_back(builder.buildClass(node));
        return;
    }
    if (node.symbol == "global_var_decl") {
        output.push_back(builder.buildVariable(node));
        return;
    }
    for (const auto& child : node.children)
        if (child) collectTopLevel(*child, builder, output);
}

void collectClassMembers(const ParseTreeNode& node,
                         const ASTBuilder& builder,
                         std::vector<std::shared_ptr<ASTNode>>& output) {
    if (node.symbol == "field_decl") {
        output.push_back(builder.buildField(node));
        return;
    }
    if (node.symbol == "method_decl") {
        output.push_back(builder.buildMethod(node));
        return;
    }
    for (const auto& child : node.children)
        if (child) collectClassMembers(*child, builder, output);
}

void collectParameters(const ParseTreeNode& node,
                       const ASTBuilder& builder,
                       std::vector<std::shared_ptr<ASTNode>>& output) {
    if (node.symbol == "block") return;
    if (node.symbol == "parameter") {
        output.push_back(builder.buildParameter(node));
        return;
    }
    for (const auto& child : node.children)
        if (child) collectParameters(*child, builder, output);
}

void collectBlockItems(const ParseTreeNode& node,
                       const ASTBuilder& builder,
                       std::vector<std::shared_ptr<ASTNode>>& output) {
    if (node.symbol == "var_decl") {
        output.push_back(builder.buildVariable(node));
        return;
    }
    if (node.symbol == "block") {
        output.push_back(builder.buildBlock(node));
        return;
    }
    for (const auto& child : node.children)
        if (child) collectBlockItems(*child, builder, output);
}

} // namespace
