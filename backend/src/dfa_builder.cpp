#include "dfa_builder.h"
#include "regex_parser.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

// ─── Arbol combinado aumentado ────────────────

RegexNode* DFABuilder::buildCombinedTree(const YALexSpec& spec, int& pos_counter) {
    pos_counter = 0;
    RegexNode* root = nullptr;
    for (int i = 0; i < (int)spec.rules.size(); i++) {
        RegexParser rp(pos_counter);
        RegexNode* tree = nullptr;
        try { tree = rp.parse(spec.rules[i].expanded_regex); }
        catch (const std::exception& e) {
            throw std::runtime_error("[DFA] Error en regla " + std::to_string(i) +
                                     " '" + spec.rules[i].original_regex + "': " + e.what());
        }
        RegexNode* aug = makeConcat(tree, makeEndMarker(i, pos_counter));
        root = (root == nullptr) ? aug : makeAlter(root, aug);
    }
    return root;
}

// ─── Mejor regla aceptante ───────────────────

int DFABuilder::bestRule(const std::set<int>& spos, const std::vector<int>& pos_rule) {
    int best = -1;
    for (int p : spos)
        if (p < (int)pos_rule.size() && pos_rule[p] >= 0)
            if (best == -1 || pos_rule[p] < best) best = pos_rule[p];
    return best;
}

// ─── Construccion directa del AFD ─────────────

DFA DFABuilder::buildDFA(RegexNode* root, int total,
                          const std::vector<std::bitset<256>>& pos_charset,
                          const std::vector<int>& pos_rule,
                          const std::vector<std::set<int>>& followpos) {
    using PosSet = std::set<int>;
    std::map<PosSet, int> state_map;
    std::vector<PosSet>   state_sets;
    std::queue<int>       worklist;
    DFA dfa;

    PosSet initial = root->firstpos;
    state_map[initial] = 0;
    state_sets.push_back(initial);
    worklist.push(0);
    dfa.states.push_back(DFAState());
    dfa.states[0].id = 0;

    while (!worklist.empty()) {
        int sid = worklist.front(); worklist.pop();
        const PosSet S = state_sets[sid];
        int rule = bestRule(S, pos_rule);
        if (rule >= 0) { dfa.states[sid].is_accepting = true; dfa.states[sid].accepting_rule = rule; }
        for (int a = 0; a < 256; a++) {
            PosSet U;
            for (int p : S)
                if (p < total && pos_charset[p].test(a))
                    U.insert(followpos[p].begin(), followpos[p].end());
            if (U.empty()) continue;
            int next_id;
            auto it = state_map.find(U);
            if (it == state_map.end()) {
                next_id = (int)state_sets.size();
                state_map[U] = next_id;
                state_sets.push_back(U);
                worklist.push(next_id);
                dfa.states.push_back(DFAState());
                dfa.states.back().id = next_id;
            } else next_id = it->second;
            dfa.states[sid].transitions[a] = next_id;
        }
    }
    dfa.start_state = 0;
    return dfa;
}

// ─── Minimizacion por particiones ────────────

DFA DFABuilder::minimize(const DFA& dfa, int num_rules) {
    int N = (int)dfa.states.size();
    if (N == 0) return dfa;
    std::vector<int> group(N, 0);
    for (int s = 0; s < N; s++)
        if (dfa.states[s].is_accepting) group[s] = dfa.states[s].accepting_rule + 1;
    int num_groups = num_rules + 1;
    bool changed = true;
    while (changed) {
        changed = false;
        std::map<std::pair<int, std::vector<int>>, int> sig_map;
        std::vector<int> new_group(N);
        int ng = 0;
        for (int s = 0; s < N; s++) {
            std::vector<int> tg(256);
            for (int a = 0; a < 256; a++) { int t = dfa.states[s].transitions[a]; tg[a] = (t==-1)?-1:group[t]; }
            auto sig = std::make_pair(group[s], tg);
            auto it = sig_map.find(sig);
            if (it == sig_map.end()) { sig_map[sig] = ng; new_group[s] = ng++; }
            else new_group[s] = it->second;
        }
        if (ng != num_groups) { changed = true; group = new_group; num_groups = ng; }
    }
    DFA min_dfa;
    min_dfa.states.resize(num_groups);
    for (int g = 0; g < num_groups; g++) { min_dfa.states[g].id = g; min_dfa.states[g].transitions.fill(-1); }
    min_dfa.start_state = group[dfa.start_state];
    for (int s = 0; s < N; s++) {
        int g = group[s];
        if (dfa.states[s].is_accepting && !min_dfa.states[g].is_accepting) {
            min_dfa.states[g].is_accepting   = true;
            min_dfa.states[g].accepting_rule = dfa.states[s].accepting_rule;
        }
        for (int a = 0; a < 256; a++) {
            int t = dfa.states[s].transitions[a];
            if (t != -1) min_dfa.states[g].transitions[a] = group[t];
        }
    }
    return min_dfa;
}

