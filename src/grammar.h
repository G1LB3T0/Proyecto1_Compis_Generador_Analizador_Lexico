#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

// ============================================================
//  Production
//  Una producción de la gramática.
//  Ejemplo: expr → expr OP_PLUS term
//
//    head = "expr"
//    body = ["expr", "OP_PLUS", "term"]
//
//  La cadena vacía se representa como body = [""]
// ============================================================
struct Production {
    std::string              head;  // lado izquierdo: "expr"
    std::vector<std::string> body;  // lado derecho:  ["expr", "OP_PLUS", "term"]
};

// ============================================================
//  Grammar
//  Representación completa de la gramática del .yalp.
//
//  Contiene:
//    terminals     ← tokens declarados con %token
//    non_terminals ← nombres de producciones
//    productions   ← todas las producciones en orden
//    start_symbol  ← primera producción definida
//    ignored       ← tokens declarados con IGNORE
// ============================================================
struct Grammar {
    std::vector<std::string>  terminals;      // %token ID, INT_LIT...
    std::vector<std::string>  non_terminals;  // expr, term, factor...
    std::vector<Production>   productions;    // todas las reglas
    std::string               start_symbol;  // primera producción
    std::set<std::string>     ignored;        // IGNORE WS

    // ── Utilidades ──

    // ¿Es un terminal? (mayúscula o $)
    bool isTerminal(const std::string& s) const {
        if (s == "$" || s == "") return true;
        for (const auto& t : terminals)
            if (t == s) return true;
        return false;
    }

    // ¿Es un no terminal?
    bool isNonTerminal(const std::string& s) const {
        for (const auto& nt : non_terminals)
            if (nt == s) return true;
        return false;
    }

    // Imprime la gramática completa — para debug y evaluación
    void print() const {
        std::cout << "\n========== GRAMÁTICA ==========\n";
        std::cout << "Símbolo inicial: " << start_symbol << "\n\n";

        std::cout << "Terminales (" << terminals.size() << "):\n  ";
        for (const auto& t : terminals) std::cout << t << " ";
        std::cout << "\n\n";

        std::cout << "No terminales (" << non_terminals.size() << "):\n  ";
        for (const auto& nt : non_terminals) std::cout << nt << " ";
        std::cout << "\n\n";

        std::cout << "Producciones (" << productions.size() << "):\n";
        for (size_t i = 0; i < productions.size(); i++) {
            std::cout << "  [" << i << "] "
                      << productions[i].head << " → ";
            if (productions[i].body.empty() ||
                productions[i].body[0] == "") {
                std::cout << "ε";
            } else {
                for (const auto& s : productions[i].body)
                    std::cout << s << " ";
            }
            std::cout << "\n";
        }

        if (!ignored.empty()) {
            std::cout << "\nIgnorados: ";
            for (const auto& s : ignored) std::cout << s << " ";
            std::cout << "\n";
        }
        std::cout << "================================\n\n";
    }
};