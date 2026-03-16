#include "automata_serializer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <sys/stat.h>   // para exists()

// ============================================================
//  UTILIDADES JSON MINIMALISTAS
//  No se usan librerías externas para no agregar dependencias.
//  Escribe/lee JSON manualmente — es sencillo porque es
//  una estructura es fija y predecible.
// ============================================================

// ── Escritura ─────────────────────────────────────────────

// Escapa un string para que sea JSON-safe
static std::string jsonEscape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if      (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else if (c == '\t') r += "\\t";
        else                r += c;
    }
    return r;
}

void AutomataSerializer::save(const DFA& dfa,
                               const YALexSpec& spec,
                               const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open())
        throw std::runtime_error("[Serializer] No se puede escribir: " + path);

    int N = (int)dfa.states.size();
    int R = (int)spec.rules.size();

    f << "{\n";

    // ── Metadata ──
    f << "  \"entrypoint\": \""  << jsonEscape(spec.entrypoint) << "\",\n";
    f << "  \"num_states\": "    << N << ",\n";
    f << "  \"start_state\": "   << dfa.start_state << ",\n";

    // ── Header / trailer del usuario ──
    f << "  \"header\": \""  << jsonEscape(spec.header)  << "\",\n";
    f << "  \"trailer\": \"" << jsonEscape(spec.trailer) << "\",\n";

    // ── Estados del AFD ──
    f << "  \"states\": [\n";
    for (int s = 0; s < N; s++) {
        const DFAState& st = dfa.states[s];
        f << "    {\n";
        f << "      \"id\": "             << st.id             << ",\n";
        f << "      \"is_accepting\": "   << (st.is_accepting ? "true" : "false") << ",\n";
        f << "      \"accepting_rule\": " << st.accepting_rule << ",\n";

        // transitions[256] como array JSON en una sola línea
        f << "      \"transitions\": [";
        for (int a = 0; a < 256; a++) {
            f << st.transitions[a];
            if (a < 255) f << ",";
        }
        f << "]\n";

        f << "    }";
        if (s < N - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // ── Reglas léxicas ──
    f << "  \"rules\": [\n";
    for (int i = 0; i < R; i++) {
        const LexRule& r = spec.rules[i];
        f << "    {\n";
        f << "      \"priority\": "        << r.priority                          << ",\n";
        f << "      \"original_regex\": \"" << jsonEscape(r.original_regex)       << "\",\n";
        f << "      \"expanded_regex\": \"" << jsonEscape(r.expanded_regex)       << "\",\n";
        f << "      \"action\": \""         << jsonEscape(r.action)               << "\"\n";
        f << "    }";
        if (i < R - 1) f << ",";
        f << "\n";
    }
    f << "  ]\n";

    f << "}\n";
    f.close();

    std::cout << "[serializer] save() -> " << path << "\n";
    std::cout << "  " << N << " estados, " << R << " reglas persistidas en disco.\n\n";
}

// ── Lectura ───────────────────────────────────────────────
//
// Parser JSON minimalista: busca claves conocidas con find()
// y extrae sus valores. Funciona porque el formato que
// escribimos en save() es siempre el mismo.

// Extrae el valor de "clave": VALUE del JSON
static std::string extractString(const std::string& json,
                                  const std::string& key) {
    std::string search = "\"" + key + "\": \"";
    size_t p = json.find(search);
    if (p == std::string::npos) return "";
    p += search.size();
    std::string result;
    while (p < json.size() && json[p] != '"') {
        if (json[p] == '\\' && p + 1 < json.size()) {
            char esc = json[++p];
            if      (esc == 'n')  result += '\n';
            else if (esc == 't')  result += '\t';
            else if (esc == 'r')  result += '\r';
            else if (esc == '"')  result += '"';
            else if (esc == '\\') result += '\\';
            else                  result += esc;
        } else {
            result += json[p];
        }
        p++;
    }
    return result;
}

static int extractInt(const std::string& json,
                       const std::string& key) {
    std::string search = "\"" + key + "\": ";
    size_t p = json.find(search);
    if (p == std::string::npos)
        throw std::runtime_error("[Serializer] Clave no encontrada: " + key);
    p += search.size();
    // leer número (puede ser negativo)
    bool neg = false;
    if (p < json.size() && json[p] == '-') { neg = true; p++; }
    int val = 0;
    while (p < json.size() && std::isdigit((unsigned char)json[p]))
        val = val * 10 + (json[p++] - '0');
    return neg ? -val : val;
}

static bool extractBool(const std::string& json,
                          const std::string& key) {
    std::string search = "\"" + key + "\": ";
    size_t p = json.find(search);
    if (p == std::string::npos) return false;
    p += search.size();
    return json.compare(p, 4, "true") == 0;
}

// Extrae "transitions": [n0,n1,...,n255]
static std::array<int,256> extractTransitions(const std::string& block) {
    std::array<int,256> t;
    t.fill(-1);
    size_t p = block.find("\"transitions\": [");
    if (p == std::string::npos) return t;
    p += 17; // salta hasta el primer número
    for (int a = 0; a < 256 && p < block.size(); a++) {
        // saltar espacios
        while (p < block.size() && !std::isdigit((unsigned char)block[p]) && block[p] != '-') p++;
        bool neg = false;
        if (p < block.size() && block[p] == '-') { neg = true; p++; }
        int val = 0;
        while (p < block.size() && std::isdigit((unsigned char)block[p]))
            val = val * 10 + (block[p++] - '0');
        t[a] = neg ? -val : val;
        // saltar la coma
        while (p < block.size() && block[p] != ',' && block[p] != ']') p++;
        if (p < block.size() && block[p] == ',') p++;
    }
    return t;
}

void AutomataSerializer::load(const std::string& path,
                               DFA& out_dfa,
                               YALexSpec& out_spec) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("[Serializer] No se puede leer: " + path);

    // Leer todo el JSON a un string
    std::string json((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
    f.close();

    // ── Metadata ──
    out_spec.entrypoint  = extractString(json, "entrypoint");
    out_spec.header      = extractString(json, "header");
    out_spec.trailer     = extractString(json, "trailer");
    out_dfa.start_state  = extractInt(json, "start_state");
    int N                = extractInt(json, "num_states");

    // ── Estados ──
    out_dfa.states.clear();
    out_dfa.states.resize(N);

    // Divide el JSON en bloques de estado usando "\"id\":"
    // como separador (sabemos que cada estado tiene un "id")
    size_t search_pos = 0;
    int state_idx = 0;
    while (state_idx < N) {
        size_t block_start = json.find("\"id\":", search_pos);
        if (block_start == std::string::npos) break;

        // El bloque del estado va desde "id" hasta el siguiente "}" de cierre
        size_t block_end = json.find("\n    }", block_start);
        if (block_end == std::string::npos) block_end = json.size();
        std::string block = json.substr(block_start, block_end - block_start + 6);

        DFAState& st = out_dfa.states[state_idx];
        st.id             = extractInt(block,  "id");
        st.is_accepting   = extractBool(block, "is_accepting");
        st.accepting_rule = extractInt(block,  "accepting_rule");
        st.transitions    = extractTransitions(block);

        search_pos = block_end + 6;
        state_idx++;
    }

    // ── Reglas ──
    out_spec.rules.clear();
    search_pos = 0;
    while (true) {
        size_t block_start = json.find("\"priority\":", search_pos);
        if (block_start == std::string::npos) break;

        size_t block_end = json.find("\n    }", block_start);
        if (block_end == std::string::npos) break;
        std::string block = json.substr(block_start, block_end - block_start + 6);

        LexRule rule;
        rule.priority        = extractInt(block,    "priority");
        rule.original_regex  = extractString(block, "original_regex");
        rule.expanded_regex  = extractString(block, "expanded_regex");
        rule.action          = extractString(block, "action");
        out_spec.rules.push_back(rule);

        search_pos = block_end + 6;
    }

    std::cout << "[serializer] load() <- " << path << "\n";
    std::cout << "  " << N << " estados, "
              << out_spec.rules.size() << " reglas cargadas desde disco.\n";
    std::cout << "  -> dfa_builder OMITIDO (ya persistido)\n\n";
}

bool AutomataSerializer::exists(const std::string& path) const {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
