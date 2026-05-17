#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

// ============================================================
//  Token
//  Representa un token reconocido por el lexer.
//  Es la unidad básica que el parser va a consumir.
//
//  Ejemplo: para la entrada "int x = 10;"
//    Token{ "KW_INT",    "int", linea=1, col=1 }
//    Token{ "ID",        "x",   linea=1, col=5 }
//    Token{ "OP_ASSIGN", "=",   linea=1, col=7 }
//    Token{ "INT_LIT",   "10",  linea=1, col=9 }
//    Token{ "SEMICOLON", ";",   linea=1, col=11}
// ============================================================
struct Token {
    std::string tipo;    // nombre del token: "KW_INT", "ID"...
    std::string lexema;  // texto real capturado del input
    int         linea;
    int         columna;

    Token(const std::string& t, const std::string& l,
          int lin = 0, int col = 0)
        : tipo(t), lexema(l), linea(lin), columna(col) {}
};

// ============================================================
//  TokenStream
//  Lista de tokens que el parser consume de izquierda
//  a derecha. Tiene tres operaciones clave:
// ============================================================
class TokenStream {
public:
    explicit TokenStream(const std::vector<Token>& tokens)
        : tokens_(tokens), pos_(0) {}

    // Mira el token actual sin consumirlo
    // el parser usa para decidir qué producción aplicar
    const Token& lookahead() const {
        if (pos_ < tokens_.size()) return tokens_[pos_];
        static Token eof("$", "$");
        return eof;
    }

    // Toma el token actual y avanza al siguiente
    Token consume() {
        if (pos_ < tokens_.size()) return tokens_[pos_++];
        return Token("$", "$");
    }

    // Consume y valida — lanza error si el tipo no coincide
    Token expect(const std::string& tipo_esperado) {
        Token t = consume();
        if (t.tipo != tipo_esperado)
            throw std::runtime_error(
                "Error sintáctico en línea " + std::to_string(t.linea) +
                ", col " + std::to_string(t.columna) +
                ": se esperaba '" + tipo_esperado +
                "' pero se encontró '" + t.tipo +
                "' (\"" + t.lexema + "\")");
        return t;
    }

    bool atEnd() const { return pos_ >= tokens_.size(); }
    int  pos()   const { return (int)pos_; }
    int  size()  const { return (int)tokens_.size(); }

    // Imprime todos los tokens — para debug y evaluación
    void print() const {
        std::cout << "\n========== TOKEN STREAM ==========\n";
        for (size_t i = 0; i < tokens_.size(); i++) {
            std::cout << "[" << i << "] "
                      << "<" << tokens_[i].tipo << ", \""
                      << tokens_[i].lexema << "\">"
                      << "  linea=" << tokens_[i].linea
                      << " col=" << tokens_[i].columna << "\n";
        }
        std::cout << "[" << tokens_.size() << "] <$, \"$\">  EOF\n";
        std::cout << "Total: " << tokens_.size() << " tokens\n";
        std::cout << "==================================\n\n";
    }

private:
    std::vector<Token> tokens_;
    size_t             pos_;
};