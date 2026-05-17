#pragma once
#include "lr0_automaton.h"
#include "first_follow.h"
#include "slr_table.h"    // reutiliza Action, ActionType, ParseConflict
#include <map>
#include <set>
#include <string>
#include <vector>

// Un ítem LR(1): producción con punto + lookahead terminal
// [A → α • β, a]  →  {prod_idx, dot, lookahead}
// El lookahead indica el símbolo que puede seguir después de reducir A → αβ
struct LR1Item {
    int         prod_idx;
    int         dot;
    std::string lookahead;

    bool operator<(const LR1Item& o) const {
        if (prod_idx != o.prod_idx) return prod_idx < o.prod_idx;
        if (dot       != o.dot)      return dot       < o.dot;
        return lookahead < o.lookahead;
    }
    bool operator==(const LR1Item& o) const {
        return prod_idx == o.prod_idx && dot == o.dot && lookahead == o.lookahead;
    }
};

using LR1ItemSet = std::set<LR1Item>;

// ============================================================
//  LALRTable
//  Tabla de análisis LALR(1).
//
//  Estrategia (Dragon Book, Sección 4.7.4):
//    1. Construir todos los estados LR(1) (ítems con lookahead)
//    2. Agrupar estados que tienen el mismo "núcleo" LR(0)
//       (mismas producciones y posiciones de punto, ignorando lookaheads)
//    3. Fusionar: un estado LALR = unión de lookaheads de su grupo
//    4. Construir ACTION/GOTO igual que SLR pero con los lookaheads LALR
//
//  Ventaja sobre SLR: menos falsos conflictos porque los lookaheads son
//  más precisos que FOLLOW(A). Misma cantidad de estados que LR(0).
// ============================================================
class LALRTable {
public:
    // action[estado][terminal] = Action  (misma estructura que SLRTable)
    std::map<int, std::map<std::string, Action>> action;
    // goto_table[estado][no_terminal] = estado_destino
    std::map<int, std::map<std::string, int>>    goto_table;
    std::vector<ParseConflict> conflicts;

    // Construye la tabla LALR desde el autómata LR(0) y los FIRST/FOLLOW
    void build(const LR0Automaton& lr0, const FirstFollow& ff);

    // Imprime la tabla — para debug y evaluación
    void print(const Grammar& g) const;

    bool hasConflicts() const { return !conflicts.empty(); }

private:
    // Cierre LR(1): para [A → α • Bβ, a] agrega [B → • γ, b] con b ∈ FIRST(βa)
    LR1ItemSet closure1(const LR1ItemSet& items,
                        const Grammar& g,
                        const FirstFollow& ff) const;

    // GOTO LR(1): mueve el punto y toma closure1
    LR1ItemSet goto1(const LR1ItemSet& items,
                     const std::string& sym,
                     const Grammar& g,
                     const FirstFollow& ff) const;

    // Extrae el núcleo de un conjunto LR(1): los ítems sin lookahead
    std::set<std::pair<int,int>> core(const LR1ItemSet& items) const;

    // Escribe en ACTION, detectando conflictos
    void setAction(int state, const std::string& sym, Action a);

    bool isComplete(const LR1Item& item, const Grammar& g) const;
};
