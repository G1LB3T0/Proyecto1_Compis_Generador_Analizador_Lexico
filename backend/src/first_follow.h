#pragma once
#include "grammar.h"
#include <map>
#include <set>
#include <string>

// ============================================================
//  FirstFollow
//  Calcula los conjuntos FIRST y FOLLOW para cada símbolo
//  de la gramática. Son el preprocesamiento obligatorio
//  para construir la tabla de análisis LL(1).
//
//  FIRST(A)  = conjunto de terminales con los que puede
//              empezar cualquier cadena derivada de A.
//              Si A puede derivar ε, también incluye ε.
//
//  FOLLOW(A) = conjunto de terminales que pueden aparecer
//              inmediatamente después de A en alguna forma
//              sentencial. Siempre incluye $ para el inicio.
//
//  Algoritmo:
//    FIRST  → se calcula en post-order sobre las producciones
//    FOLLOW → usa los FIRST ya calculados, itera hasta convergencia
// ============================================================
class FirstFollow {
public:
    // Calcula FIRST y FOLLOW para toda la gramática
    void compute(const Grammar& g);

    // Devuelve FIRST de un símbolo
    const std::set<std::string>& first(const std::string& symbol) const;

    // Devuelve FOLLOW de un no terminal
    const std::set<std::string>& follow(const std::string& symbol) const;

    // Calcula FIRST de una secuencia de símbolos
    // (útil para el cuerpo de una producción)
    std::set<std::string> firstOfSequence(
        const std::vector<std::string>& seq) const;

    // Imprime las tablas — para debug y evaluación
    void print() const;

private:
    std::map<std::string, std::set<std::string>> first_;
    std::map<std::string, std::set<std::string>> follow_;

    static const std::string EPSILON;  // "ε"
    static const std::string END;      // "$"

    void computeFirst(const Grammar& g);
    void computeFollow(const Grammar& g);
};