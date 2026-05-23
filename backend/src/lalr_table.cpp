#include "lalr_table.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// ── Helper interno ────────────────────────────────────────────

static bool isEpsilonProd1(const Production& p) {
    return p.body.size() == 1 && p.body[0] == "";
}

bool LALRTable::isComplete(const LR1Item& item, const Grammar& g) const {
    const Production& prod = g.productions[item.prod_idx];
    if (isEpsilonProd1(prod)) return true;
    return item.dot >= (int)prod.body.size();
}

// ── Núcleo de un conjunto LR(1) ───────────────────────────────

std::set<std::pair<int,int>> LALRTable::core(const LR1ItemSet& items) const {
    // El núcleo es el conjunto de (prod_idx, dot) sin lookaheads
    std::set<std::pair<int,int>> c;
    for (const auto& item : items)
        c.insert({item.prod_idx, item.dot});
    return c;
}

// ── Cierre LR(1) ─────────────────────────────────────────────

LR1ItemSet LALRTable::closure1(const LR1ItemSet& items,
                               const Grammar& g,
                               const FirstFollow& ff) const {
    LR1ItemSet result = items;
    bool changed = true;

    while (changed) {
        changed = false;
        LR1ItemSet current = result;
        for (const auto& item : current) {
            if (isComplete(item, g)) continue;
            const Production& prod = g.productions[item.prod_idx];
            const std::string& B = prod.body[item.dot];
            if (!g.isNonTerminal(B)) continue;

            // Construir la secuencia β a (lo que sigue al punto, más el lookahead)
            std::vector<std::string> betaA;
            for (int k = item.dot + 1; k < (int)prod.body.size(); k++)
                betaA.push_back(prod.body[k]);
            betaA.push_back(item.lookahead);

            // FIRST(β a): los lookaheads para los nuevos ítems de B
            std::set<std::string> firstBetaA = ff.firstOfSequence(betaA);
            firstBetaA.erase("ε");  // los lookaheads nunca son ε

            // Agregar [B → • γ, b] para cada producción de B y cada b en FIRST(βa)
            for (int i = 0; i < (int)g.productions.size(); i++) {
                if (g.productions[i].head != B) continue;
                for (const auto& b : firstBetaA) {
                    LR1Item newItem{i, 0, b};
                    if (!result.count(newItem)) {
                        result.insert(newItem);
                        changed = true;
                    }
                }
            }
        }
    }
    return result;
}

// ── GOTO LR(1) ────────────────────────────────────────────────

LR1ItemSet LALRTable::goto1(const LR1ItemSet& items,
                            const std::string& sym,
                            const Grammar& g,
                            const FirstFollow& ff) const {
    LR1ItemSet kernel;
    for (const auto& item : items) {
        if (isComplete(item, g)) continue;
        const Production& prod = g.productions[item.prod_idx];
        if (prod.body[item.dot] == sym)
            kernel.insert({item.prod_idx, item.dot + 1, item.lookahead});
    }
    if (kernel.empty()) return {};
    return closure1(kernel, g, ff);
}

// ── Escritura con detección de conflictos ─────────────────────

void LALRTable::setAction(int state, const std::string& sym, Action incoming) {
    Action& cell = action[state][sym];
    if (cell.isEmpty()) { cell = incoming; return; }
    if (cell.type == incoming.type && cell.value == incoming.value) return;

    ParseConflict c;
    c.state    = state;
    c.symbol   = sym;
    c.existing = cell;
    c.incoming = incoming;
    std::string t1 = (cell.type     == ActionType::SHIFT ? "shift" : "reduce");
    std::string t2 = (incoming.type == ActionType::SHIFT ? "shift" : "reduce");
    c.message  = "[LALR] Conflicto " + t1 + "-" + t2 +
                 " en estado " + std::to_string(state) +
                 " con símbolo '" + sym + "'";
    conflicts.push_back(c);
    if (incoming.type == ActionType::SHIFT) cell = incoming;
}

// ── Construcción de la tabla LALR ─────────────────────────────

