#include "regex_node.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

// ─── Fabricas ─────────────────────────────────

RegexNode* makeLeaf(const std::bitset<256>& cs, int& pos_counter,
                    const std::string& lbl) {
    RegexNode* n = new RegexNode();
    n->type    = RegexNode::LEAF;
    n->charset = cs;
    n->position = pos_counter++;
    n->label   = lbl.empty() ? ("p" + std::to_string(n->position)) : lbl;
    return n;
}

RegexNode* makeEndMarker(int rule_idx, int& pos_counter) {
    RegexNode* n = new RegexNode();
    n->type       = RegexNode::END_MARKER;
    n->position   = pos_counter++;
    n->rule_index = rule_idx;
    n->label      = "#" + std::to_string(rule_idx);
    return n;
}

RegexNode* makeConcat(RegexNode* l, RegexNode* r) {
    RegexNode* n = new RegexNode();
    n->type = RegexNode::CONCAT; n->left = l; n->right = r; n->label = "·";
    return n;
}
RegexNode* makeAlter(RegexNode* l, RegexNode* r) {
    RegexNode* n = new RegexNode();
    n->type = RegexNode::ALTER; n->left = l; n->right = r; n->label = "|";
    return n;
}
RegexNode* makeStar(RegexNode* l) {
    RegexNode* n = new RegexNode(); n->type = RegexNode::STAR; n->left = l; n->label = "*"; return n;
}
RegexNode* makePlus(RegexNode* l) {
    RegexNode* n = new RegexNode(); n->type = RegexNode::PLUS; n->left = l; n->label = "+"; return n;
}
RegexNode* makeQuest(RegexNode* l) {
    RegexNode* n = new RegexNode(); n->type = RegexNode::QUEST; n->left = l; n->label = "?"; return n;
}
RegexNode* makeDiff(RegexNode* l, RegexNode* r) {
    RegexNode* n = new RegexNode();
    n->type = RegexNode::DIFF; n->left = l; n->right = r; n->label = "#"; return n;
}

// ─── nullable / firstpos / lastpos ────────────

void computePositions(RegexNode* node) {
    if (!node) return;
    computePositions(node->left);
    computePositions(node->right);

    auto U = [](std::set<int>& d, const std::set<int>& s) { d.insert(s.begin(), s.end()); };

    switch (node->type) {
        case RegexNode::LEAF:
        case RegexNode::END_MARKER:
            node->nullable = false;
            node->firstpos = { node->position };
            node->lastpos  = { node->position };
            break;
        case RegexNode::ALTER:
            node->nullable = node->left->nullable || node->right->nullable;
            node->firstpos = node->left->firstpos; U(node->firstpos, node->right->firstpos);
            node->lastpos  = node->left->lastpos;  U(node->lastpos,  node->right->lastpos);
            break;
        case RegexNode::CONCAT:
            node->nullable = node->left->nullable && node->right->nullable;
            node->firstpos = node->left->firstpos;
            if (node->left->nullable) U(node->firstpos, node->right->firstpos);
            node->lastpos = node->right->lastpos;
            if (node->right->nullable) U(node->lastpos, node->left->lastpos);
            break;
        case RegexNode::STAR:
        case RegexNode::QUEST:
            node->nullable = true;
            node->firstpos = node->left->firstpos;
            node->lastpos  = node->left->lastpos;
            break;
        case RegexNode::PLUS:
            node->nullable = node->left->nullable;
            node->firstpos = node->left->firstpos;
            node->lastpos  = node->left->lastpos;
            break;
        case RegexNode::DIFF:
            node->nullable = node->left->nullable;
            node->firstpos = node->left->firstpos;
            node->lastpos  = node->left->lastpos;
            break;
    }
}

// ─── followpos ────────────────────────────────

void computeFollowpos(RegexNode* node, std::vector<std::set<int>>& followpos) {
    if (!node) return;
    if (node->type == RegexNode::CONCAT)
        for (int i : node->left->lastpos)
            followpos[i].insert(node->right->firstpos.begin(), node->right->firstpos.end());
    else if (node->type == RegexNode::STAR || node->type == RegexNode::PLUS)
        for (int i : node->lastpos)
            followpos[i].insert(node->firstpos.begin(), node->firstpos.end());
    computeFollowpos(node->left,  followpos);
    computeFollowpos(node->right, followpos);
}

