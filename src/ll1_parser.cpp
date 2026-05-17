#include "ll1_parser.h"
#include <stack>
#include <iostream>

LL1ParseResult LL1Parser::parse(const LL1Table& table,
                                 const Grammar&  grammar,
                                 TokenStream&    stream) const
{
    LL1ParseResult result;

    // Pila inicializada con el símbolo inicial y el marcador de fin
    std::stack<std::string> stk;
    stk.push("$");
    stk.push(grammar.start_symbol);

    while (!stk.empty()) {
        const std::string top      = stk.top();
        const std::string lookahead = stream.lookahead().tipo;

        // Condición de aceptación: ambos marcadores coinciden en la cima
        if (top == "$" && lookahead == "$") {
            result.accepted = true;
            return result;
        }

        if (grammar.isTerminal(top) || top == "$") {
            // Match: el terminal de la pila debe coincidir con el input
            if (top == lookahead) {
                stk.pop();
                stream.consume();
            } else {
                result.error_msg =
                    "Error sintáctico en línea " +
                    std::to_string(stream.lookahead().linea) +
                    ", col " + std::to_string(stream.lookahead().columna) +
                    ":\n  Se esperaba '" + top +
                    "' pero se encontró '" + lookahead +
                    "' (\"" + stream.lookahead().lexema + "\")";
                return result;
            }

        } else if (grammar.isNonTerminal(top)) {
            // Consultar la tabla M[top, lookahead]
            auto itNT = table.table.find(top);
            if (itNT == table.table.end() ||
                itNT->second.find(lookahead) == itNT->second.end() ||
                itNT->second.at(lookahead) == -1)
            {
                // Construir lista de terminales válidos para este no-terminal
                std::string valid;
                if (itNT != table.table.end())
                    for (const auto& [t, p] : itNT->second)
                        if (p != -1) valid += "'" + t + "' ";

                result.error_msg =
                    "Error sintáctico en línea " +
                    std::to_string(stream.lookahead().linea) +
                    ", col " + std::to_string(stream.lookahead().columna) +
                    ":\n  No hay producción para [" + top + ", " + lookahead + "]" +
                    (valid.empty() ? "" : "\n  Símbolos válidos: " + valid);
                return result;
            }

            int prodIdx = itNT->second.at(lookahead);
            result.derivations.push_back(prodIdx);

            const Production& prod = grammar.productions[prodIdx];
            stk.pop();  // sacar el no-terminal A

            // Empujar el cuerpo en orden INVERSO para que el primer símbolo quede al tope
            bool epsilonProd = (prod.body.size() == 1 && prod.body[0] == "");
            if (!epsilonProd) {
                for (int k = (int)prod.body.size() - 1; k >= 0; k--)
                    stk.push(prod.body[k]);
            }
            // Si es ε: solo se hace pop del no-terminal (no se empuja nada)

        } else {
            result.error_msg = "Símbolo desconocido en la pila: '" + top + "'";
            return result;
        }
    }

    // La pila se vació sin llegar al accept
    result.error_msg = "Análisis incompleto: pila vacía sin token $";
    return result;
}
