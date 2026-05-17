#pragma once
#include "grammar.h"
#include "first_follow.h"
#include <map>
#include <string>
#include <vector>

// Conflicto en la tabla LL(1): la gramática no es LL(1)
struct LL1Conflict {
    std::string nonterminal;
    std::string terminal;
    int         existing_prod;   // índice de producción ya registrada
    int         incoming_prod;   // índice de la nueva producción que colisiona
};

// ============================================================
//  LL1Table
//  Tabla de análisis predictivo LL(1).
//
//  Construcción (Dragon Book, Sección 4.4):
//    Para cada producción A → α:
//      Para cada a ∈ FIRST(α) − {ε}: M[A, a] = A → α
//      Si ε ∈ FIRST(α):
//        Para cada b ∈ FOLLOW(A):  M[A, b] = A → α
//
//  Si M[A, a] ya tiene una entrada distinta → conflicto:
//    la gramática NO es LL(1).
// ============================================================
class LL1Table {
public:
    // table[no_terminal][terminal] = índice de producción (-1 = error/vacío)
    std::map<std::string, std::map<std::string, int>> table;
    std::vector<LL1Conflict> conflicts;

    // Construye la tabla usando los conjuntos FIRST/FOLLOW ya calculados
    void build(const Grammar& g, const FirstFollow& ff);

    // Imprime la tabla — para debug y evaluación
    void print(const Grammar& g) const;

    bool hasConflicts() const { return !conflicts.empty(); }
};
