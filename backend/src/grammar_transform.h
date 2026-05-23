#pragma once
#include "grammar.h"

// Transformaciones de gramática necesarias para análisis LL(1).
// Deben aplicarse ANTES de construir la tabla LL(1).
class GrammarTransform {
public:
    // Aplica eliminación de recursión izquierda + factorización izquierda.
    // Devuelve una gramática equivalente apta para LL(1).
    Grammar transform(const Grammar& g) const;

    // Solo elimina recursión izquierda (directa e indirecta).
    Grammar eliminateLeftRecursion(const Grammar& g) const;

    // Solo aplica factorización izquierda.
    Grammar leftFactor(const Grammar& g) const;

    // Devuelve true si la gramática tiene recursión izquierda directa.
    bool hasLeftRecursion(const Grammar& g) const;
};
