#include "yapar_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>

std::string YaparParser::readFile(const std::string& filename) const {
    std::ifstream f(filename);
    if (!f.is_open())
        throw std::runtime_error("[YaparParser] No se puede abrir: " + filename);
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
}

std::string YaparParser::removeComments(const std::string& src) const {
    std::string result;
    size_t i = 0;
    while (i < src.size()) {
        if (i + 1 < src.size() && src[i] == '/' && src[i+1] == '*') {
            i += 2;
            while (i + 1 < src.size()) {
                if (src[i] == '*' && src[i+1] == '/') { i += 2; break; }
                if (src[i] == '\n') result += '\n';
                i++;
            }
        } else {
            result += src[i++];
        }
    }
    return result;
}

std::string YaparParser::trim(const std::string& s) const {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

void YaparParser::skipWhitespace(const std::string& s, size_t& pos) const {
    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
}

std::string YaparParser::readWord(const std::string& s, size_t& pos) const {
    skipWhitespace(s, pos);
    size_t start = pos;
    while (pos < s.size() &&
           (std::isalnum((unsigned char)s[pos]) || s[pos] == '_'))
        pos++;
    return s.substr(start, pos - start);
}

Grammar YaparParser::parse(const std::string& filename) {
    std::string src = readFile(filename);
    src = removeComments(src);
    Grammar g;
    size_t pos = 0;

    // ── Sección TOKENS ────────────────────────────────────
    while (pos < src.size()) {
        skipWhitespace(src, pos);
        if (pos >= src.size()) break;
        if (pos + 1 < src.size() &&
            src[pos] == '%' && src[pos+1] == '%') {
            pos += 2; break;
        }
        if (src[pos] == '%') {
            pos++;
            std::string kw = readWord(src, pos);
            if (kw == "token") {
                size_t lineEnd = src.find('\n', pos);
                if (lineEnd == std::string::npos) lineEnd = src.size();
                std::string line = src.substr(pos, lineEnd - pos);
                pos = lineEnd;
                std::istringstream ss(line);
                std::string tok;
                while (ss >> tok)
                    if (!tok.empty()) g.terminals.push_back(tok);
            }
        } else {
            std::string kw = readWord(src, pos);
            if (kw == "IGNORE") {
                std::string tok = readWord(src, pos);
                if (!tok.empty()) g.ignored.insert(tok);
            } else {
                size_t lineEnd = src.find('\n', pos);
                if (lineEnd == std::string::npos) lineEnd = src.size();
                pos = lineEnd;
            }
        }
    }

    // helpers
    auto isTerm = [&](const std::string& s) {
        for (const auto& t : g.terminals) if (t == s) return true;
        return false;
    };
    auto addNT = [&](const std::string& s) {
        if (!isTerm(s) &&
            std::find(g.non_terminals.begin(),
                      g.non_terminals.end(), s) == g.non_terminals.end())
            g.non_terminals.push_back(s);
    };

    // ── Sección PRODUCCIONES ──────────────────────────────
    // Estrategia: divide el texto por ';' para obtener cada
    // grupo de producción, luego dentro divide por '|' para
    // obtener cada alternativa.
    std::string prodSection = src.substr(pos);

    // Divide por ';'
    std::vector<std::string> groups;
    {
        std::string cur;
        for (char c : prodSection) {
            if (c == ';') {
                groups.push_back(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        if (!trim(cur).empty()) groups.push_back(cur);
    }

    for (const auto& group : groups) {
        std::string g_trim = trim(group);
        if (g_trim.empty()) continue;

        // Primera línea antes del primer '|' o '\n' es el nombre + ':'
        size_t colon = g_trim.find(':');
        if (colon == std::string::npos) continue;

        std::string head = trim(g_trim.substr(0, colon));
        if (head.empty()) continue;

        addNT(head);
        if (g.start_symbol.empty()) g.start_symbol = head;

        // El resto son las alternativas separadas por '|'
        std::string rest = g_trim.substr(colon + 1);

        // Divide por '|' — pero solo los que están al nivel superior
        std::vector<std::string> alts;
        {
            std::string cur;
            for (char c : rest) {
                if (c == '|') {
                    alts.push_back(cur);
                    cur.clear();
                } else {
                    cur += c;
                }
            }
            alts.push_back(cur); // última alternativa
        }

        for (const auto& alt : alts) {
            std::string a = trim(alt);

            Production prod;
            prod.head = head;

            if (a.empty()) {
                // ── PRODUCCIÓN VACÍA ε ──────────────────
                // Una alternativa vacía después de | significa ε
                prod.body = { "" };
                g.productions.push_back(prod);
                continue;
            }

            // Lee los símbolos del cuerpo
            std::istringstream ss(a);
            std::string sym;
            while (ss >> sym) {
                prod.body.push_back(sym);
                if (!isTerm(sym)) addNT(sym);
            }

            if (!prod.body.empty())
                g.productions.push_back(prod);
        }
    }

    if (g.productions.empty())
        throw std::runtime_error("[YaparParser] No se encontraron producciones");
    if (g.start_symbol.empty())
        g.start_symbol = g.non_terminals[0];

    std::cout << "[yapar_parser] parse(\"" << filename << "\") OK\n";
    std::cout << "  " << g.terminals.size()     << " terminales\n";
    std::cout << "  " << g.non_terminals.size() << " no terminales\n";
    std::cout << "  " << g.productions.size()   << " producciones\n";
    std::cout << "  Símbolo inicial: " << g.start_symbol << "\n\n";
    return g;
}
