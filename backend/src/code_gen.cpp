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

// ─── Extrae el nombre del token de la acción ──
// De: std::cout << "<KW_INT, \"" << lxm << "\">\n";
// Saca: "KW_INT"
// Si la acción es un comentario o vacía devuelve ""
// (esos tokens se ignoran — espacios, newlines, comentarios)
static std::string extractTokenName(const std::string& action) {
    // Busca el patrón "<NOMBRE," dentro de la acción
    size_t lt = action.find('"');
    if (lt == std::string::npos) {
        // Puede ser "return TOKEN" — busca return
        size_t ret = action.find("return ");
        if (ret != std::string::npos) {
            std::string after = action.substr(ret + 7);
            // quita espacios y punto y coma
            size_t start = after.find_first_not_of(" \t");
            size_t end   = after.find_last_not_of(" \t;");
            if (start != std::string::npos)
                return after.substr(start, end - start + 1);
        }
        return ""; // acción sin token identificable
    }
    // Busca el < después de la primera "
    size_t angle = action.find('<', lt);
    if (angle == std::string::npos) return "";
    size_t comma = action.find(',', angle);
    if (comma == std::string::npos) return "";
    std::string name = action.substr(angle + 1, comma - angle - 1);
    // trim
    size_t s = name.find_first_not_of(" \t");
    size_t e = name.find_last_not_of(" \t");
    if (s == std::string::npos) return "";
    return name.substr(s, e - s + 1);
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

    // ── Cabecera ──
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

    // ── Struct Token — la novedad del Avance 1 ──
    // El lexer generado ya no solo imprime tokens,
    // también puede devolverlos como vector<Token>
    // para que el parser los consuma.
    f << "// ─────────────────────────────────────────────\n"
      << "// Struct Token — unidad que consume el parser\n"
      << "// ─────────────────────────────────────────────\n"
      << "struct Token {\n"
      << "    std::string tipo;    // \"KW_INT\", \"ID\", etc.\n"
      << "    std::string lexema;  // texto real capturado\n"
      << "    int         linea;\n"
      << "    int         columna;\n"
      << "    Token(const std::string& t, const std::string& l,\n"
      << "          int lin=0, int col=0)\n"
      << "        : tipo(t), lexema(l), linea(lin), columna(col) {}\n"
      << "};\n\n";

    // ── Tabla de transiciones ──
    f << "static const int NUM_STATES  = " << N << ";\n"
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

    // ── Tabla de aceptación ──
    f << "static const int accepting_rule[" << N << "] = { ";
    for (int s = 0; s < N; s++) {
        f << (dfa.states[s].is_accepting ? dfa.states[s].accepting_rule : -1);
        if (s < N - 1) f << ", ";
    }
    f << " };\n\n";

    // ── Tabla de nombres de tokens — NUEVO ──
    // token_types[i] = nombre del token que acepta la regla i
    // Si es "" significa que esa regla se ignora (espacios, etc.)
    f << "// Nombre del token por regla\n"
      << "// \"\" = ignorar (espacios, comentarios, newlines)\n"
      << "static const char* token_types[" << R << "] = {\n";
    for (int i = 0; i < R; i++) {
        std::string name = extractTokenName(spec.rules[i].action);
        f << "    \"" << cppEscape(name) << "\"";
        if (i < R - 1) f << ",";
        f << "  // regla " << i
          << ": " << cppEscape(spec.rules[i].original_regex) << "\n";
    }
    f << "};\n\n";

    // ── Clase Lexer ──
    f << "class Lexer {\n"
      << "public:\n"
      << "    std::string input;\n"
      << "    size_t lexemeBegin;\n"
      << "    int    line;\n"
      << "    int    col;\n"
      << "    std::string lxm;\n\n"
      << "    explicit Lexer(const std::string& src)\n"
      << "        : input(src), lexemeBegin(0), line(1), col(1) {}\n\n"
      << "    bool hasMore() const { return lexemeBegin < input.size(); }\n\n"
      // ── nextToken: esquema Dragon Book ──
      << "    int nextToken() {\n"
      << "        if (lexemeBegin >= input.size()) return -2;\n\n"
      << "        int    state            = START_STATE;\n"
      << "        int    last_accept_rule = -1;\n"
      << "        size_t last_accept_pos  = lexemeBegin;\n"
      << "        size_t forward          = lexemeBegin;\n\n"
      << "        while (forward < input.size()) {\n"
      << "            unsigned char c = (unsigned char)input[forward];\n"
      << "            int next = transitions[state][c];\n"
      << "            if (next == -1) break;\n"
      << "            state = next;\n"
      << "            forward++;\n"
      << "            if (accepting_rule[state] != -1) {\n"
      << "                last_accept_rule = accepting_rule[state];\n"
      << "                last_accept_pos  = forward;\n"
      << "            }\n"
      << "        }\n\n"
      << "        if (last_accept_rule == -1) {\n"
      << "            lxm = std::string(1, input[lexemeBegin]);\n"
      << "            lexemeBegin++;\n"
      << "            return -1;\n"
      << "        }\n\n"
      << "        lxm = input.substr(lexemeBegin, last_accept_pos - lexemeBegin);\n"
      << "        for (char c : lxm) {\n"
      << "            if (c == '\\n') { line++; col = 1; }\n"
      << "            else col++;\n"
      << "        }\n"
      << "        lexemeBegin = last_accept_pos;\n"
      << "        return last_accept_rule;\n"
      << "    }\n\n"
      // ── executeAction: modo demo (imprime en pantalla) ──
      << "    void executeAction(int rule) {\n"
      << "        switch (rule) {\n";

    for (int i = 0; i < R; i++) {
        f << "            case " << i << ": {\n";
        if (!spec.rules[i].action.empty()) {
            f << "                " << spec.rules[i].action << "\n";
        }
        f << "                break;\n"
          << "            }\n";
    }
    f << "            case -1:\n"
      << "                std::cerr << \"[Error léxico] línea \" << line\n"
      << "                          << \" col \" << col\n"
      << "                          << \": '\" << lxm << \"'\\n\";\n"
      << "                break;\n"
      << "            default: break;\n"
      << "        }\n"
      << "    }\n"
      << "};\n\n";

    // ── scanAll: modo demo — imprime tokens en pantalla ──
    f << "// Modo demo: imprime todos los tokens\n"
      << "void scanAll(const std::string& input) {\n"
      << "    Lexer lexer(input);\n"
      << "    int tok_count = 0, err_count = 0;\n"
      << "    while (lexer.hasMore()) {\n"
      << "        int rule = lexer.nextToken();\n"
      << "        if (rule == -2) break;\n"
      << "        if (rule == -1) { err_count++; lexer.executeAction(-1); }\n"
      << "        else { lexer.executeAction(rule); tok_count++; }\n"
      << "    }\n"
      << "    std::cout << \"\\n=== Resumen ===\\n\";\n"
      << "    std::cout << \"Tokens reconocidos: \" << tok_count << \"\\n\";\n"
      << "    std::cout << \"Errores léxicos:    \" << err_count << \"\\n\";\n"
      << "}\n\n";

    // ── tokenize: Avance 1 — devuelve vector<Token> ──
    // Esta es la función que usa el parser.
    // Los tokens con tipo "" (espacios, comentarios) se ignoran.
    // Al final agrega el token especial "$" que indica EOF.
    f << "// ─────────────────────────────────────────────\n"
      << "// Avance 1: tokenize()\n"
      << "// En lugar de imprimir, devuelve vector<Token>\n"
      << "// para que el parser los pueda consumir.\n"
      << "// Los tokens ignorados (ws, comentarios) no se incluyen.\n"
      << "// El último token es siempre <$, \"$\"> = EOF.\n"
      << "// ─────────────────────────────────────────────\n"
      << "std::vector<Token> tokenize(const std::string& input) {\n"
      << "    std::vector<Token> result;\n"
      << "    Lexer lexer(input);\n"
      << "    int err_count = 0;\n\n"
      << "    while (lexer.hasMore()) {\n"
      << "        int lin = lexer.line;\n"
      << "        int col = lexer.col;\n"
      << "        int rule = lexer.nextToken();\n\n"
      << "        if (rule == -2) break;\n\n"
      << "        if (rule == -1) {\n"
      << "            // Error léxico — igual se reporta y se sigue\n"
      << "            std::cerr << \"[Error léxico] línea \" << lin\n"
      << "                      << \" col \" << col\n"
      << "                      << \": '\" << lexer.lxm << \"'\\n\";\n"
      << "            err_count++;\n"
      << "            result.push_back(Token(\"ERROR\", lexer.lxm, lin, col));\n"
      << "        } else {\n"
      << "            std::string tipo = token_types[rule];\n"
      << "            if (!tipo.empty()) {\n"
      << "                // Token válido y no ignorado → agregar al stream\n"
      << "                result.push_back(Token(tipo, lexer.lxm, lin, col));\n"
      << "            }\n"
      << "            // Si tipo == \"\" es ws/comentario → se descarta\n"
      << "        }\n"
      << "    }\n\n"
      << "    // Token EOF — el parser lo usa para saber que terminó\n"
      << "    result.push_back(Token(\"$\", \"$\", lexer.line, lexer.col));\n\n"
      << "    std::cout << \"[tokenize] \" << result.size() - 1\n"
      << "              << \" tokens (sin contar EOF), \"\n"
      << "              << err_count << \" errores\\n\";\n"
      << "    return result;\n"
      << "}\n\n";

    // ── main ──
    f << "int main(int argc, char* argv[]) {\n"
      << "    if (argc < 2) {\n"
      << "        std::cerr << \"Uso: \" << argv[0]\n"
      << "                  << \" <archivo> [--tokens]\\n\";\n"
      << "        std::cerr << \"  --tokens  muestra el vector<Token>\\n\";\n"
      << "        return 1;\n"
      << "    }\n\n"
      << "    std::ifstream file(argv[1]);\n"
      << "    if (!file.is_open()) {\n"
      << "        std::cerr << \"No se puede abrir: \" << argv[1] << \"\\n\";\n"
      << "        return 1;\n"
      << "    }\n"
      << "    std::string input((std::istreambuf_iterator<char>(file)),\n"
      << "                       std::istreambuf_iterator<char>());\n"
      << "    file.close();\n\n"
      << "    // Modo --tokens: usa tokenize() y muestra el vector<Token>\n"
      << "    bool modo_tokens = (argc >= 3 &&\n"
      << "                        std::string(argv[2]) == \"--tokens\");\n\n"
      << "    std::cout << \"Analizando: \" << argv[1] << \"\\n\";\n"
      << "    std::cout << std::string(40, '-') << \"\\n\";\n\n"
      << "    if (modo_tokens) {\n"
      << "        // ── Avance 1: tokenize() ──\n"
      << "        std::vector<Token> tokens = tokenize(input);\n"
      << "        std::cout << \"\\n========== TOKEN STREAM ==========\\n\";\n"
      << "        for (size_t i = 0; i < tokens.size(); i++) {\n"
      << "            std::cout << \"[\" << i << \"] \"\n"
      << "                      << \"<\" << tokens[i].tipo << \", \\\"\"\n"
      << "                      << tokens[i].lexema << \"\\\">\";\n"
      << "            if (tokens[i].tipo != \"$\")\n"
      << "                std::cout << \"  l:\" << tokens[i].linea\n"
      << "                          << \" c:\" << tokens[i].columna;\n"
      << "            std::cout << \"\\n\";\n"
      << "        }\n"
      << "        std::cout << \"==================================\\n\";\n"
      << "    } else {\n"
      << "        // ── Modo demo: scanAll() imprime en pantalla ──\n"
      << "        scanAll(input);\n"
      << "    }\n\n";

    if (!spec.trailer.empty()) {
        f << "    " << spec.trailer << "\n\n";
    }

    f << "    return 0;\n"
      << "}\n";

    f.close();

    std::cout << "[code_gen] generate() -> " << out_path << "\n";
    std::cout << "  transitions[" << N << "][256]\n";
    std::cout << "  token_types[" << R << "]  <- NUEVO: nombre de cada token\n";
    std::cout << "  tokenize()              <- NUEVO: devuelve vector<Token>\n";
    std::cout << "  scanAll()               <- modo demo (imprime en pantalla)\n";
    std::cout << "  Uso normal:  ./lexer_bin archivo\n";
    std::cout << "  Uso tokens:  ./lexer_bin archivo --tokens\n\n";
}
