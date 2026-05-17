#pragma once
#include "ll1_table.h"
#include "token_stream.h"
#include <string>
#include <vector>

// Resultado del análisis LL(1)
struct LL1ParseResult {
    bool        accepted = false;
    std::string error_msg;
    // Orden de producciones aplicadas (índices) — derivación izquierda
    std::vector<int> derivations;
};

// ============================================================
//  LL1Parser
//  Motor de análisis predictivo LL(1) dirigido por tabla.
//
//  Algoritmo predictivo:
//    Stack iniciado con [S, $]
//    1. Tope = terminal que coincide con lookahead → match, pop, consume
//    2. Tope = no-terminal A → buscar M[A, lookahead]
//       Si existe: pop A, empujar cuerpo en orden inverso
//    3. Tope = $ y lookahead = $ → accept
//    4. Cualquier otro caso → error con ubicación precisa
// ============================================================
class LL1Parser {
public:
    LL1ParseResult parse(const LL1Table& table,
                         const Grammar&  grammar,
                         TokenStream&    stream) const;
};
