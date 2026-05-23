#include "ll1_table.h"
#include <iostream>
#include <iomanip>

// ── Construcción de la tabla LL(1) ────────────────────────────

void LL1Table::build(const Grammar& g, const FirstFollow& ff) {
    table.clear();
    conflicts.clear();

    // Inicializar todas las celdas en -1 (error)
    for (const auto& nt : g.non_terminals) {
        for (const auto& t : g.terminals) table[nt][t] = -1;
        table[nt]["$"] = -1;
    }

    for (int i = 0; i < (int)g.productions.size(); i++) {
        const Production& prod = g.productions[i];
        const std::string& A   = prod.head;

        // Calcular FIRST(α) donde α es el cuerpo de la producción
        std::set<std::string> firstAlpha = ff.firstOfSequence(prod.body);

        // Regla 1: para cada a ∈ FIRST(α) − {ε}  →  M[A, a] = i
        for (const auto& a : firstAlpha) {
            if (a == "ε") continue;
            int& cell = table[A][a];
            if (cell != -1 && cell != i)
                conflicts.push_back({A, a, cell, i});
            else
                cell = i;
        }

        // Regla 2: si ε ∈ FIRST(α)  →  para cada b ∈ FOLLOW(A): M[A, b] = i
        if (firstAlpha.count("ε")) {
            for (const auto& b : ff.follow(A)) {
                int& cell = table[A][b];
                if (cell != -1 && cell != i)
                    conflicts.push_back({A, b, cell, i});
                else
                    cell = i;
            }
        }
    }
}

// ── Print ─────────────────────────────────────────────────────

void LL1Table::print(const Grammar& g) const {
    std::vector<std::string> terms;
    for (const auto& t : g.terminals) terms.push_back(t);
    terms.push_back("$");

    std::cout << "\n========== TABLA LL(1) ==========\n";

    // Encabezado
    std::cout << std::setw(12) << "NT";
    for (const auto& t : terms) std::cout << std::setw(22) << t;
    std::cout << "\n" << std::string(12 + 22 * terms.size(), '-') << "\n";

    for (const auto& nt : g.non_terminals) {
        std::cout << std::setw(12) << nt;
        auto itNT = table.find(nt);
        for (const auto& t : terms) {
            std::string cell;
            if (itNT != table.end()) {
                auto itT = itNT->second.find(t);
                if (itT != itNT->second.end() && itT->second != -1) {
                    // Mostrar la producción completa en la celda
                    const Production& prod = g.productions[itT->second];
                    cell = prod.head + "→";
                    for (const auto& s : prod.body)
                        cell += (s.empty() ? "ε" : s) + " ";
                }
            }
            std::cout << std::setw(22) << cell;
        }
        std::cout << "\n";
    }
    std::cout << "=================================\n";

    if (!conflicts.empty()) {
        std::cout << "\n[LL(1)] Conflictos (" << conflicts.size() << "):\n";
        for (const auto& c : conflicts) {
            std::cout << "  M[" << c.nonterminal << ", " << c.terminal
                      << "]: producción " << c.existing_prod
                      << " vs " << c.incoming_prod << "\n";
        }
    }
    std::cout << "\n";
}
