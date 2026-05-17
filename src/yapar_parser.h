#pragma once
#include "grammar.h"
#include <string>

// ============================================================
//  YaparParser — Módulo que lee el archivo .yalp
//
//  Hace lo mismo que YalexParser pero para gramáticas:
//    1. Lee el .yalp completo
//    2. Extrae los %token declarados
//    3. Extrae los IGNORE
//    4. Salta el %%
//    5. Parsea cada producción y sus alternativas
//    6. Devuelve un Grammar listo para FirstFollow
//
//  El símbolo inicial es la PRIMERA producción definida.
// ============================================================
class YaparParser {
public:
    // Lee el .yalp y devuelve la Grammar completa
    Grammar parse(const std::string& filename);

private:
    // Lee el archivo completo como string
    std::string readFile(const std::string& filename) const;

    // Elimina comentarios /* ... */
    std::string removeComments(const std::string& src) const;

    // Utilidades
    std::string trim(const std::string& s) const;
    void skipWhitespace(const std::string& s, size_t& pos) const;
    std::string readWord(const std::string& s, size_t& pos) const;
};