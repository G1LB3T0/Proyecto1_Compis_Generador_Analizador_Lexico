#include "slr_table.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// ── Action::toString ──────────────────────────────────────────

std::string Action::toString() const {
    switch (type) {
        case ActionType::SHIFT:  return "s" + std::to_string(value);
        case ActionType::REDUCE: return "r" + std::to_string(value);
        case ActionType::ACCEPT: return "acc";
        case ActionType::ERROR:  return "";
    }
    return "";
}

// ── Escritura con detección de conflictos ─────────────────────

void SLRTable::setAction(int state, const std::string& sym, Action incoming) {
    Action& cell = action[state][sym];

    if (cell.isEmpty()) {
        cell = incoming;
        return;
    }
    // La celda ya tiene un valor distinto: conflicto
    if (cell.type == incoming.type && cell.value == incoming.value) return;

    ParseConflict c;
    c.state    = state;
    c.symbol   = sym;
    c.existing = cell;
    c.incoming = incoming;

    std::string t1 = (cell.type     == ActionType::SHIFT ? "shift" : "reduce");
    std::string t2 = (incoming.type == ActionType::SHIFT ? "shift" : "reduce");
    c.message  = "[SLR] Conflicto " + t1 + "-" + t2 +
                 " en estado " + std::to_string(state) +
                 " con símbolo '" + sym + "'";
    conflicts.push_back(c);

    // En shift-reduce: preferir shift (convención estándar)
    if (incoming.type == ActionType::SHIFT) cell = incoming;
}

// ── Construcción de la tabla SLR(1) ──────────────────────────

void SLRTable::build(const LR0Automaton& automaton, const FirstFollow& ff) {
    action.clear();
    goto_table.clear();
    conflicts.clear();

    const Grammar& g        = automaton.augmented;
    // S' es siempre la producción 0 tras la aumentación
    const std::string& Sprime = g.productions[0].head;

    for (int i = 0; i < (int)automaton.states.size(); i++) {
        for (const auto& item : automaton.states[i]) {
            const Production& prod = g.productions[item.prod_idx];
            bool epsilonProd = (prod.body.size() == 1 && prod.body[0] == "");
            int  bodyLen     = epsilonProd ? 0 : (int)prod.body.size();
            bool complete    = epsilonProd || item.dot >= bodyLen;

            if (!complete) {
                const std::string& sym = prod.body[item.dot];

                if (g.isTerminal(sym) && sym != "$") {
                    // Regla 1: [A → α • a β]  → ACTION[i, a] = shift GOTO(i, a)
                    auto itT = automaton.transitions.find(i);
                    if (itT == automaton.transitions.end()) continue;
                    auto itS = itT->second.find(sym);
                    if (itS == itT->second.end()) continue;
                    setAction(i, sym, {ActionType::SHIFT, itS->second});
                }

            } else {
                // Ítem completo: punto al final (o producción ε)
                if (prod.head == Sprime) {
                    // Regla 3: [S' → S •]  → ACTION[i, $] = accept
                    setAction(i, "$", {ActionType::ACCEPT, -1});
                } else {
                    // Regla 2: [A → α •]  → ACTION[i, b] = reduce  ∀b ∈ FOLLOW(A)
                    for (const auto& followSym : ff.follow(prod.head))
                        setAction(i, followSym, {ActionType::REDUCE, item.prod_idx});
                }
            }
        }

        // GOTO para no-terminales: directo del autómata LR(0)
        auto itT = automaton.transitions.find(i);
        if (itT != automaton.transitions.end()) {
            for (const auto& [sym, dst] : itT->second)
                if (g.isNonTerminal(sym))
                    goto_table[i][sym] = dst;
        }
    }
}

// ── Print ─────────────────────────────────────────────────────

void SLRTable::print(const Grammar& g) const {
    // Recolectar todas las columnas (terminales + $ + no-terminales)
    std::vector<std::string> terms;
    for (const auto& t : g.terminals) terms.push_back(t);
    terms.push_back("$");

    std::vector<std::string> nonterms;
    for (const auto& nt : g.non_terminals) nonterms.push_back(nt);

    std::cout << "\n========== TABLA SLR(1) ==========\n";

    // Encabezado
    std::cout << std::setw(5) << "Estado";
    for (const auto& t  : terms)    std::cout << std::setw(8) << t;
    for (const auto& nt : nonterms) std::cout << std::setw(8) << nt;
    std::cout << "\n" << std::string(5 + 8*(terms.size()+nonterms.size()), '-') << "\n";

    int n = (int)action.empty() ? 0 :
            (int)std::max_element(action.begin(), action.end(),
                [](const auto& a, const auto& b){ return a.first < b.first; })->first + 1;

    for (int i = 0; i < n; i++) {
        std::cout << std::setw(5) << i;
        for (const auto& t : terms) {
            std::string cell;
            auto itS = action.find(i);
            if (itS != action.end()) {
                auto itA = itS->second.find(t);
                if (itA != itS->second.end()) cell = itA->second.toString();
            }
            std::cout << std::setw(8) << cell;
        }
        for (const auto& nt : nonterms) {
            std::string cell;
            auto itG = goto_table.find(i);
            if (itG != goto_table.end()) {
                auto itN = itG->second.find(nt);
                if (itN != itG->second.end())
                    cell = std::to_string(itN->second);
            }
            std::cout << std::setw(8) << cell;
        }
        std::cout << "\n";
    }
    std::cout << "==================================\n";

    if (!conflicts.empty()) {
        std::cout << "\n[SLR] Conflictos (" << conflicts.size() << "):\n";
        for (const auto& c : conflicts) std::cout << "  " << c.message << "\n";
    }
    std::cout << "\n";
}
