#include "regex_parser.h"
#include <stdexcept>
#include <cctype>
#include <iostream>

// ─── Utilidades de tokenización ──────────────

unsigned char RegexParser::parseEscape(const std::string& s, size_t& p) {
    // p apunta al carácter después de '\'
    if (p >= s.size()) return '\\';
    char c = s[p++];
    switch (c) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"':  return '"';
        case '0':  return '\0';
        case ' ':  return ' ';
        default:   return (unsigned char)c;
    }
}

// Parsea 'c' o '\n' y devuelve su charset (un solo bit)
std::bitset<256> RegexParser::parseQuotedChar(const std::string& s, size_t& p) {
    // p apunta al '\'' inicial
    if (p >= s.size() || s[p] != '\'')
        throw std::runtime_error("[RegexParser] Se esperaba '\\'");
    p++; // consumir '

    std::bitset<256> cs;
    unsigned char ch;
    if (p < s.size() && s[p] == '\\') {
        p++; // consumir backslash
        ch = parseEscape(s, p);
    } else if (p < s.size()) {
        ch = (unsigned char)s[p++];
    } else {
        throw std::runtime_error("[RegexParser] Carácter vacío");
    }
    cs.set(ch);

    if (p < s.size() && s[p] == '\'') p++; // consumir '
    return cs;
}

// Parsea [...] y devuelve su charset
// Formato: ['a' 'b' 'c'-'z' '\n']  o [^ ...]
std::bitset<256> RegexParser::parseCharClass(const std::string& s, size_t& p) {
    // p apunta al '[' inicial
    if (p >= s.size() || s[p] != '[')
        throw std::runtime_error("[RegexParser] Se esperaba '['");
    p++; // consumir [

    bool negated = false;
    if (p < s.size() && s[p] == '^') {
        negated = true;
        p++;
    }

    std::bitset<256> cs;
    while (p < s.size() && s[p] != ']') {
        // Saltar espacios dentro del charset
        if (s[p] == ' ' || s[p] == '\t') { p++; continue; }

        if (s[p] == '\'') {
            // Carácter quoted: 'x' o '\n'
            std::bitset<256> c1 = parseQuotedChar(s, p);

            // Posible rango: 'a'-'z'
            size_t save = p;
            while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) p++;
            if (p < s.size() && s[p] == '-') {
                p++; // consumir '-'
                while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) p++;
                if (p < s.size() && s[p] == '\'') {
                    std::bitset<256> c2 = parseQuotedChar(s, p);
                    // Encontrar el carácter único en c1 y c2
                    int from = -1, to = -1;
                    for (int i = 0; i < 256; i++) if (c1.test(i)) { from = i; break; }
                    for (int i = 0; i < 256; i++) if (c2.test(i)) { to   = i; break; }
                    if (from >= 0 && to >= 0 && from <= to) {
                        for (int i = from; i <= to; i++) cs.set(i);
                    }
                } else {
                    cs |= c1;
                    p = save; // restaurar si no hay rango
                }
            } else {
                cs |= c1;
                p = save;
            }
        } else {
            // Carácter literal sin comillas dentro del charset
            cs.set((unsigned char)s[p++]);
        }
    }
    if (p < s.size()) p++; // consumir ']'

    if (negated) cs.flip(); // complemento

    return cs;
}

// ─── Tokenizador ─────────────────────────────

static void skip_ws(const std::string& s, size_t& p) {
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) p++;
}