// ─── Punto de entrada publico ─────────────────

DFA DFABuilder::build(const YALexSpec& spec, RegexNode*& out_tree, int& out_pos_count) {
    if (spec.rules.empty())
        throw std::runtime_error("[DFA] No hay reglas");

    // Paso 1: arbol combinado (R0·#0)|(R1·#1)|...|(Rn·#n)
    int pos_counter = 0;
    RegexNode* root = buildCombinedTree(spec, pos_counter);
    out_pos_count = pos_counter;
    out_tree = root;

    // Paso 2: nullable / firstpos / lastpos
    computePositions(root);

    // Paso 3: followpos
    std::vector<std::set<int>> followpos(pos_counter);
    computeFollowpos(root, followpos);
    std::vector<std::bitset<256>> pos_charset(pos_counter);
    std::vector<int>              pos_rule(pos_counter, -1);
    buildPosCharsetMap(root, pos_charset);
    buildPosRuleMap(root, pos_rule);

    // Paso 4: construccion directa
    DFA dfa = buildDFA(root, pos_counter, pos_charset, pos_rule, followpos);

    // Paso 5: minimizacion
    DFA min_dfa = minimize(dfa, (int)spec.rules.size());

    // ── dfa_builder.cpp devuelve: DFA minimizado ────────────────────────
    std::cout << "[dfa_builder] build() devuelve:\n";

    std::cout << "  firstpos(raiz) = { ";
    for (int p : root->firstpos) std::cout << p << " ";
    std::cout << "} <- estado inicial del AFD\n";

    std::cout << "  followpos (primeras filas no vacias):\n";
    std::cout << "  " << std::setw(5) << "pos"
              << "  " << std::setw(14) << std::left << "tipo"
              << "  followpos\n";
    std::cout << "  " << std::string(48, '-') << "\n";
    int shown = 0;
    for (int i = 0; i < pos_counter && shown < 15; i++) {
        if (followpos[i].empty()) continue;
        std::string tipo = (pos_rule[i] >= 0)
            ? "END_MARKER #" + std::to_string(pos_rule[i])
            : "hoja";
        std::cout << "  p" << std::setw(3) << i
                  << "  " << std::setw(14) << std::left << tipo << "  { ";
        for (int p : followpos[i]) std::cout << p << " ";
        std::cout << "}\n";
        shown++;
    }
    if (pos_counter > 15) std::cout << "  ... (" << pos_counter-15 << " posiciones mas)\n";

    std::cout << "\n  Estados del AFD minimizado:\n";
    std::cout << "  " << std::setw(7) << "estado"
              << "  " << std::setw(12) << std::left << "tipo"
              << "  acepta\n";
    std::cout << "  " << std::string(36, '-') << "\n";
    for (const auto& st : min_dfa.states) {
        std::string tipo = (st.id == min_dfa.start_state) ? "INICIAL     " :
                           st.is_accepting                  ? "ACEPTANTE   " :
                                                              "intermedio  ";
        std::string reg = st.is_accepting ? "regla " + std::to_string(st.accepting_rule) : "-";
        std::cout << "  S" << std::setw(5) << st.id
                  << "  " << tipo << "  " << reg << "\n";
    }
    std::cout << "  Total: " << min_dfa.states.size() << " estados, "
              << pos_counter << " posiciones\n\n";

    return min_dfa;
}
