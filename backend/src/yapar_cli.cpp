#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "yalex_parser.h"
#include "dfa_builder.h"
#include "types.h"
#include "yapar_parser.h"
#include "first_follow.h"
#include "lr0_automaton.h"
#include "slr_table.h"
#include "ll1_table.h"
#include "lalr_table.h"
#include "lr_parser.h"
#include "ll1_parser.h"
#include "grammar_transform.h"
#include "token_stream.h"
#include "parse_step.h"
#include "parse_tree.h"

// ── JSON helpers ──────────────────────────────────────────────

static std::string jstr(const std::string& s) {
    std::string r = "\"";
    for (unsigned char c : s) {
        if      (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else if (c == '\t') r += "\\t";
        else                r += (char)c;
    }
    return r + "\"";
}

static std::string jbool(bool b) { return b ? "true" : "false"; }
static std::string jint(int v)   { return std::to_string(v); }

static std::string jstrArr(const std::vector<std::string>& v) {
    std::string r = "[";
    for (size_t i = 0; i < v.size(); i++) {
        r += jstr(v[i]);
        if (i + 1 < v.size()) r += ",";
    }
    return r + "]";
}

static std::string jstrSet(const std::set<std::string>& s) {
    std::string r = "[";
    bool first = true;
    for (const auto& x : s) {
        if (!first) r += ",";
        r += jstr(x);
        first = false;
    }
    return r + "]";
}

// ── Tokenizador (igual que en menu.cpp) ──────────────────────

static std::string extractTokenName(const std::string& action) {
    size_t lt = action.find('"');
    if (lt == std::string::npos) {
        size_t ret = action.find("return ");
        if (ret != std::string::npos) {
            std::string a = action.substr(ret + 7);
            size_t s = a.find_first_not_of(" \t");
            size_t e = a.find_last_not_of(" \t;");
            if (s != std::string::npos) return a.substr(s, e - s + 1);
        }
        return "";
    }
    size_t angle = action.find('<', lt);
    if (angle == std::string::npos) return "";
    size_t comma = action.find(',', angle);
    if (comma == std::string::npos) return "";
    std::string name = action.substr(angle + 1, comma - angle - 1);
    size_t s = name.find_first_not_of(" \t");
    size_t e = name.find_last_not_of(" \t");
    if (s == std::string::npos) return "";
    return name.substr(s, e - s + 1);
}

static std::vector<Token> tokenize(const DFA& dfa,
                                    const YALexSpec& spec,
                                    const std::string& input)
{
    std::vector<std::string> names;
    for (const auto& r : spec.rules) names.push_back(extractTokenName(r.action));

    std::vector<Token> result;
    size_t pos = 0;
    int line = 1, col = 1;

    while (pos < input.size()) {
        int state = dfa.start_state;
        int lastRule = -1;
        size_t lastPos = pos;
        int lastLine = line, lastCol = col;
        int curLine = line, curCol = col;
        size_t fwd = pos;

        while (fwd < input.size()) {
            unsigned char c = input[fwd];
            int next = dfa.states[state].transitions[c];
            if (next == -1) break;
            state = next;
            if (dfa.states[state].is_accepting) {
                lastRule = dfa.states[state].accepting_rule;
                lastPos  = fwd + 1;
                lastLine = curLine; lastCol = curCol;
            }
            if (c == '\n') { curLine++; curCol = 1; } else curCol++;
            fwd++;
        }

        if (lastRule == -1) {
            if (input[pos] == '\n') { line++; col = 1; } else col++;
            pos++; continue;
        }

        std::string lexeme = input.substr(pos, lastPos - pos);
        for (size_t k = pos; k < lastPos; k++) {
            if (input[k] == '\n') { line++; col = 1; } else col++;
        }
        pos = lastPos;

        std::string name = (lastRule < (int)names.size()) ? names[lastRule] : "";
        if (!name.empty())
            result.push_back(Token(name, lexeme, lastLine, lastCol));
    }
    result.push_back(Token("$", "$", line, col));
    return result;
}

// ── Serializadores ────────────────────────────────────────────

static std::string grammarToJson(const Grammar& g) {
    std::string r = "{";
    r += "\"start\":" + jstr(g.start_symbol) + ",";
    r += "\"terminals\":" + jstrArr(g.terminals) + ",";
    r += "\"non_terminals\":" + jstrArr(g.non_terminals) + ",";
    r += "\"productions\":[";
    for (size_t i = 0; i < g.productions.size(); i++) {
        const auto& p = g.productions[i];
        bool eps = (p.body.size() == 1 && p.body[0].empty());
        std::string body = eps ? "ε" : "";
        if (!eps) for (size_t k = 0; k < p.body.size(); k++) {
            if (k) body += " ";
            body += p.body[k];
        }
        r += "{\"id\":" + jint(i) + ",\"head\":" + jstr(p.head)
          + ",\"body\":" + jstr(body) + "}";
        if (i + 1 < g.productions.size()) r += ",";
    }
    r += "]}";
    return r;
}

static std::string ffToJson(const FirstFollow& ff, const Grammar& g) {
    std::string r = "{\"first\":{";
    bool f1 = true;
    for (const auto& nt : g.non_terminals) {
        if (!f1) r += ",";
        r += jstr(nt) + ":" + jstrSet(ff.first(nt));
        f1 = false;
    }
    r += "},\"follow\":{";
    bool f2 = true;
    for (const auto& nt : g.non_terminals) {
        if (!f2) r += ",";
        r += jstr(nt) + ":" + jstrSet(ff.follow(nt));
        f2 = false;
    }
    r += "}}";
    return r;
}

static std::string itemStr(const LR0Item& item, const Grammar& aug) {
    const auto& prod = aug.productions[item.prod_idx];
    bool eps = (prod.body.size() == 1 && prod.body[0].empty());
    std::string s = prod.head + " → ";
    if (eps) {
        s += (item.dot == 0) ? "• ε" : "ε •";
    } else {
        for (int k = 0; k <= (int)prod.body.size(); k++) {
            if (k == item.dot) s += "•";
            if (k < (int)prod.body.size()) {
                if (k == item.dot) s += " ";
                s += prod.body[k];
                if (k + 1 < (int)prod.body.size() || item.dot <= (int)prod.body.size())
                    s += " ";
            }
        }
    }
    // trim trailing space
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

static std::string lr0ToJson(const LR0Automaton& lr0) {
    std::string r = "{\"count\":" + jint(lr0.states.size()) + ",\"states\":[";
    for (size_t i = 0; i < lr0.states.size(); i++) {
        r += "{\"id\":" + jint(i) + ",\"items\":[";
        bool fi = true;
        for (const auto& item : lr0.states[i]) {
            if (!fi) r += ",";
            r += jstr(itemStr(item, lr0.augmented));
            fi = false;
        }
        r += "],\"transitions\":{";
        auto it = lr0.transitions.find(i);
        if (it != lr0.transitions.end()) {
            bool ft = true;
            for (const auto& [sym, dst] : it->second) {
                if (!ft) r += ",";
                r += jstr(sym) + ":" + jint(dst);
                ft = false;
            }
        }
        r += "}}";
        if (i + 1 < lr0.states.size()) r += ",";
    }
    r += "]}";
    return r;
}

static std::string actionLabel(const Action& a) {
    if (a.type == ActionType::SHIFT)  return "s" + std::to_string(a.value);
    if (a.type == ActionType::REDUCE) return "r" + std::to_string(a.value);
    if (a.type == ActionType::ACCEPT) return "acc";
    return "";
}

static std::string actionType(const Action& a) {
    if (a.type == ActionType::SHIFT)  return "shift";
    if (a.type == ActionType::REDUCE) return "reduce";
    if (a.type == ActionType::ACCEPT) return "accept";
    return "error";
}

static std::string lrTableToJson(
    const std::string& method,
    const std::map<int, std::map<std::string, Action>>& actionMap,
    const std::map<int, std::map<std::string, int>>&    gotoMap,
    const Grammar& g,
    bool hasConflicts,
    const std::vector<ParseConflict>& conflicts)
{
    // Recolectar headers
    std::vector<std::string> actHdrs;
    for (const auto& t : g.terminals) actHdrs.push_back(t);
    actHdrs.push_back("$");

    // NTs del goto (excluir S')
    std::vector<std::string> gotoHdrs;
    for (const auto& nt : g.non_terminals) {
        if (nt != g.start_symbol + "'") gotoHdrs.push_back(nt);
    }

    std::string r = "{\"method\":" + jstr(method);
    r += ",\"has_conflicts\":" + jbool(hasConflicts);
    r += ",\"action_headers\":" + jstrArr(actHdrs);
    r += ",\"goto_headers\":" + jstrArr(gotoHdrs);
    r += ",\"rows\":[";

    // Recolectar todos los estados
    std::set<int> states;
    for (const auto& [s, _] : actionMap) states.insert(s);
    for (const auto& [s, _] : gotoMap)   states.insert(s);

    bool firstRow = true;
    for (int s : states) {
        if (!firstRow) r += ",";
        r += "{\"state\":" + jint(s) + ",\"action\":{";
        auto itA = actionMap.find(s);
        bool fa = true;
        if (itA != actionMap.end()) {
            for (const auto& hdr : actHdrs) {
                auto itC = itA->second.find(hdr);
                if (itC != itA->second.end() && !itC->second.isEmpty()) {
                    if (!fa) r += ",";
                    r += jstr(hdr) + ":{\"type\":" + jstr(actionType(itC->second))
                      + ",\"label\":" + jstr(actionLabel(itC->second))
                      + ",\"value\":" + jint(itC->second.value) + "}";
                    fa = false;
                }
            }
        }
        r += "},\"goto\":{";
        auto itG = gotoMap.find(s);
        bool fg = true;
        if (itG != gotoMap.end()) {
            for (const auto& [nt, dst] : itG->second) {
                if (!fg) r += ",";
                r += jstr(nt) + ":" + jint(dst);
                fg = false;
            }
        }
        r += "}}";
        firstRow = false;
    }
    r += "]}";
    return r;
}

static std::string ll1TableToJson(const LL1Table& tbl, const Grammar& g) {
    std::vector<std::string> actHdrs;
    for (const auto& t : g.terminals) actHdrs.push_back(t);
    actHdrs.push_back("$");

    std::string r = "{\"method\":\"ll1\"";
    r += ",\"has_conflicts\":" + jbool(tbl.hasConflicts());
    r += ",\"action_headers\":" + jstrArr(actHdrs);
    r += ",\"goto_headers\":[]";
    r += ",\"rows\":[";

    bool firstRow = true;
    for (const auto& nt : g.non_terminals) {
        if (!firstRow) r += ",";
        r += "{\"state\":" + jstr(nt) + ",\"nt\":" + jstr(nt) + ",\"action\":{";
        auto itNT = tbl.table.find(nt);
        bool fa = true;
        if (itNT != tbl.table.end()) {
            for (const auto& hdr : actHdrs) {
                auto itC = itNT->second.find(hdr);
                if (itC != itNT->second.end() && itC->second != -1) {
                    int pi = itC->second;
                    const auto& p = g.productions[pi];
                    bool eps = (p.body.size() == 1 && p.body[0].empty());
                    std::string bodyStr = eps ? "ε" : "";
                    if (!eps) for (size_t k = 0; k < p.body.size(); k++) {
                        if (k) bodyStr += " ";
                        bodyStr += p.body[k];
                    }
                    std::string label = p.head + "→" + bodyStr;
                    if (!fa) r += ",";
                    r += jstr(hdr) + ":{\"type\":\"predict\",\"label\":"
                      + jstr(label) + ",\"value\":" + jint(pi) + "}";
                    fa = false;
                }
            }
        }
        r += "},\"goto\":{}}";
        firstRow = false;
    }
    r += "]}";
    return r;
}

// Serializa ítem LR(1) agrupando lookaheads del mismo (prod,dot) → "E → T • E'  {+, $}"
static std::string lr1ItemStr(
    const std::map<std::pair<int,int>, std::set<std::string>>& grouped,
    const std::pair<int,int>& key,
    const Grammar& aug)
{
    const auto& prod = aug.productions[key.first];
    int  dot = key.second;
    bool eps = (prod.body.size() == 1 && prod.body[0].empty());
    std::string s = prod.head + " → ";
    if (eps) {
        s += (dot == 0) ? "• ε" : "ε •";
    } else {
        for (int k = 0; k <= (int)prod.body.size(); k++) {
            if (k == dot) s += "•";
            if (k < (int)prod.body.size()) {
                if (k == dot) s += " ";
                s += prod.body[k];
                if (k + 1 < (int)prod.body.size() || dot <= (int)prod.body.size()) s += " ";
            }
        }
        while (!s.empty() && s.back() == ' ') s.pop_back();
    }
    const auto& las = grouped.at(key);
    s += "  {";
    bool first = true;
    for (const auto& la : las) {
        if (!first) s += ", ";
        s += la;
        first = false;
    }
    s += "}";
    return s;
}

static std::string lr1AutomatonToJson(
    const std::vector<LR1ItemSet>& states,
    const std::map<int, std::map<std::string, int>>& trans,
    const Grammar& aug)
{
    std::string r = "{\"count\":" + jint(states.size()) + ",\"states\":[";
    for (size_t i = 0; i < states.size(); i++) {
        std::map<std::pair<int,int>, std::set<std::string>> grouped;
        for (const auto& item : states[i])
            grouped[{item.prod_idx, item.dot}].insert(item.lookahead);

        r += "{\"id\":" + jint(i) + ",\"items\":[";
        bool fi = true;
        for (const auto& [key, _] : grouped) {
            if (!fi) r += ",";
            r += jstr(lr1ItemStr(grouped, key, aug));
            fi = false;
        }
        r += "],\"transitions\":{";
        auto it = trans.find(i);
        if (it != trans.end()) {
            bool ft = true;
            for (const auto& [sym, dst] : it->second) {
                if (!ft) r += ",";
                r += jstr(sym) + ":" + jint(dst);
                ft = false;
            }
        }
        r += "}}";
        if (i + 1 < states.size()) r += ",";
    }
    return r + "]}";
}

static std::string treeToJson(const TreeNodePtr& node) {
    if (!node) return "null";
    std::string r = "{\"label\":" + jstr(node->label)
                  + ",\"lexema\":" + jstr(node->lexema)
                  + ",\"children\":[";
    for (size_t i = 0; i < node->children.size(); i++) {
        if (i) r += ",";
        r += treeToJson(node->children[i]);
    }
    return r + "]}";
}

static std::string traceToJson(const std::vector<ParseStep>& trace) {
    std::string r = "[";
    for (size_t i = 0; i < trace.size(); i++) {
        if (i) r += ",";
        r += "{\"stack\":"   + jstr(trace[i].stack)
          + ",\"symbols\":" + jstr(trace[i].symbols)
          + ",\"input\":"   + jstr(trace[i].input)
          + ",\"action\":"  + jstr(trace[i].action) + "}";
    }
    return r + "]";
}

static std::string tokensToJson(const std::vector<Token>& tokens) {
    std::string r = "[";
    for (size_t i = 0; i < tokens.size(); i++) {
        r += "{\"tipo\":" + jstr(tokens[i].tipo)
          + ",\"lexema\":" + jstr(tokens[i].lexema)
          + ",\"l\":" + jint(tokens[i].linea)
          + ",\"c\":" + jint(tokens[i].columna) + "}";
        if (i + 1 < tokens.size()) r += ",";
    }
    return r + "]";
}

// ── MAIN ──────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::string yal_path, yalp_path, method = "slr", input_str;
    bool has_input = false;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--yal"    && i+1 < argc) { yal_path  = argv[++i]; }
        else if (a == "--yalp"   && i+1 < argc) { yalp_path = argv[++i]; }
        else if (a == "--method" && i+1 < argc) { method    = argv[++i]; }
        else if (a == "--input"  && i+1 < argc) { input_str = argv[++i]; has_input = true; }
    }

    if (yal_path.empty() || yalp_path.empty()) {
        std::cout << "{\"ok\":false,\"error\":\"Faltan --yal y/o --yalp\"}" << std::endl;
        return 1;
    }

    // Suprimir la salida verbose de los parsers durante el build
    std::streambuf* origCout = std::cout.rdbuf();
    std::ofstream   devNull("/dev/null");
    std::cout.rdbuf(devNull.rdbuf());

    // ── Proyecto 1: DFA ──────────────────────────────────────
    YALexSpec spec;
    DFA dfa;
    try {
        YALexParser yalex;
        spec = yalex.parse(yal_path);
        RegexNode* tree = nullptr; int pc = 0;
        DFABuilder builder;
        dfa = builder.build(spec, tree, pc);
        freeTree(tree);
    } catch (const std::exception& e) {
        std::cout << "{\"ok\":false,\"error\":" << jstr(std::string("DFA: ") + e.what()) << "}" << std::endl;
        return 1;
    }

    std::vector<std::string> dfaTokens;
    std::vector<std::string> dfaPatterns;
    for (const auto& r : spec.rules) {
        std::string n = extractTokenName(r.action);
        if (!n.empty()) {
            dfaTokens.push_back(n);
            dfaPatterns.push_back(r.original_regex);
        }
    }

    // ── Proyecto 2: Gramática, FIRST/FOLLOW, LR(0) ───────────
    Grammar grammar;
    FirstFollow ff;
    LR0Automaton lr0;
    try {
        YaparParser yapar;
        grammar = yapar.parse(yalp_path);
        ff.compute(grammar);
        lr0.build(grammar);
    } catch (const std::exception& e) {
        std::cout << "{\"ok\":false,\"error\":" << jstr(std::string("Gramática: ") + e.what()) << "}" << std::endl;
        return 1;
    }

    // ── Tabla de análisis ─────────────────────────────────────
    SLRTable  slrTable;
    LALRTable lalrTable;
    LL1Table  ll1Table;
    Grammar   grammarLL1;
    FirstFollow ffLL1;
    bool useSLR = false, useLALR = false, useLL1 = false;

    try {
        if (method == "slr") {
            slrTable.build(lr0, ff);
            useSLR = true;
        } else if (method == "lalr") {
            lalrTable.build(lr0, ff);
            useLALR = true;
        } else {
            // LL(1): transformar gramática primero
            GrammarTransform gt;
            grammarLL1 = gt.transform(grammar);
            ffLL1.compute(grammarLL1);
            ll1Table.build(grammarLL1, ffLL1);
            useLL1 = true;
        }
    } catch (const std::exception& e) {
        std::cout << "{\"ok\":false,\"error\":" << jstr(std::string("Tabla: ") + e.what()) << "}" << std::endl;
        return 1;
    }

    // ── Ensamblar JSON de build ───────────────────────────────
    std::string tableJson;
    if (useSLR)
        tableJson = lrTableToJson("slr", slrTable.action, slrTable.goto_table,
                                   lr0.augmented, slrTable.hasConflicts(), slrTable.conflicts);
    else if (useLALR)
        tableJson = lrTableToJson("lalr", lalrTable.action, lalrTable.goto_table,
                                   lr0.augmented, lalrTable.hasConflicts(), lalrTable.conflicts);
    else
        tableJson = ll1TableToJson(ll1Table, grammarLL1);

    std::string out = "{\"ok\":true";
    out += ",\"dfa\":{\"states\":" + jint(dfa.states.size())
        + ",\"tokens\":" + jstrArr(dfaTokens)
        + ",\"patterns\":" + jstrArr(dfaPatterns) + "}";
    out += ",\"grammar\":" + grammarToJson(grammar);
    if (useLL1)
        out += ",\"grammar_ll1\":" + grammarToJson(grammarLL1);
    else
        out += ",\"grammar_ll1\":null";
    out += ",\"ff\":" + ffToJson(ff, grammar);
    if (useLL1)
        out += ",\"ff_ll1\":" + ffToJson(ffLL1, grammarLL1);
    else
        out += ",\"ff_ll1\":null";
    if (useSLR)
        out += ",\"lr0\":" + lr0ToJson(lr0) + ",\"lr1\":null";
    else if (useLALR)
        out += ",\"lr0\":null,\"lr1\":" + lr1AutomatonToJson(
                    lalrTable.mergedStates, lalrTable.mergedTrans, lr0.augmented);
    else
        out += ",\"lr0\":null,\"lr1\":null";
    out += ",\"table\":" + tableJson;

    // ── Si hay input: tokenizar + parsear ────────────────────
    if (has_input) {
        std::vector<Token> tokens = tokenize(dfa, spec, input_str);

        std::vector<Token> filtered;
        const Grammar& g = useLL1 ? grammarLL1 : grammar;
        for (const auto& t : tokens)
            if (!g.ignored.count(t.tipo)) filtered.push_back(t);

        bool accepted = false;
        std::vector<std::string> errors;
        std::vector<ParseStep>   trace;
        TreeNodePtr              tree;

        LRParser  lrParser;
        LL1Parser ll1Parser;

        if (useSLR) {
            TokenStream ts(filtered);
            auto res = lrParser.parse(slrTable, lr0.augmented, ts);
            accepted = res.accepted;
            errors   = res.errors;
            trace    = res.trace;
            tree     = res.tree;
        } else if (useLALR) {
            TokenStream ts(filtered);
            auto res = lrParser.parse(lalrTable.action, lalrTable.goto_table,
                                       lr0.augmented, ts);
            accepted = res.accepted;
            errors   = res.errors;
            trace    = res.trace;
            tree     = res.tree;
        } else {
            TokenStream ts(filtered);
            auto res = ll1Parser.parse(ll1Table, grammarLL1, ts);
            accepted = res.accepted;
            errors   = res.errors;
            trace    = res.trace;
            tree     = res.tree;
        }

        out += ",\"tokens\":" + tokensToJson(tokens);
        out += ",\"result\":{\"accepted\":" + jbool(accepted)
            + ",\"errors\":" + jstrArr(errors)
            + ",\"trace\":"  + traceToJson(trace)
            + ",\"tree\":"   + treeToJson(tree) + "}";
    } else {
        out += ",\"tokens\":null,\"result\":null";
    }

    out += "}";

    // Restaurar stdout y emitir JSON
    std::cout.rdbuf(origCout);
    std::cout << out << std::endl;
    return 0;
}
