#include "code_gen.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <iostream>

// ─── Escape de strings para C++ ──────────────

static std::string cppEscape(const std::string& s) {
    std::string r;
    for (unsigned char c : s) {
        if (c == '"')  r += "\\\"";
        else if (c == '\\') r += "\\\\";
        else if (c == '\n') r += "\\n";
        else if (c == '\t') r += "\\t";
        else if (c == '\r') r += "\\r";
        else if (c < 32 || c >= 127)
            r += "\\x" + std::to_string((int)c);
        else r += (char)c;
    }
    return r;
}

// ─── Generación del archivo C++ ──────────────

void CodeGenerator::generate(const DFA& dfa,
                               const YALexSpec& spec,
                               const std::string& output_base) {

    std::string out_path = output_base + ".cpp";
    std::ofstream f(out_path);
    if (!f.is_open())
        throw std::runtime_error("[CodeGen] No se puede crear: " + out_path);

    int N = (int)dfa.states.size();
    int R = (int)spec.rules.size();

    // ── Cabecera del archivo generado ──
    f << "// =====================================================\n"
      << "// Analizador Léxico generado automáticamente\n"
      << "// Punto de entrada: " << spec.entrypoint << "\n"
      << "// Reglas: " << R << "  |  Estados AFD: " << N << "\n"
      << "// =====================================================\n\n";

    f << "#include <iostream>\n"
      << "#include <fstream>\n"
      << "#include <string>\n"
      << "#include <vector>\n"
      << "#include <stdexcept>\n\n";

    // ── Header del usuario ──
    if (!spec.header.empty()) {
        f << "// --- Header del usuario ---\n"
          << spec.header << "\n"
          << "// --- Fin header ---\n\n";
    }

    // ── Tabla de transiciones ──
    // transitions[estado][char] = estado_destino (-1 = error)
    f << "// Tabla de transiciones del AFD minimizado\n"
      << "// transitions[estado][caracter] = estado_destino  (-1 = sin transición)\n"
      << "static const int NUM_STATES = " << N << ";\n"
      << "static const int START_STATE = " << dfa.start_state << ";\n\n"
      << "static const int transitions[" << N << "][256] = {\n";

    for (int s = 0; s < N; s++) {
        f << "    {";
        for (int a = 0; a < 256; a++) {
            f << std::setw(3) << dfa.states[s].transitions[a];
            if (a < 255) f << ",";
        }
        f << "}";
        if (s < N - 1) f << ",";
        f << "  // Estado " << s;
        if (dfa.states[s].is_accepting)
            f << " [acepta regla " << dfa.states[s].accepting_rule << "]";
        f << "\n";
    }
    f << "};\n\n";

    // ── Tabla de reglas de aceptación ──
    f << "// accepting_rule[estado] = índice de regla (-1 si no acepta)\n"
      << "static const int accepting_rule[" << N << "] = { ";
    for (int s = 0; s < N; s++) {
        f << (dfa.states[s].is_accepting ? dfa.states[s].accepting_rule : -1);
        if (s < N - 1) f << ", ";
    }
    f << " };\n\n";

    // ── Nombres de tokens ──
    f << "// Nombres de los tokens (para reportes)\n"
      << "static const char* token_names[" << R << "] = {\n";
    for (int i = 0; i < R; i++) {
        f << "    \"TOKEN_" << i << "\"";
        if (i < R - 1) f << ",";
        f << "  // " << cppEscape(spec.rules[i].original_regex) << "\n";
    }
    f << "};\n\n";

    // ── Clase Lexer ──
    f << "// ─────────────────────────────────────────────\n"
      << "// Clase principal del analizador léxico\n"
      << "// ─────────────────────────────────────────────\n"
      << "class Lexer {\n"
      << "public:\n"
      << "    std::string input;      // buffer de entrada\n"
      << "    size_t lexemeBegin;     // inicio del lexema actual (Dragon Book: lexemeBegin)\n"
      << "    int    line;            // línea actual (para reporte de errores)\n"
      << "    int    col;             // columna actual\n"
      << "    std::string lxm;        // último lexema reconocido\n\n"
      << "    explicit Lexer(const std::string& src)\n"
      << "        : input(src), lexemeBegin(0), line(1), col(1) {}\n\n"
      << "    bool hasMore() const { return lexemeBegin < input.size(); }\n\n"
      << "    // Escanea el siguiente token usando el AFD (máximo lexema)\n"
      << "    // Implementa el esquema de dos punteros del Dragon Book:\n"
      << "    //   lexemeBegin → inicio del lexema actual\n"
      << "    //   forward     → puntero explorador que avanza\n"
      << "    // Retorna el índice de regla que acepta, o -1 en error\n"
      << "    int nextToken() {\n"
      << "        if (lexemeBegin >= input.size()) return -2; // EOF\n\n"
      << "        int    state           = START_STATE;\n"
      << "        int    last_accept_rule = -1;\n"
      << "        size_t last_accept_pos  = lexemeBegin; // posición del último match\n"
      << "        size_t forward          = lexemeBegin; // puntero explorador\n\n"
      << "        // Mover 'forward' mientras el AFD tenga transiciones válidas\n"
      << "        while (forward < input.size()) {\n"
      << "            unsigned char c = (unsigned char)input[forward];\n"
      << "            int next = transitions[state][c];\n"
      << "            if (next == -1) break;   // forward no puede avanzar más\n"
      << "            state = next;\n"
      << "            forward++;               // forward avanza un carácter\n"
      << "            if (accepting_rule[state] != -1) {\n"
      << "                last_accept_rule = accepting_rule[state];\n"
      << "                last_accept_pos  = forward; // guardar último match\n"
      << "            }\n"
      << "        }\n\n"
      << "        if (last_accept_rule == -1) {\n"
      << "            // Sin coincidencia → error léxico (avanzar un carácter)\n"
      << "            lxm = std::string(1, input[lexemeBegin]);\n"
      << "            lexemeBegin++;\n"
      << "            return -1;\n"
      << "        }\n\n"
      << "        // Extraer lexema: desde lexemeBegin hasta last_accept_pos\n"
      << "        lxm = input.substr(lexemeBegin, last_accept_pos - lexemeBegin);\n"
      << "        for (char c : lxm) {\n"
      << "            if (c == '\\n') { line++; col = 1; }\n"
      << "            else col++;\n"
      << "        }\n"
      << "        // Mover lexemeBegin al inicio del siguiente token\n"
      << "        lexemeBegin = last_accept_pos;\n"
      << "        return last_accept_rule;\n"
      << "    }\n\n"
      << "    // Ejecuta la acción semántica según el índice de regla\n"
      << "    void executeAction(int rule) {\n"
      << "        switch (rule) {\n";

    // ── Inyección de acciones semánticas ──
    for (int i = 0; i < R; i++) {
        f << "            case " << i << ": {\n";
        if (!spec.rules[i].action.empty()) {
            // La variable lxm está disponible como this->lxm
            // Reemplazar 'lxm' con this->lxm para que compile
            f << "                // Regex: " << cppEscape(spec.rules[i].original_regex) << "\n";
            f << "                " << spec.rules[i].action << "\n";
        } else {
            f << "                // (sin acción definida)\n";
        }
        f << "                break;\n"
          << "            }\n";
    }
    f << "            case -1:\n"
      << "                std::cerr << \"Error léxico en línea \" << line\n"
      << "                          << \", col \" << col\n"
      << "                          << \": carácter inesperado '\" << lxm << \"'\\n\";\n"
      << "                break;\n"
      << "            default:\n"
      << "                break;\n"
      << "        }\n"
      << "    }\n"
      << "};\n\n";

    // ── Función scan: escanea todo el input ──
    f << "// Escanea el texto completo e imprime todos los tokens\n"
      << "void scanAll(const std::string& input) {\n"
      << "    Lexer lexer(input);\n"
      << "    int token_count  = 0;\n"
      << "    int error_count  = 0;\n\n"
      << "    while (lexer.hasMore()) {\n"
      << "        size_t start = lexer.lexemeBegin; // posición antes de avanzar\n"
      << "        int rule = lexer.nextToken();\n\n"
      << "        if (rule == -2) break; // EOF\n\n"
      << "        if (rule == -1) {\n"
      << "            std::cerr << \"[Error léxico] línea \" << lexer.line\n"
      << "                      << \", col \" << lexer.col\n"
      << "                      << \": '\" << lexer.lxm << \"'\\n\";\n"
      << "            error_count++;\n"
      << "        } else {\n"
      << "            lexer.executeAction(rule);\n"
      << "            token_count++;\n"
      << "        }\n"
      << "    }\n\n"
      << "    std::cout << \"\\n=== Resumen ===\\n\";\n"
      << "    std::cout << \"Tokens reconocidos: \" << token_count << \"\\n\";\n"
      << "    std::cout << \"Errores léxicos:    \" << error_count << \"\\n\";\n"
      << "}\n\n";

    // ── main() ──
    f << "int main(int argc, char* argv[]) {\n"
      << "    if (argc < 2) {\n"
      << "        std::cerr << \"Uso: \" << argv[0] << \" <archivo_entrada>\\n\";\n"
      << "        return 1;\n"
      << "    }\n\n"
      << "    // Leer archivo de entrada\n"
      << "    std::ifstream file(argv[1]);\n"
      << "    if (!file.is_open()) {\n"
      << "        std::cerr << \"No se puede abrir: \" << argv[1] << \"\\n\";\n"
      << "        return 1;\n"
      << "    }\n"
      << "    std::string input((std::istreambuf_iterator<char>(file)),\n"
      << "                       std::istreambuf_iterator<char>());\n"
      << "    file.close();\n\n"
      << "    std::cout << \"Analizando: \" << argv[1] << \"\\n\";\n"
      << "    std::cout << std::string(40, '-') << \"\\n\";\n\n"
      << "    scanAll(input);\n\n";

    // ── Trailer del usuario ──
    if (!spec.trailer.empty()) {
        f << "    // --- Trailer del usuario ---\n"
          << "    " << spec.trailer << "\n"
          << "    // --- Fin trailer ---\n\n";
    }

    f << "    return 0;\n"
      << "}\n";

    f.close();

    // ── code_gen.cpp devuelve: archivo .cpp compilable ──────────────────
    std::cout << "[code_gen] generate() devuelve: " << out_path << "\n";
    std::cout << "  Contiene:\n";
    std::cout << "    transitions[" << N << "][256]  <- tabla de transiciones del AFD\n";
    std::cout << "    accepting_rule[" << N << "]       <- que regla acepta cada estado\n";
    std::cout << "    class Lexer con lexemeBegin y forward (Dragon Book)\n";
    std::cout << "    switch(" << R << " casos) con acciones semanticas\n";
    std::cout << "  Compilar: g++ -O2 -std=c++17 -o lexer " << out_path << "\n\n";
}