// ─── Mapas auxiliares ─────────────────────────

void buildPosCharsetMap(RegexNode* node, std::vector<std::bitset<256>>& pos_charset) {
    if (!node) return;
    if ((node->type == RegexNode::LEAF || node->type == RegexNode::END_MARKER) &&
        node->position >= 0 && node->position < (int)pos_charset.size())
        pos_charset[node->position] = node->charset;
    buildPosCharsetMap(node->left,  pos_charset);
    buildPosCharsetMap(node->right, pos_charset);
}

void buildPosRuleMap(RegexNode* node, std::vector<int>& pos_rule) {
    if (!node) return;
    if (node->type == RegexNode::END_MARKER &&
        node->position >= 0 && node->position < (int)pos_rule.size())
        pos_rule[node->position] = node->rule_index;
    buildPosRuleMap(node->left,  pos_rule);
    buildPosRuleMap(node->right, pos_rule);
}

void freeTree(RegexNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// ─── Impresion: firstpos/lastpos del arbol ────

void printTreePositions(RegexNode* root) {
    if (!root) return;
    std::cout << "[regex_node] computePositions devuelve en la raiz:\n";
    std::cout << "  firstpos = { ";
    for (int p : root->firstpos) std::cout << p << " ";
    std::cout << "}\n";
    std::cout << "  lastpos  = { ";
    for (int p : root->lastpos) std::cout << p << " ";
    std::cout << "}\n";
    std::cout << "  nullable = " << (root->nullable ? "true" : "false") << "\n";
}

// ─── Impresion: tabla de followpos ────────────

void printFollowposTable(const std::vector<std::set<int>>& followpos,
                         const std::vector<int>& pos_rule) {
    std::cout << "[regex_node] computeFollowpos devuelve:\n";
    std::cout << "  pos  tipo           followpos\n";
    std::cout << "  " << std::string(44, '-') << "\n";
    int shown = 0;
    for (int i = 0; i < (int)followpos.size(); i++) {
        if (followpos[i].empty()) continue;
        if (shown++ >= 25) { std::cout << "  ... (" << followpos.size()-25 << " posiciones mas)\n"; break; }
        std::string tipo = (pos_rule[i] >= 0)
            ? "END_MARKER #" + std::to_string(pos_rule[i])
            : "hoja       ";
        std::cout << "  p" << std::setw(3) << i
                  << " " << std::setw(14) << std::left << tipo << " { ";
        for (int p : followpos[i]) std::cout << p << " ";
        std::cout << "}\n";
    }
    std::cout << "\n";
}

void printTreeProperties(RegexNode* node, int depth, int limit, int& count) {
    if (!node || count >= limit) return;
    if (node->type == RegexNode::LEAF || node->type == RegexNode::END_MARKER) {
        std::string indent(depth * 2, ' ');
        std::string tipo = (node->type == RegexNode::END_MARKER) ? "END_MARKER" : "LEAF";
        std::cout << "  " << indent << tipo << " '" << node->label << "'"
                  << " pos=" << node->position
                  << " nullable=" << (node->nullable ? "T" : "F")
                  << " first={" << node->position << "}"
                  << " last={" << node->position << "}\n";
        count++;
    } else {
        std::string indent(depth * 2, ' ');
        std::string tip;
        switch(node->type) {
            case RegexNode::CONCAT: tip = "CONCAT(·)"; break;
            case RegexNode::ALTER:  tip = "ALTER(|)";  break;
            case RegexNode::STAR:   tip = "STAR(*)";   break;
            case RegexNode::PLUS:   tip = "PLUS(+)";   break;
            case RegexNode::QUEST:  tip = "QUEST(?)";  break;
            default:                tip = "OP";         break;
        }
        std::cout << "  " << indent << tip
                  << " nullable=" << (node->nullable ? "T" : "F")
                  << " first={ ";
        for (int p : node->firstpos) std::cout << p << " ";
        std::cout << "} last={ ";
        for (int p : node->lastpos) std::cout << p << " ";
        std::cout << "}\n";
        count++;
    }
    printTreeProperties(node->left,  depth+1, limit, count);
    printTreeProperties(node->right, depth+1, limit, count);
}
