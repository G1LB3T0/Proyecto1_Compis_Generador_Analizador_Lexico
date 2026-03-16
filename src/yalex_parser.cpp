#include "yalex_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <iostream>

// ─── Utilitarios ────────────────────────────

std::string YALexParser::read_file(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open())
        throw std::runtime_error("[Parser] No se puede abrir archivo: " + filename);
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
}

void YALexParser::remove_comments(std::string& content) {
    size_t p = 0;
    while ((p = content.find("(*", p)) != std::string::npos) {
        size_t end = content.find("*)", p + 2);
        if (end == std::string::npos) { content = content.substr(0, p); break; }
        std::string repl(end - p + 2, ' ');
        for (size_t i = p; i < end + 2; i++)
            if (content[i] == '\n') repl[i - p] = '\n';
        content.replace(p, end - p + 2, repl);
        p++;
    }
}

std::string YALexParser::trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

bool YALexParser::starts_with(const std::string& s, size_t pos, const std::string& prefix) {
    return s.compare(pos, prefix.size(), prefix) == 0;
}

void YALexParser::skip_whitespace(const std::string& c, size_t& p) {
    while (p < c.size() && isspace((unsigned char)c[p])) p++;
}

std::string YALexParser::extract_braced(const std::string& content, size_t& pos) {
    if (pos >= content.size() || content[pos] != '{')
        throw std::runtime_error("[Parser] Se esperaba '{'");
    int depth = 1;
    size_t start = pos + 1;
    pos++;
    while (pos < content.size() && depth > 0) {
        char c = content[pos];
        if (c == '\'' || c == '"') {
            char quote = c; pos++;
            while (pos < content.size() && content[pos] != quote) {
                if (content[pos] == '\\') pos++;
                pos++;
            }
        } else if (c == '{') depth++;
        else if (c == '}') depth--;
        pos++;
    }
    return content.substr(start, pos - start - 1);
}

std::string YALexParser::expand_defs(const std::string& regex,
                                     const std::map<std::string, std::string>& defs,
                                     int depth) {
    if (depth > 50)
        throw std::runtime_error("[Parser] Posible ciclo en macros");
    std::string result = regex;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& [name, value] : defs) {
            size_t p = 0;
            while ((p = result.find(name, p)) != std::string::npos) {
                bool lo = (p == 0) || (!isalnum((unsigned char)result[p-1]) && result[p-1] != '_');
                bool ro = (p + name.size() >= result.size()) ||
                          (!isalnum((unsigned char)result[p + name.size()]) && result[p + name.size()] != '_');
                if (lo && ro) {
                    std::string rep = "(" + value + ")";
                    result.replace(p, name.size(), rep);
                    p += rep.size(); changed = true;
                } else p += name.size();
            }
        }
    }
    return result;
}

std::string YALexParser::read_regex(const std::string& content, size_t& pos) {
    std::string result;
    while (pos < content.size()) {
        char c = content[pos];
        if (c == '{') break;
        if (c == '\n') {
            size_t la = pos + 1;
            while (la < content.size() && (content[la]==' '||content[la]=='\t')) la++;
            if (la < content.size() && content[la] == '{') { pos++; continue; }
            break;
        }
        if (c == '\'') {
            result += c; pos++;
            if (pos < content.size() && content[pos] == '\\') result += content[pos++];
            if (pos < content.size()) result += content[pos++];
            if (pos < content.size() && content[pos] == '\'') result += content[pos++];
        } else if (c == '"') {
            result += c; pos++;
            while (pos < content.size() && content[pos] != '"') {
                if (content[pos] == '\\') result += content[pos++];
                result += content[pos++];
            }
            if (pos < content.size()) result += content[pos++];
        } else if (c == '[') {
            result += c; pos++;
            int depth = 1;
            while (pos < content.size() && depth > 0) {
                char cc = content[pos];
                if (cc == '[') depth++;
                else if (cc == ']') depth--;
                if (depth > 0 || cc == ']') {
                    if (cc == '\'') {
                        result += cc; pos++;
                        if (pos < content.size() && content[pos] == '\\') result += content[pos++];
                        if (pos < content.size()) result += content[pos++];
                        if (pos < content.size() && content[pos] == '\'') result += content[pos++];
                        continue;
                    }
                    result += cc; pos++;
                } else { result += cc; pos++; }
            }
        } else { result += c; pos++; }
    }
    return trim(result);
}

