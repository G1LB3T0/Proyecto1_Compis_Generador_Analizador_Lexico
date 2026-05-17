#pragma once
#include "lr0_automaton.h"
#include "first_follow.h"
#include <map>
#include <string>
#include <vector>

// ── Tipos de acción en la tabla ACTION ───────────────────────
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

// Una celda de la tabla ACTION
struct Action {
    ActionType  type  = ActionType::ERROR;
    int         value = -1;   // estado destino (SHIFT) o índice de producción (REDUCE)

    std::string toString() const;
    bool isEmpty() const { return type == ActionType::ERROR; }
};

// Registro de un conflicto detectado durante la construcción
struct ParseConflict {
    int         state;
    std::string symbol;
    Action      existing;
    Action      incoming;
    std::string message;
};

// ============================================================
//  SLRTable
//  Tabla de análisis SLR(1).
//
//  Construcción (Dragon Book, Sección 4.7):
//    Para cada estado i y cada ítem en ese estado:
//      • [A → α • aβ]  →  ACTION[i, a] = shift GOTO(i, a)
//      • [A → α •]     →  ACTION[i, b] = reduce A→α  ∀b ∈ FOLLOW(A)
//      • [S'→ S •]     →  ACTION[i, $] = accept
//    Para no-terminales A: GOTO[i, A] = transición del autómata LR(0)
//
//  Si una celda ya tiene valor al intentar escribir → conflicto.
//  Por defecto: en shift-reduce, prefiere shift (comportamiento estándar).
// ============================================================
class SLRTable {
public:
    // action[estado][terminal] = Action
    std::map<int, std::map<std::string, Action>> action;
    // goto_table[estado][no_terminal] = estado_destino
    std::map<int, std::map<std::string, int>>    goto_table;
    // Conflictos encontrados (vacío = gramática SLR(1) válida)
    std::vector<ParseConflict> conflicts;

    // Construye la tabla desde el autómata LR(0) y los conjuntos FOLLOW
    void build(const LR0Automaton& automaton, const FirstFollow& ff);

    // Imprime la tabla completa — para debug y evaluación
    void print(const Grammar& g) const;

    bool hasConflicts() const { return !conflicts.empty(); }

private:
    // Escribe en ACTION, detectando conflictos
    void setAction(int state, const std::string& sym, Action a);
};
