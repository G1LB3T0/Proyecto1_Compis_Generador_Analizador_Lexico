#pragma once
#include "grammar.h"
#include <vector>
#include <map>
#include <set>
#include <string>

// Un ítem LR(0): producción[prod_idx] con el punto en posición `dot`
// Ejemplo: A → α • β  se representa como {prod_idx=i, dot=|α|}
struct LR0Item {
    int prod_idx;
    int dot;

    bool operator<(const LR0Item& o) const {
        return prod_idx != o.prod_idx ? prod_idx < o.prod_idx : dot < o.dot;
    }
    bool operator==(const LR0Item& o) const {
        return prod_idx == o.prod_idx && dot == o.dot;
    }
};

// Un estado del autómata LR(0) es un conjunto de ítems LR(0)
using ItemSet = std::set<LR0Item>;

// ============================================================
//  LR0Automaton
//  Construye la colección canónica de conjuntos de ítems LR(0).
//
//  Algoritmo (Dragon Book, Sección 4.6):
//    1. Aumentar la gramática: S' → S
//    2. Estado inicial = closure({[S' → • S]})
//    3. Para cada estado I y símbolo X: si GOTO(I,X) ≠ ∅ →
//       nuevo estado y transición
//    4. Repetir hasta convergencia
// ============================================================
class LR0Automaton {
public:
    Grammar  augmented;   // gramática original + producción S' → start_symbol
    std::vector<ItemSet> states;  // colección canónica de conjuntos de ítems
    // transitions[estado][símbolo] = estado_destino
    std::map<int, std::map<std::string, int>> transitions;

    // Construye el autómata a partir de una gramática
    void build(const Grammar& g);

    // Imprime estados y transiciones — para debug y evaluación
    void print() const;

private:
    // Agrega S' → start_symbol al inicio de las producciones
    Grammar augmentGrammar(const Grammar& g) const;

    // Cierre de un conjunto de ítems: para [A → α • Bβ] agrega [B → • γ]
    ItemSet closure(const ItemSet& items) const;

    // GOTO(I, X): mueve el punto más allá de X y toma closure
    ItemSet gotoSet(const ItemSet& items, const std::string& sym) const;

    // ¿El ítem está completo (punto al final)?
    bool isComplete(const LR0Item& item) const;

    // Símbolo inmediatamente después del punto
    const std::string& symbolAtDot(const LR0Item& item) const;
};