void LALRTable::build(const LR0Automaton& lr0, const FirstFollow& ff) {
    action.clear();
    goto_table.clear();
    conflicts.clear();

    const Grammar& g        = lr0.augmented;
    const std::string& Sprime = g.productions[0].head;

    // Recolectar todos los símbolos
    std::vector<std::string> symbols;
    for (const auto& t  : g.terminals)     symbols.push_back(t);
    for (const auto& nt : g.non_terminals)  symbols.push_back(nt);

    // ── Paso 1: construir todos los estados LR(1) ────────────
    // Estado inicial: closure1({[S' → • S, $]})
    LR1ItemSet init;
    init.insert({0, 0, "$"});
    std::vector<LR1ItemSet> lr1States;
    lr1States.push_back(closure1(init, g, ff));

    // Mapa de transiciones LR(1): lr1Trans[estado][símbolo] = estado
    std::map<int, std::map<std::string, int>> lr1Trans;

    for (int i = 0; i < (int)lr1States.size(); i++) {
        for (const auto& sym : symbols) {
            LR1ItemSet next = goto1(lr1States[i], sym, g, ff);
            if (next.empty()) continue;

            // Buscar si el estado LR(1) ya existe
            int nextIdx = -1;
            for (int j = 0; j < (int)lr1States.size(); j++) {
                if (lr1States[j] == next) { nextIdx = j; break; }
            }
            if (nextIdx == -1) {
                nextIdx = (int)lr1States.size();
                lr1States.push_back(next);
            }
            lr1Trans[i][sym] = nextIdx;
        }
    }

    // ── Paso 2: agrupar estados LR(1) por núcleo LR(0) ──────
    // coreMap[núcleo] = lista de índices de estados LR(1) con ese núcleo
    std::map<std::set<std::pair<int,int>>, std::vector<int>> coreMap;
    for (int i = 0; i < (int)lr1States.size(); i++)
        coreMap[core(lr1States[i])].push_back(i);

    // ── Paso 3: fusionar estados con el mismo núcleo ─────────
    // Crear estados LALR = unión de lookaheads de cada grupo
    // Asignar un índice LALR a cada grupo (usando el menor índice LR(1) del grupo)
    std::map<int, int> lr1ToLalr;  // lr1_state → lalr_state
    std::vector<LR1ItemSet> lalrStates;

    for (auto& [c, group] : coreMap) {
        int lalrIdx = (int)lalrStates.size();
        // El estado LALR = unión de todos los ítems de los estados del grupo
        LR1ItemSet merged;
        for (int lr1Idx : group) {
            merged.insert(lr1States[lr1Idx].begin(), lr1States[lr1Idx].end());
            lr1ToLalr[lr1Idx] = lalrIdx;
        }
        lalrStates.push_back(merged);
    }

    // ── Paso 4: reconstruir transiciones LALR ────────────────
    std::map<int, std::map<std::string, int>> lalrTrans;
    for (auto& [lr1From, symMap] : lr1Trans) {
        int lalrFrom = lr1ToLalr[lr1From];
        for (auto& [sym, lr1To] : symMap)
            lalrTrans[lalrFrom][sym] = lr1ToLalr[lr1To];
    }

    // ── Paso 5: llenar ACTION / GOTO con los estados LALR ────
    for (int i = 0; i < (int)lalrStates.size(); i++) {
        for (const auto& item : lalrStates[i]) {
            bool complete = isComplete(item, g);
            const Production& prod = g.productions[item.prod_idx];

            if (!complete) {
                const std::string& sym = prod.body[item.dot];
                if (g.isTerminal(sym) && sym != "$") {
                    // Shift: ACTION[i, sym] = shift GOTO(i, sym)
                    auto itT = lalrTrans.find(i);
                    if (itT == lalrTrans.end()) continue;
                    auto itS = itT->second.find(sym);
                    if (itS == itT->second.end()) continue;
                    setAction(i, sym, {ActionType::SHIFT, itS->second});
                }
            } else {
                if (prod.head == Sprime) {
                    // Accept: [S' → S •, $]
                    setAction(i, "$", {ActionType::ACCEPT, -1});
                } else {
                    // Reduce: ACTION[i, lookahead] = reduce (usando lookahead LALR)
                    setAction(i, item.lookahead, {ActionType::REDUCE, item.prod_idx});
                }
            }
        }

        // GOTO para no-terminales
        auto itT = lalrTrans.find(i);
        if (itT != lalrTrans.end()) {
            for (const auto& [sym, dst] : itT->second)
                if (g.isNonTerminal(sym))
                    goto_table[i][sym] = dst;
        }
    }
}

// ── Print ─────────────────────────────────────────────────────

void LALRTable::print(const Grammar& g) const {
    std::vector<std::string> terms;
    for (const auto& t : g.terminals) terms.push_back(t);
    terms.push_back("$");

    std::vector<std::string> nonterms;
    for (const auto& nt : g.non_terminals) nonterms.push_back(nt);

    std::cout << "\n========== TABLA LALR(1) ==========\n";

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
    std::cout << "===================================\n";

    if (!conflicts.empty()) {
        std::cout << "\n[LALR] Conflictos (" << conflicts.size() << "):\n";
        for (const auto& c : conflicts) std::cout << "  " << c.message << "\n";
    }
    std::cout << "\n";
}
