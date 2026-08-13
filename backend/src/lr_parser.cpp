#include "lr_parser.h"
#include <sstream>
#include <iostream>

static const int MAX_ERRORS      = 10;
static const int MAX_TRACE_STEPS = 300;

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

    auto addError = [&](const std::string& msg) {
        if (result.error_msg.empty()) result.error_msg = msg;
        result.errors.push_back(msg);
    };

    std::vector<int>         stateStack  = {0};
    std::vector<std::string> symbolStack = {"$"};
    // Pila semántica paralela. A diferencia de symbolStack, no contiene el
    // marcador $, porque cada entrada representa un símbolo real del árbol.
    std::vector<std::shared_ptr<ParseTreeNode>> nodeStack;

    while (true) {
        if ((int)result.errors.size() >= MAX_ERRORS) {
            addError("Análisis detenido: demasiados errores.");
            return result;
        }

        int         topState  = stateStack.back();
        std::string lookahead = stream.lookahead().tipo;

        // ── Capturar estado para la traza ──────────────────────
        bool doTrace = (int)result.trace.size() < MAX_TRACE_STEPS;
        ParseStep step;
        if (doTrace) {
            std::string ss, sym;
            for (size_t k = 0; k < stateStack.size(); k++) {
                if (k) ss += " ";
                ss += std::to_string(stateStack[k]);
            }
            for (size_t k = 0; k < symbolStack.size(); k++) {
                if (k) sym += " ";
                sym += symbolStack[k];
            }
            step.stack   = ss;
            step.symbols = sym;
            step.input   = stream.remainingStr();
        }

        // ── Buscar ACTION ──────────────────────────────────────
        auto itState = actionMap.find(topState);
        if (itState == actionMap.end()) {
            std::string msg =
                "Error sintáctico: estado " + std::to_string(topState) +
                " sin entradas. Token: '" + lookahead + "'" +
                " en línea " + std::to_string(stream.lookahead().linea) +
                ", col " + std::to_string(stream.lookahead().columna);
            if (doTrace) {
                step.action = "ERROR: estado sin entradas para '" + lookahead + "'";
                result.trace.push_back(step);
            }
            addError(msg);
            if (lookahead == "$") return result;
            stream.consume();
            continue;
        }

        auto itAct = itState->second.find(lookahead);
        if (itAct == itState->second.end() || itAct->second.isEmpty()) {
            std::string expected;
            for (const auto& [sym, act] : itState->second)
                if (!act.isEmpty()) expected += "'" + sym + "' ";

            std::string msg =
                "Error sintáctico en línea " +
                std::to_string(stream.lookahead().linea) +
                ", col " + std::to_string(stream.lookahead().columna) +
                ": token inesperado '" + lookahead +
                "' (\"" + stream.lookahead().lexema + "\")" +
                (expected.empty() ? "" : " — esperaba: " + expected);
            if (doTrace) {
                step.action = "ERROR: token '" + lookahead + "' inesperado";
                result.trace.push_back(step);
            }
            addError(msg);

            if (lookahead == "$") return result;
            stream.consume();
            continue;
        }

        const Action& act = itAct->second;

        if (act.type == ActionType::SHIFT) {
            if (doTrace) {
                step.action = "ACTION[" + std::to_string(topState) + "," + lookahead +
                    "] = s" + std::to_string(act.value) +
                    "  →  Shift, ir a estado " + std::to_string(act.value);
                result.trace.push_back(step);
            }
            Token shifted = stream.consume();
            symbolStack.push_back(shifted.tipo);
            stateStack.push_back(act.value);
            nodeStack.push_back(makeTerminalNode(shifted));

        } else if (act.type == ActionType::REDUCE) {
            const Production& prod = grammar.productions[act.value];
            result.reductions.push_back(act.value);

            bool epsilonProd = (prod.body.size() == 1 && prod.body[0] == "");
            int  popCount    = epsilonProd ? 0 : (int)prod.body.size();

            if (doTrace) {
                std::string bodyStr = epsilonProd ? "ε" : "";
                if (!epsilonProd) {
                    for (size_t k = 0; k < prod.body.size(); k++) {
                        if (k) bodyStr += " ";
                        bodyStr += prod.body[k];
                    }
                }
                step.action = "ACTION[" + std::to_string(topState) + "," + lookahead +
                    "] = r" + std::to_string(act.value) +
                    "  →  Reduce " + prod.head + " → " + bodyStr +
                    "  (pop " + std::to_string(popCount) + ")";
                result.trace.push_back(step);
            }

            std::vector<std::shared_ptr<ParseTreeNode>> children;
            if (popCount > 0) {
                const size_t firstChild = nodeStack.size() - popCount;
                children.assign(nodeStack.begin() + firstChild, nodeStack.end());
                nodeStack.erase(nodeStack.begin() + firstChild, nodeStack.end());
            }

            for (int k = 0; k < popCount; k++) {
                stateStack.pop_back();
                symbolStack.pop_back();
            }

            symbolStack.push_back(prod.head);
            nodeStack.push_back(
                makeNonTerminalNode(prod.head, act.value, children));
            int newTop = stateStack.back();

            auto itGoto = gotoMap.find(newTop);
            if (itGoto == gotoMap.end()) {
                addError("Error interno: GOTO vacío en estado " +
                         std::to_string(newTop) + " para '" + prod.head + "'");
                return result;
            }
            auto itGSym = itGoto->second.find(prod.head);
            if (itGSym == itGoto->second.end()) {
                addError("Error interno: no hay GOTO[" +
                         std::to_string(newTop) + ", " + prod.head + "]");
                return result;
            }
            stateStack.push_back(itGSym->second);

        } else if (act.type == ActionType::ACCEPT) {
            if (doTrace) {
                step.action = "ACCEPT ✓";
                result.trace.push_back(step);
            }
            result.accepted = result.errors.empty();
            if (result.accepted && !nodeStack.empty())
                result.parse_tree = nodeStack.back();
            return result;
        }
    }
}
