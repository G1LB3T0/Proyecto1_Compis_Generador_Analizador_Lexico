#pragma once
#include "regex_node.h"
#include "types.h"
#include <string>

// ─────────────────────────────────────────────
// Generador de archivos DOT (Graphviz)
// Para visualizar el Árbol de Expresión
// ─────────────────────────────────────────────

class DotGenerator {
public:
    // Genera archivo .dot del árbol de expresión
    void generateExprTree(RegexNode* root,
                          const std::string& out_path,
                          const YALexSpec& spec);

    // Genera archivo .dot del AFD
    void generateDFA(const DFA& dfa,
                     const YALexSpec& spec,
                     const std::string& out_path);

private:
    int node_id_counter = 0;

    // Escribe recursivamente los nodos y aristas del árbol
    int writeNode(RegexNode* node, std::string& out);

    // Escapa caracteres especiales para DOT
    std::string escape(const std::string& s);
};