std::vector<RegexToken> RegexParser::tokenize(const std::string& s) {
    std::vector<RegexToken> result;
    size_t p = 0;

    while (p < s.size()) {
        skip_ws(s, p);
        if (p >= s.size()) break;

        char c = s[p];

        if (c == '\'') {
            // Carácter literal 'x'
            auto cs = parseQuotedChar(s, p);
            std::string lbl = "?";
            for (int i = 0; i < 256; i++) if (cs.test(i)) { lbl = std::string(1, (char)i); break; }
            result.push_back(RegexToken(cs, "'" + lbl + "'"));

        } else if (c == '"') {
            // String literal "abc" → tokenizar como chars separados
            p++; // consumir "
            std::vector<RegexToken> chars;
            while (p < s.size() && s[p] != '"') {
                std::bitset<256> cs;
                if (s[p] == '\\') {
                    p++;
                    cs.set(parseEscape(s, p));
                } else {
                    cs.set((unsigned char)s[p++]);
                }
                char ch = '?'; for (int i=0;i<256;i++){if(cs.test(i)){ch=(char)i;break;}}
                chars.push_back(RegexToken(cs, std::string(1, ch)));
            }
            if (p < s.size()) p++; // consumir "

            // Envolver en paréntesis si hay más de un char
            if (chars.size() > 1) {
                result.push_back(RegexToken(RegexToken::LPAR));
                for (auto& t : chars) result.push_back(t);
                result.push_back(RegexToken(RegexToken::RPAR));
            } else {
                for (auto& t : chars) result.push_back(t);
            }

        } else if (c == '[') {
            // Clase de caracteres
            auto cs = parseCharClass(s, p);
            result.push_back(RegexToken(cs, "[class]"));

        } else if (c == '_') {
            // Cualquier carácter
            p++;
            std::bitset<256> cs;
            cs.set(); // todos los bits
            result.push_back(RegexToken(cs, "_"));

        } else if (c == 'e' && s.compare(p, 3, "eof") == 0) {
            // eof → carácter 0 (NUL como sentinel de fin)
            p += 3;
            std::bitset<256> cs;
            cs.set(0);
            result.push_back(RegexToken(cs, "eof"));

        } else if (c == '(') {
            p++;
            result.push_back(RegexToken(RegexToken::LPAR));
        } else if (c == ')') {
            p++;
            result.push_back(RegexToken(RegexToken::RPAR));
        } else if (c == '*') {
            p++;
            result.push_back(RegexToken(RegexToken::STAR));
        } else if (c == '+') {
            p++;
            result.push_back(RegexToken(RegexToken::PLUS));
        } else if (c == '?') {
            p++;
            result.push_back(RegexToken(RegexToken::QUEST));
        } else if (c == '|') {
            p++;
            result.push_back(RegexToken(RegexToken::ALTER));
        } else if (c == '#') {
            p++;
            result.push_back(RegexToken(RegexToken::DIFF));
        } else if (isalpha(c) || c == '_') {
            // Identificador (macro no expandida) — tratar como literal de cada char
            // (en este punto los macros ya deberían estar expandidos)
            std::bitset<256> cs;
            cs.set((unsigned char)c);
            result.push_back(RegexToken(cs, std::string(1, c)));
            p++;
        } else {
            // Cualquier otro carácter → literal
            std::bitset<256> cs;
            cs.set((unsigned char)c);
            result.push_back(RegexToken(cs, std::string(1, c)));
            p++;
        }
    }

    result.push_back(RegexToken(RegexToken::END));
    return result;
}

// ─── Descenso recursivo ──────────────────────

RegexToken& RegexParser::peek() {
    return tokens[tok_pos];
}

RegexToken RegexParser::consume() {
    return tokens[tok_pos++];
}

// Determina si el token actual puede comenzar un átomo
static bool canStartAtom(const RegexToken& t) {
    return t.type == RegexToken::CHARSET ||
           t.type == RegexToken::LPAR;
}

// expr → concat ('|' concat)*
RegexNode* RegexParser::parseExpr() {
    RegexNode* left = parseConcat();
    while (peek().type == RegexToken::ALTER) {
        consume();
        RegexNode* right = parseConcat();
        left = makeAlter(left, right);
    }
    return left;
}

// concat → diff (diff)*   (concatenación implícita)
RegexNode* RegexParser::parseConcat() {
    RegexNode* left = parseDiff();
    while (canStartAtom(peek()) || peek().type == RegexToken::LPAR) {
        RegexNode* right = parseDiff();
        left = makeConcat(left, right);
    }
    return left;
}

// diff → postfix ('#' postfix)*
RegexNode* RegexParser::parseDiff() {
    RegexNode* left = parsePostfix();
    while (peek().type == RegexToken::DIFF) {
        consume();
        RegexNode* right = parsePostfix();
        left = makeDiff(left, right);
    }
    return left;
}

// postfix → atom ('*' | '+' | '?')*
RegexNode* RegexParser::parsePostfix() {
    RegexNode* node = parseAtom();
    while (true) {
        if (peek().type == RegexToken::STAR) {
            consume();
            node = makeStar(node);
        } else if (peek().type == RegexToken::PLUS) {
            consume();
            node = makePlus(node);
        } else if (peek().type == RegexToken::QUEST) {
            consume();
            node = makeQuest(node);
        } else {
            break;
        }
    }
    return node;
}

// atom → '(' expr ')' | CHARSET
RegexNode* RegexParser::parseAtom() {
    if (peek().type == RegexToken::LPAR) {
        consume(); // (
        RegexNode* node = parseExpr();
        if (peek().type != RegexToken::RPAR)
            throw std::runtime_error("[RegexParser] Se esperaba ')'");
        consume(); // )
        return node;
    } else if (peek().type == RegexToken::CHARSET) {
        RegexToken t = consume();
        return makeLeaf(t.charset, pos_counter, t.label);
    } else {
        throw std::runtime_error("[RegexParser] Token inesperado: tipo=" +
                                  std::to_string((int)peek().type));
    }
}

// ─── Punto de entrada ─────────────────────────

RegexNode* RegexParser::parse(const std::string& regex_str) {
    tokens  = tokenize(regex_str);
    tok_pos = 0;

    if (tokens.empty() || tokens[0].type == RegexToken::END)
        throw std::runtime_error("[RegexParser] Expresión regular vacía");

    RegexNode* root = parseExpr();

    if (peek().type != RegexToken::END)
        throw std::runtime_error("[RegexParser] Tokens sobrantes después de parsear");

    // ── regex_parser.cpp devuelve: RegexNode* (raiz del arbol AST) ──
    // (firstpos se calcula despues en regex_node::computePositions)
    std::cout << "[regex_parser] parse(\"" << regex_str.substr(0, std::min((int)regex_str.size(),45))
              << (regex_str.size()>45?"...":"") << "\") -> arbol AST OK\n";

    return root;
}
