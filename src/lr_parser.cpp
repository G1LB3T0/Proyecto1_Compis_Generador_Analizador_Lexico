#include "lr_parser.h"
#include <sstream>
#include <iostream>

// ── Sobrecargas públicas ──────────────────────────────────────

LRParseResult LRParser::parse(const SLRTable& table,
                               const Grammar&  grammar,
                               TokenStream&    stream) const {
    return run(table.action, table.goto_table, grammar, stream);
}

LRParseResult LRParser::parse(
    const std::map<int, std::map<std::string, Action>>& actionMap,
    const std::map<int, std::map<std::string, int>>&    gotoMap,
    const Grammar&  grammar,
    TokenStream&    stream) const {
    return run(actionMap, gotoMap, grammar, stream);
}

// ── Núcleo del algoritmo LR ───────────────────────────────────

LRParseResult LRParser::run(
    const std::map<int, std::map<std::string, Action>>& actionMap,
    const std::map<int, std::map<std::string, int>>&    gotoMap,
    const Grammar&  grammar,
    TokenStream&    stream) const
{
    LRParseResult result;

    // Pila de estados y pila paralela de símbolos (para mensajes de error)
    std::vector<int>         stateStack  = {0};
    std::vector<std::string> symbolStack = {"$"};

    while (true) {
        int         topState  = stateStack.back();
        std::string lookahead = stream.lookahead().tipo;

        // Buscar ACTION[topState, lookahead]
        auto itState = actionMap.find(topState);
        if (itState == actionMap.end()) {
            result.error_msg =
                "Error sintáctico: estado " + std::to_string(topState) +
                " no tiene entradas. Token: '" + lookahead + "'" +
                " en línea " + std::to_string(stream.lookahead().linea) +
                ", col "      + std::to_string(stream.lookahead().columna);
            return result;
        }

        auto itAct = itState->second.find(lookahead);
        if (itAct == itState->second.end() || itAct->second.isEmpty()) {
            // Error: construir mensaje con los símbolos esperados
            std::string expected;
            for (const auto& [sym, act] : itState->second)
                if (!act.isEmpty()) expected += "'" + sym + "' ";

            result.error_msg =
                "Error sintáctico en línea " +
                std::to_string(stream.lookahead().linea) +
                ", col " + std::to_string(stream.lookahead().columna) +
                ":\n  Token inesperado: '" + lookahead +
                "' (\"" + stream.lookahead().lexema + "\")" +
                "\n  Se esperaba uno de: " + expected;
            return result;
        }

        const Action& act = itAct->second;

        if (act.type == ActionType::SHIFT) {
            // Shift: consumir el token y empujar el nuevo estado
            symbolStack.push_back(stream.consume().tipo);
            stateStack.push_back(act.value);

        } else if (act.type == ActionType::REDUCE) {
            // Reduce por la producción act.value
            const Production& prod = grammar.productions[act.value];
            result.reductions.push_back(act.value);

            bool epsilonProd = (prod.body.size() == 1 && prod.body[0] == "");
            int  popCount    = epsilonProd ? 0 : (int)prod.body.size();

            // Sacar |cuerpo| elementos de la pila
            for (int k = 0; k < popCount; k++) {
                stateStack.pop_back();
                symbolStack.pop_back();
            }

            // Empujar el LHS y el nuevo estado desde GOTO
            symbolStack.push_back(prod.head);
            int newTop = stateStack.back();

            auto itGoto = gotoMap.find(newTop);
            if (itGoto == gotoMap.end()) {
                result.error_msg =
                    "Error interno: GOTO vacío en estado " +
                    std::to_string(newTop) + " para '" + prod.head + "'";
                return result;
            }
            auto itGSym = itGoto->second.find(prod.head);
            if (itGSym == itGoto->second.end()) {
                result.error_msg =
                    "Error interno: no hay GOTO[" +
                    std::to_string(newTop) + ", " + prod.head + "]";
                return result;
            }
            stateStack.push_back(itGSym->second);

        } else if (act.type == ActionType::ACCEPT) {
            result.accepted = true;
            return result;
        }
    }
}
