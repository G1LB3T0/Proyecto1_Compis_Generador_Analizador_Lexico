#include "ll1_parser.h"
#include <iostream>

static const int MAX_ERRORS      = 10;
static const int MAX_TRACE_STEPS = 300;

LL1ParseResult LL1Parser::parse(const LL1Table& table,
                                 const Grammar&  grammar,
                                 TokenStream&    stream) const
{
    LL1ParseResult result;

    auto addError = [&](const std::string& msg) {
        if (result.error_msg.empty()) result.error_msg = msg;
        result.errors.push_back(msg);
    };

    // Pila de símbolos (back = tope)
    std::vector<std::string> stk;
    stk.push_back("$");
    stk.push_back(grammar.start_symbol);

    // Pila paralela de nodos del árbol
    auto root = std::make_shared<TreeNode>(grammar.start_symbol);
    std::vector<TreeNodePtr> nodeStk;
    nodeStk.push_back(nullptr);   // para el $
    nodeStk.push_back(root);

    while (!stk.empty()) {
        if ((int)result.errors.size() >= MAX_ERRORS) {
            addError("Análisis detenido: demasiados errores.");
            return result;
        }

        const std::string top       = stk.back();
        const std::string lookahead = stream.lookahead().tipo;

        // ── Capturar estado para la traza (tope primero) ───────
        bool doTrace = (int)result.trace.size() < MAX_TRACE_STEPS;
        ParseStep step;
        if (doTrace) {
            std::string stkStr;
            for (int k = (int)stk.size() - 1; k >= 0; k--) {
                if (k < (int)stk.size() - 1) stkStr += " ";
                stkStr += stk[k];
            }
            step.stack   = stkStr;
            step.symbols = "";
            step.input   = stream.remainingStr();
        }

        // ── Aceptación ─────────────────────────────────────────
        if (top == "$" && lookahead == "$") {
            if (doTrace) { step.action = "ACCEPT ✓"; result.trace.push_back(step); }
            result.accepted = result.errors.empty();
            result.tree = root;
            return result;
        }

        if (grammar.isTerminal(top) || top == "$") {
            if (top == lookahead) {
                if (doTrace) {
                    step.action = "MATCH " + top +
                        "  →  consumir \"" + stream.lookahead().lexema + "\"";
                    result.trace.push_back(step);
                }
                // Registrar lexema en el nodo hoja
                if (nodeStk.back()) nodeStk.back()->lexema = stream.lookahead().lexema;
                nodeStk.pop_back();
                stk.pop_back();
                stream.consume();
            } else {
                std::string msg =
                    "Error en línea " +
                    std::to_string(stream.lookahead().linea) +
                    ", col " + std::to_string(stream.lookahead().columna) +
                    ": se esperaba '" + top +
                    "' pero se encontró '" + lookahead +
                    "' (\"" + stream.lookahead().lexema + "\")";
                if (doTrace) {
                    step.action = "ERROR: esperaba '" + top +
                        "', encontró '" + lookahead + "'";
                    result.trace.push_back(step);
                }
                addError(msg);
                nodeStk.pop_back();
                stk.pop_back();
            }

        } else if (grammar.isNonTerminal(top)) {
            auto itNT = table.table.find(top);
            if (itNT == table.table.end() ||
                itNT->second.find(lookahead) == itNT->second.end() ||
                itNT->second.at(lookahead) == -1)
            {
                std::string valid;
                if (itNT != table.table.end())
                    for (const auto& [t, p] : itNT->second)
                        if (p != -1) valid += "'" + t + "' ";

                std::string msg =
                    "Error en línea " +
                    std::to_string(stream.lookahead().linea) +
                    ", col " + std::to_string(stream.lookahead().columna) +
                    ": no hay producción para [" + top + ", " + lookahead + "]" +
                    (valid.empty() ? "" : " — esperaba: " + valid);
                if (doTrace) {
                    step.action = "ERROR: no hay M[" + top + "," + lookahead + "]";
                    result.trace.push_back(step);
                }
                addError(msg);

                if (lookahead == "$") return result;
                stream.consume();

            } else {
                int prodIdx = itNT->second.at(lookahead);
                result.derivations.push_back(prodIdx);

                const Production& prod = grammar.productions[prodIdx];
                bool epsilonProd = (prod.body.size() == 1 && prod.body[0] == "");

                if (doTrace) {
                    std::string bodyStr = epsilonProd ? "ε" : "";
                    if (!epsilonProd) {
                        for (size_t k = 0; k < prod.body.size(); k++) {
                            if (k) bodyStr += " ";
                            bodyStr += prod.body[k];
                        }
                    }
                    step.action = "M[" + top + "," + lookahead + "] = " +
                        prod.head + " → " + bodyStr +
                        "  (prod " + std::to_string(prodIdx) + ")";
                    result.trace.push_back(step);
                }

                // Nodo padre actual (tope de nodeStk = nodo para 'top')
                TreeNodePtr parentNode = nodeStk.back();
                nodeStk.pop_back();
                stk.pop_back();

                if (epsilonProd) {
                    if (parentNode)
                        parentNode->children.push_back(
                            std::make_shared<TreeNode>("ε", "ε"));
                } else {
                    // Crear hijos en orden y adjuntarlos
                    std::vector<TreeNodePtr> children;
                    for (const auto& sym : prod.body)
                        children.push_back(std::make_shared<TreeNode>(sym));
                    if (parentNode) parentNode->children = children;

                    // Empujar en orden inverso (para que el primero quede en el tope)
                    for (int k = (int)prod.body.size() - 1; k >= 0; k--) {
                        stk.push_back(prod.body[k]);
                        nodeStk.push_back(children[k]);
                    }
                }
            }

        } else {
            addError("Símbolo desconocido en la pila: '" + top + "'");
            return result;
        }
    }

    if (result.errors.empty()) {
        result.accepted = true;
        result.tree = root;
    }
    return result;
}
