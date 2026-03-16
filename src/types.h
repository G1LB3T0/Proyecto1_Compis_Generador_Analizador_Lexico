#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <array>

// ─────────────────────────────────────────────
// Módulo 1 – Procesador de Especificaciones
// ─────────────────────────────────────────────

// Una regla léxica: expresión regular (ya expandida) + acción semántica
struct LexRule {
    std::string original_regex;  // regex tal como está en el .yal
    std::string expanded_regex;  // regex con macros sustituidas
    std::string action;          // código de acción
    int priority;                // orden de definición (menor = mayor prioridad)
};

// Especificación completa del .yal
struct YALexSpec {
    std::string header;                           // código del bloque { header }
    std::string trailer;                          // código del bloque { trailer }
    std::string entrypoint;                       // nombre del punto de entrada
    std::map<std::string, std::string> defs;      // tabla de símbolos: ident → regex
    std::vector<LexRule> rules;                   // lista de reglas en orden
};

// ─────────────────────────────────────────────
// Módulo 3 – Autómata
// ─────────────────────────────────────────────

// Un estado del AFD minimizado
struct DFAState {
    int id;
    bool is_accepting;
    int accepting_rule;               // índice de la regla que acepta (-1 si no acepta)
    std::array<int, 256> transitions; // transitions[c] = next_state, -1 si no hay

    DFAState() : id(-1), is_accepting(false), accepting_rule(-1) {
        transitions.fill(-1);
    }
};

// AFD completo
struct DFA {
    std::vector<DFAState> states;
    int start_state = 0;
};