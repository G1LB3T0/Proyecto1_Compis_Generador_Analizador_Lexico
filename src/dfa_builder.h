#pragma once
#include "types.h"
#include "regex_node.h"
#include <vector>
#include <set>
#include <map>
#include <bitset>

// ─────────────────────────────────────────────
// Módulo 3 – Generador de Autómatas
//
// Aplica el algoritmo de construcción directa:
//   1. Construye el árbol combinado para todas las reglas
//   2. Calcula firstpos/lastpos/followpos
//   3. Construye el AFD directamente (sin pasar por AFN)
//   4. Minimiza el AFD usando el método de particiones
// ─────────────────────────────────────────────

class DFABuilder {
public:
    // Construye el AFD a partir de la especificación completa
    // También llena 'out_tree' con el árbol combinado (para graficación)
    DFA build(const YALexSpec& spec, RegexNode*& out_tree, int& out_pos_count);

private:
    // ── Paso 1: construir árbol combinado ──
    // Combina todas las reglas como: (R1·#0) | (R2·#1) | ... | (Rn·#n-1)
    RegexNode* buildCombinedTree(const YALexSpec& spec, int& pos_counter);

    // ── Paso 2: construction directa ──
    DFA buildDFA(RegexNode* root,
                 int total_positions,
                 const std::vector<std::bitset<256>>& pos_charset,
                 const std::vector<int>& pos_rule,
                 const std::vector<std::set<int>>& followpos);

    // ── Paso 3: minimización por particiones ──
    DFA minimize(const DFA& dfa, int num_rules);

    // Determina cuál regla aceptar cuando un estado tiene múltiples end-markers
    // (máxima prioridad = menor índice de regla)
    int bestRule(const std::set<int>& state_positions,
                 const std::vector<int>& pos_rule);
};