// ─── Parser principal ────────────────────────

YALexSpec YALexParser::parse(const std::string& filename) {
    std::string content = read_file(filename);
    remove_comments(content);

    YALexSpec spec;
    size_t pos = 0;
    skip_whitespace(content, pos);

    if (pos < content.size() && content[pos] == '{') {
        spec.header = extract_braced(content, pos);
        skip_whitespace(content, pos);
    }

    while (pos < content.size()) {
        skip_whitespace(content, pos);
        if (pos >= content.size()) break;
        if (content[pos] == '{') { spec.trailer = extract_braced(content, pos); break; }
        if (!isalpha((unsigned char)content[pos]) && content[pos] != '_') { pos++; continue; }

        size_t ks = pos;
        while (pos < content.size() && (isalnum((unsigned char)content[pos]) || content[pos] == '_')) pos++;
        std::string keyword = content.substr(ks, pos - ks);

        if (keyword == "let") {
            skip_whitespace(content, pos);
            size_t ids = pos;
            while (pos < content.size() && (isalnum((unsigned char)content[pos]) || content[pos] == '_')) pos++;
            std::string ident = content.substr(ids, pos - ids);
            skip_whitespace(content, pos);
            if (pos < content.size() && content[pos] == '=') pos++;
            skip_whitespace(content, pos);
            size_t le = content.find('\n', pos);
            if (le == std::string::npos) le = content.size();
            std::string val = trim(content.substr(pos, le - pos));
            pos = le;
            spec.defs[ident] = val;

        } else if (keyword == "rule") {
            skip_whitespace(content, pos);
            size_t eps = pos;
            while (pos < content.size() && (isalnum((unsigned char)content[pos]) || content[pos] == '_')) pos++;
            spec.entrypoint = content.substr(eps, pos - eps);
            while (pos < content.size() && content[pos] != '=') pos++;
            if (pos < content.size()) pos++;

            int rule_count = 0;
            while (pos < content.size()) {
                skip_whitespace(content, pos);
                if (pos >= content.size()) break;
                if (content[pos] == '{') break;
                if (isalpha((unsigned char)content[pos])) {
                    size_t save = pos, ks2 = pos;
                    while (pos < content.size() && (isalnum((unsigned char)content[pos]) || content[pos] == '_')) pos++;
                    std::string kw2 = content.substr(ks2, pos - ks2);
                    if (kw2 == "let" || kw2 == "rule") { pos = save; break; }
                    pos = save;
                }
                if (content[pos] == '|') pos++;
                skip_whitespace(content, pos);
                if (pos >= content.size()) break;
                std::string rx = read_regex(content, pos);
                if (rx.empty()) continue;
                skip_whitespace(content, pos);
                std::string action;
                if (pos < content.size() && content[pos] == '{')
                    action = trim(extract_braced(content, pos));
                if (!rx.empty()) {
                    LexRule rule;
                    rule.original_regex = rx;
                    rule.expanded_regex = expand_defs(rx, spec.defs);
                    rule.action = action;
                    rule.priority = rule_count++;
                    spec.rules.push_back(rule);
                }
            }
        }
    }

    if (spec.rules.empty())
        throw std::runtime_error("[Parser] No se encontraron reglas en el archivo .yal");

    // ── yalex_parser.cpp devuelve: YALexSpec ────────────────────────────
    std::cout << "\n[yalex_parser] Resultado:\n";
    std::cout << "  Tabla de simbolos (" << spec.defs.size() << " macros):\n";
    for (const auto& [name, val] : spec.defs)
        std::cout << "    let " << std::setw(10) << std::left << name << " = " << val << "\n";

    std::cout << "  Reglas (" << spec.rules.size() << " total):\n";
    std::cout << "  " << std::setw(3) << "#"
              << "  " << std::setw(28) << std::left << "patron"
              << "  accion\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    for (const auto& r : spec.rules) {
        std::string rx = r.original_regex;
        if (rx.size() > 26) rx = rx.substr(0,23) + "...";
        std::string ac = r.action;
        if (ac.size() > 28) ac = ac.substr(0,25) + "...";
        std::cout << "  " << std::setw(3) << r.priority
                  << "  " << std::setw(28) << std::left << rx
                  << "  " << ac << "\n";
    }
    std::cout << "  Punto de entrada: " << spec.entrypoint << "\n\n";

    return spec;
}
