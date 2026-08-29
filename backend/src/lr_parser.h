#pragma once
#include "slr_table.h"
#include "token_stream.h"
#include "parse_step.h"
#include "parse_tree.h"
#include <vector>
#include <string>

// Resultado del análisis LR (sirve para SLR y LALR)
struct LRParseResult {
    bool        accepted     = false;
    std::string error_msg;
    std::vector<std::string> errors;
    std::vector<int> reductions;
    std::vector<ParseStep> trace;    // traza paso a paso
    std::shared_ptr<ParseTreeNode> parse_tree; // CST si la cadena fue aceptada
};

// ============================================================
//  LRParser
//  Motor de análisis dirigido por tabla para SLR(1) y LALR(1).
//
//  Implementa el algoritmo de pila LR:
//    Stack: [ (estado, símbolo), ... ]
//    1. Leer ACTION[tope, lookahead]
//       • shift s  → empujar token + estado s; avanzar input
//       • reduce r → sacar |cuerpo| entradas; empujar LHS + GOTO[tope, LHS]
//       • accept   → éxito
//       • error    → reportar token problemático con línea/columna
//
//  La misma clase funciona con SLRTable y LALRTable porque
//  ambas exponen los mismos campos `action` y `goto_table`.
// ============================================================
class LRParser {
public:
    // Parsear usando la tabla SLR(1)
    LRParseResult parse(const SLRTable&  table,
                        const Grammar&   grammar,
                        TokenStream&     stream) const;

    // Parsear usando la tabla LALR(1)
    // (sobrecarga para LALRTable — misma lógica, distinta tabla)
    LRParseResult parse(const std::map<int, std::map<std::string, Action>>& actionMap,
                        const std::map<int, std::map<std::string, int>>&    gotoMap,
                        const Grammar&  grammar,
                        TokenStream&    stream) const;

private:
    // Núcleo del algoritmo: recibe los mapas directamente
    LRParseResult run(const std::map<int, std::map<std::string, Action>>& actionMap,
                      const std::map<int, std::map<std::string, int>>&    gotoMap,
                      const Grammar&  grammar,
                      TokenStream&    stream) const;
};
