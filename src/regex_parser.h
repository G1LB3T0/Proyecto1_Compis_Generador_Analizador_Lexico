#pragma once
#include "regex_node.h"
#include <string>
#include <vector>
#include <bitset>

// ─────────────────────────────────────────────
// Módulo 2 – Parser de Expresiones Regulares
//
// Convierte una cadena de texto YALex en un AST.
// Usa descenso recursivo con la siguiente gramática:
//
//   expr    → concat ('|' concat)*          (menor precedencia)
//   concat  → diff (diff)*
//   diff    → postfix ('#' postfix)*
//   postfix → atom ('*' | '+' | '?')*
//   atom    → '(' expr ')' | terminal
//
//   terminal:
//     'c'         literal carácter
//     '\n' etc.   secuencias de escape
//     "string"    cadena literal (expande a concatenación)
//     [set]       clase de caracteres
//     [^set]      clase de caracteres negada
//     _           cualquier carácter
//     eof         fin de entrada
//     ident       macro ya expandida
// ─────────────────────────────────────────────

struct RegexToken {
    enum Type {
        CHARSET,  // hoja con un conjunto de caracteres
        LPAR,     // (
        RPAR,     // )
        STAR,     // *
        PLUS,     // +
        QUEST,    // ?
        ALTER,    // |
        DIFF,     // #
        END       // fin de tokens
    };
    Type type;
    std::bitset<256> charset;
    std::string label;

    RegexToken(Type t) : type(t) {}
    RegexToken(const std::bitset<256>& cs, const std::string& lbl)
        : type(CHARSET), charset(cs), label(lbl) {}
};

class RegexParser {
public:
    explicit RegexParser(int& pos_counter) : pos_counter(pos_counter) {}

    // Convierte una string de regex YALex en un árbol AST
    RegexNode* parse(const std::string& regex_str);

private:
    int& pos_counter;

    // ── Tokenizador ──
    std::vector<RegexToken> tokens;
    size_t tok_pos = 0;

    std::vector<RegexToken> tokenize(const std::string& s);
    unsigned char parseEscape(const std::string& s, size_t& p);
    std::bitset<256> parseQuotedChar(const std::string& s, size_t& p);
    std::bitset<256> parseCharClass(const std::string& s, size_t& p);

    // ── Descenso recursivo ──
    RegexToken& peek();
    RegexToken  consume();

    RegexNode* parseExpr();
    RegexNode* parseConcat();
    RegexNode* parseDiff();
    RegexNode* parsePostfix();
    RegexNode* parseAtom();
};
