#include "lr0_automaton.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ── Helpers internos ──────────────────────────────────────────

// Una producción es ε si su único símbolo en el cuerpo es ""
static bool isEpsilonProd(const Production& p) {
    return p.body.size() == 1 && p.body[0] == "";
}

bool LR0Automaton::isComplete(const LR0Item& item) const {
    const Production& prod = augmented.productions[item.prod_idx];
    // Producción ε: el punto siempre está al final
    if (isEpsilonProd(prod)) return true;
    return item.dot >= (int)prod.body.size();
}

const std::string& LR0Automaton::symbolAtDot(const LR0Item& item) const {
    return augmented.productions[item.prod_idx].body[item.dot];
}

// ── Aumentar la gramática ─────────────────────────────────────

Grammar LR0Automaton::augmentGrammar(const Grammar& g) const {
    Grammar aug = g;

    // Nuevo símbolo inicial S' que no colisione con ningún símbolo existente
    std::string newStart = g.start_symbol + "'";
    while (aug.isNonTerminal(newStart) || aug.isTerminal(newStart))
        newStart += "'";

    // S' → start_symbol se inserta al INICIO para que sea la producción 0
    Production startProd;
    startProd.head = newStart;
    startProd.body = { g.start_symbol };
    aug.productions.insert(aug.productions.begin(), startProd);
    aug.non_terminals.insert(aug.non_terminals.begin(), newStart);
    aug.start_symbol = newStart;
    return aug;
}

// ── Closure ───────────────────────────────────────────────────

ItemSet LR0Automaton::closure(const ItemSet& items) const {
    ItemSet result = items;
    bool changed = true;

    while (changed) {
        changed = false;
        // Iterar sobre copia para no invalidar iteradores al insertar
        ItemSet current = result;
        for (const auto& item : current) {
            if (isComplete(item)) continue;  // punto al final: nada que expandir
            const std::string& B = symbolAtDot(item);
            if (!augmented.isNonTerminal(B)) continue;  // solo expandir no-terminales

            // Agregar [B → • γ] para cada producción de B
            for (int i = 0; i < (int)augmented.productions.size(); i++) {
                if (augmented.productions[i].head != B) continue;
                LR0Item newItem{i, 0};
                if (!result.count(newItem)) {
                    result.insert(newItem);
                    changed = true;
                }
            }
        }
    }
    return result;
}

// ── GOTO ──────────────────────────────────────────────────────

ItemSet LR0Automaton::gotoSet(const ItemSet& items, const std::string& sym) const {
    ItemSet kernel;
    for (const auto& item : items) {
        if (isComplete(item)) continue;
        if (symbolAtDot(item) == sym)
            // Mover el punto un paso a la derecha
            kernel.insert({item.prod_idx, item.dot + 1});
    }
    if (kernel.empty()) return {};
    return closure(kernel);
}

// ── Construcción de la colección canónica ─────────────────────

void LR0Automaton::build(const Grammar& g) {
    augmented = augmentGrammar(g);
    states.clear();
    transitions.clear();

    // Estado inicial: closure({[S' → • start_symbol]})
    // La producción S' → start_symbol siempre queda en el índice 0
    ItemSet init;
    init.insert({0, 0});
    states.push_back(closure(init));

    // Recolectar todos los símbolos de la gramática aumentada
    std::vector<std::string> symbols;
    for (const auto& t  : augmented.terminals)     symbols.push_back(t);
    for (const auto& nt : augmented.non_terminals)  symbols.push_back(nt);

    // Procesar estados a medida que se descubren (el vector crece durante el loop)
    for (int i = 0; i < (int)states.size(); i++) {
        for (const auto& sym : symbols) {
            ItemSet next = gotoSet(states[i], sym);
            if (next.empty()) continue;

            // Buscar si el estado ya existe
            int nextIdx = -1;
            for (int j = 0; j < (int)states.size(); j++) {
                if (states[j] == next) { nextIdx = j; break; }
            }
            if (nextIdx == -1) {
                // Estado nuevo: agregarlo al vector
                nextIdx = (int)states.size();
                states.push_back(next);
            }
            transitions[i][sym] = nextIdx;
        }
    }
}

// ── Print ─────────────────────────────────────────────────────

void LR0Automaton::print() const {
    std::cout << "\n========== AUTÓMATA LR(0) ==========\n";
    std::cout << "Estados: " << states.size() << "\n\n";

    for (int i = 0; i < (int)states.size(); i++) {
        std::cout << "I" << i << ":\n";
        for (const auto& item : states[i]) {
            const Production& prod = augmented.productions[item.prod_idx];
            std::cout << "  [" << prod.head << " → ";

            if (isEpsilonProd(prod)) {
                // Producción ε: el punto siempre al final
                std::cout << "• ε";
            } else {
                for (int k = 0; k < (int)prod.body.size(); k++) {
                    if (k == item.dot) std::cout << "• ";
                    std::cout << prod.body[k];
                    if (k + 1 < (int)prod.body.size()) std::cout << " ";
                }
                if (item.dot == (int)prod.body.size()) std::cout << " •";
            }
            std::cout << "]\n";
        }

        // Transiciones desde este estado
        auto it = transitions.find(i);
        if (it != transitions.end()) {
            for (const auto& [sym, dst] : it->second)
                std::cout << "  GOTO(" << sym << ") = I" << dst << "\n";
        }
        std::cout << "\n";
    }
    std::cout << "=====================================\n\n";
}
