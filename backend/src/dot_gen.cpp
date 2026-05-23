#include "dot_gen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>

// ─── Escape de caracteres especiales DOT ──────

std::string DotGenerator::escape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c == '\t') r += "\\t";
        else if (c == '<')  r += "\\<";
        else if (c == '>')  r += "\\>";
        else if (c == '{')  r += "\\{";
        else if (c == '}')  r += "\\}";
        else if (c == '|')  r += "\\|";
        else r += c;
    }
    return r;
}

// ─── Escritura recursiva de nodos ─────────────

int DotGenerator::writeNode(RegexNode* node, std::string& out) {
    if (!node) return -1;

    int myid = node_id_counter++;

    // Determinar color y forma según tipo
    std::string color = "white";
    std::string shape = "ellipse";
    std::string label;

    switch (node->type) {
        case RegexNode::LEAF: {
            shape = "box";
            color = "#cce5ff";
            // Mostrar caracteres representativos
            std::string chars;
            int cnt = 0;
            for (int i = 0; i < 256 && cnt < 6; i++) {
                if (node->charset.test(i)) {
                    if (i == '\n') chars += "\\\\n";
                    else if (i == '\t') chars += "\\\\t";
                    else if (i == '\r') chars += "\\\\r";
                    else if (i >= 32 && i < 127) chars += (char)i;
                    cnt++;
                }
            }
            if (node->charset.count() > 6) chars += "...";
            label = "p" + std::to_string(node->position) + "\\n[" + escape(chars) + "]";
            break;
        }
        case RegexNode::END_MARKER:
            shape = "diamond";
            color = "#d4edda";
            label = "#" + std::to_string(node->rule_index) +
                    "\\np" + std::to_string(node->position);
            break;
        case RegexNode::CONCAT:
            color = "#fff3cd";
            label = "·";
            break;
        case RegexNode::ALTER:
            color = "#f8d7da";
            label = "|";
            break;
        case RegexNode::STAR:
            color = "#e2d9f3";
            label = "*";
            break;
        case RegexNode::PLUS:
            color = "#e2d9f3";
            label = "+";
            break;
        case RegexNode::QUEST:
            color = "#e2d9f3";
            label = "?";
            break;
        case RegexNode::DIFF:
            color = "#ffeeba";
            label = "#";
            break;
    }

    // Añadir info de firstpos si está calculado
    if (!node->firstpos.empty()) {
        label += "\\nfirst={";
        bool first = true;
        for (int p : node->firstpos) {
            if (!first) label += ",";
            label += std::to_string(p);
            first = false;
        }
        label += "}";
    }

    out += "  n" + std::to_string(myid) +
           " [label=\"" + label + "\", shape=" + shape +
           ", style=filled, fillcolor=\"" + color + "\"];\n";

    // Hijos
    int lid = writeNode(node->left,  out);
    int rid = writeNode(node->right, out);

    if (lid >= 0)
        out += "  n" + std::to_string(myid) + " -> n" + std::to_string(lid) + ";\n";
    if (rid >= 0)
        out += "  n" + std::to_string(myid) + " -> n" + std::to_string(rid) + ";\n";

    return myid;
}

// ─── Árbol de expresión ───────────────────────

void DotGenerator::generateExprTree(RegexNode* root,
                                     const std::string& out_path,
                                     const YALexSpec& spec) {
    node_id_counter = 0;
    std::string body;

    writeNode(root, body);

    std::string dot =
        "digraph ExpressionTree {\n"
        "  graph [rankdir=TB, label=\"Árbol de Expresión YALex\\n(" +
        escape(spec.entrypoint) + ")\", fontsize=14];\n"
        "  node [fontname=\"Helvetica\"];\n" +
        body +
        "}\n";

    std::ofstream f(out_path);
    if (!f.is_open())
        throw std::runtime_error("[DOT] No se puede crear: " + out_path);
    f << dot;
    std::cout << "[dot_gen] generateExprTree() -> " << out_path << "\n";
    std::cout << "  (arbol con " << spec.rules.size() << " reglas, visualizar con: dot -Tpng " << out_path << " -o arbol.png)\n";
}

// ─── AFD ─────────────────────────────────────

void DotGenerator::generateDFA(const DFA& dfa,
                                 const YALexSpec& spec,
                                 const std::string& out_path) {
    std::string dot;
    dot += "digraph DFA {\n";
    dot += "  rankdir=LR;\n";
    dot += "  graph [label=\"AFD Minimizado\\n(" +
            escape(spec.entrypoint) + ")\", fontsize=14];\n";
    dot += "  node [fontname=\"Helvetica\"];\n";
    dot += "  __start [shape=point];\n";

    // Nodos
    for (const auto& st : dfa.states) {
        std::string shape = st.is_accepting ? "doublecircle" : "circle";
        std::string color = "white";
        std::string lbl = "S" + std::to_string(st.id);
        if (st.is_accepting && st.accepting_rule >= 0 &&
            st.accepting_rule < (int)spec.rules.size()) {
            lbl += "\\n[R" + std::to_string(st.accepting_rule) + "]";
            color = "#d4edda";
        }
        if (st.id == dfa.start_state) color = "#cce5ff";

        dot += "  S" + std::to_string(st.id) +
               " [shape=" + shape +
               ", style=filled, fillcolor=\"" + color +
               "\", label=\"" + lbl + "\"];\n";
    }

    // Arista de entrada
    dot += "  __start -> S" + std::to_string(dfa.start_state) + ";\n";

    // Transiciones (agrupar caracteres que van al mismo destino)
    for (const auto& st : dfa.states) {
        // Agrupar: destino → conjunto de chars
        std::map<int, std::set<int>> dest_chars;
        for (int a = 0; a < 256; a++) {
            int t = st.transitions[a];
            if (t != -1) dest_chars[t].insert(a);
        }

        for (const auto& [dest, chars] : dest_chars) {
            // Construir etiqueta condensada
            std::string lbl;
            int prev = -2;
            int range_start = -1;

            auto flush = [&](int last) {
                if (range_start < 0) return;
                if (lbl.size() > 20) { lbl = "..."; return; }
                if (last - range_start > 1) {
                    char a = (char)range_start, b = (char)last;
                    if (a >= 32 && a < 127 && b >= 32 && b < 127)
                        lbl += std::string(1,a) + "-" + std::string(1,b);
                    else
                        lbl += "[" + std::to_string(range_start) + "-" + std::to_string(last) + "]";
                } else {
                    for (int i = range_start; i <= last; i++) {
                        char c = (char)i;
                        if (c >= 32 && c < 127 && c != '"' && c != '\\' && c != '|')
                            lbl += c;
                        else
                            lbl += "\\\\x" + std::to_string(i);
                    }
                }
            };

            for (int c : chars) {
                if (c != prev + 1) {
                    flush(prev);
                    range_start = c;
                }
                prev = c;
            }
            flush(prev);
            if (lbl.empty() || lbl == "...") lbl = "...";

            dot += "  S" + std::to_string(st.id) +
                   " -> S" + std::to_string(dest) +
                   " [label=\"" + escape(lbl) + "\"];\n";
        }
    }

    dot += "}\n";

    std::ofstream f(out_path);
    if (!f.is_open())
        throw std::runtime_error("[DOT] No se puede crear: " + out_path);
    f << dot;
    std::cout << "[dot_gen] generateDFA() -> " << out_path << "\n";
    std::cout << "  (" << dfa.states.size() << " estados, visualizar con: dot -Tpng " << out_path << " -o dfa.png)\n\n";
}