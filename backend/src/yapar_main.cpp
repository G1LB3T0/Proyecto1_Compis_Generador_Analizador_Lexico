#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "yapar_parser.h"
#include "first_follow.h"
#include "lr0_automaton.h"
#include "slr_table.h"
#include "ll1_table.h"
#include "lalr_table.h"
#include "lr_parser.h"
#include "ll1_parser.h"
#include "token_stream.h"

// ── Uso ───────────────────────────────────────────────────────

static void printUsage(const char* prog) {
    std::cerr << "Uso: " << prog
              << " <grammar.yalp> [-i <input.txt>] [--slr] [--lalr] [--ll1] [--all]\n";
    std::cerr << "Opciones:\n";
    std::cerr << "  -i <file>  archivo de cadenas a analizar (una por línea)\n";
    std::cerr << "  --slr      mostrar tabla SLR(1) y analizar con SLR\n";
    std::cerr << "  --lalr     mostrar tabla LALR(1) y analizar con LALR\n";
    std::cerr << "  --ll1      mostrar tabla LL(1) y analizar con LL(1)\n";
    std::cerr << "  --all      todos los métodos (default si no se especifica)\n";
}

// ── Leer un archivo de tokens de texto plano ─────────────────
// Formato: cada línea es un token en la forma  TIPO lexema
// (producido manualmente o como salida de debug del lexer)
static std::vector<Token> loadTokenFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("No se puede abrir el archivo de entrada: " + path);

    std::vector<Token> tokens;
    std::string line;
    int lineno = 0;
    while (std::getline(f, line)) {
        lineno++;
        // Saltar líneas vacías y comentarios
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string tipo, lexema;
        ss >> tipo;
        std::getline(ss, lexema);
        // Quitar espacio inicial del lexema
        if (!lexema.empty() && lexema[0] == ' ') lexema = lexema.substr(1);
        tokens.push_back({tipo, lexema, lineno, 1});
    }
    return tokens;
}

// ── Mostrar resultado del análisis ───────────────────────────

static void printResult(const std::string& method,
                         bool accepted,
                         const std::string& errorMsg) {
    std::cout << "[" << method << "] ";
    if (accepted) {
        std::cout << "ACEPTADA ✓\n";
    } else {
        std::cout << "RECHAZADA ✗\n";
        if (!errorMsg.empty())
            std::cout << "  " << errorMsg << "\n";
    }
}

// ── Main ──────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 1; }

    std::string yalpFile, inputFile;
    bool doSLR = false, doLALR = false, doLL1 = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if      (arg == "-i"    && i+1 < argc) inputFile = argv[++i];
        else if (arg == "--slr")               doSLR  = true;
        else if (arg == "--lalr")              doLALR = true;
        else if (arg == "--ll1")               doLL1  = true;
        else if (arg == "--all")               doSLR = doLALR = doLL1 = true;
        else if (arg[0] != '-')                yalpFile = arg;
    }

    if (yalpFile.empty()) { printUsage(argv[0]); return 1; }

    // Si no se especificó método, activar todos
    if (!doSLR && !doLALR && !doLL1) doSLR = doLALR = doLL1 = true;

    std::cout << "=== YAPar — Generador de Analizadores Sintácticos ===\n";
    std::cout << "Gramática: " << yalpFile << "\n\n";

    try {
        // ── FASE 1: leer la gramática ─────────────────────────
        YaparParser yapar;
        Grammar g = yapar.parse(yalpFile);
        g.print();

        // ── FASE 2: calcular FIRST y FOLLOW ──────────────────
        std::cout << "--- FIRST y FOLLOW ---\n";
        FirstFollow ff;
        ff.compute(g);
        ff.print();

        // ── FASE 3: construir el autómata LR(0) ──────────────
        std::cout << "--- Autómata LR(0) ---\n";
        LR0Automaton lr0;
        lr0.build(g);
        lr0.print();

        // ── FASE 4: construir tablas según métodos solicitados
        SLRTable  slrTable;
        LALRTable lalrTable;
        LL1Table  ll1Table;

        if (doSLR || doLALR) {
            if (doSLR) {
                std::cout << "--- Tabla SLR(1) ---\n";
                slrTable.build(lr0, ff);
                slrTable.print(lr0.augmented);
                if (slrTable.hasConflicts())
                    std::cout << "[SLR] Advertencia: la gramática tiene conflictos.\n\n";
            }
            if (doLALR) {
                std::cout << "--- Tabla LALR(1) ---\n";
                lalrTable.build(lr0, ff);
                lalrTable.print(lr0.augmented);
                if (lalrTable.hasConflicts())
                    std::cout << "[LALR] Advertencia: la gramática tiene conflictos.\n\n";
            }
        }
        if (doLL1) {
            std::cout << "--- Tabla LL(1) ---\n";
            ll1Table.build(g, ff);
            ll1Table.print(g);
            if (ll1Table.hasConflicts())
                std::cout << "[LL(1)] Advertencia: la gramática tiene conflictos.\n\n";
        }

        // ── FASE 5: analizar el archivo de entrada ─────────────
        if (!inputFile.empty()) {
            std::cout << "--- Análisis de cadenas: " << inputFile << " ---\n\n";

            std::vector<Token> tokens = loadTokenFile(inputFile);

            // Filtrar tokens ignorados
            std::vector<Token> filtered;
            for (const auto& tok : tokens)
                if (!g.ignored.count(tok.tipo))
                    filtered.push_back(tok);

            if (filtered.empty()) {
                std::cout << "El archivo de entrada no contiene tokens.\n";
            } else {
                TokenStream tsDebug(filtered);
                tsDebug.print();

                LRParser  lrParser;
                LL1Parser ll1Parser;

                // Cada método recibe un TokenStream fresco
                if (doSLR && !slrTable.hasConflicts()) {
                    TokenStream ts(filtered);
                    auto res = lrParser.parse(slrTable, lr0.augmented, ts);
                    printResult("SLR(1)", res.accepted, res.error_msg);
                }
                if (doLALR && !lalrTable.hasConflicts()) {
                    TokenStream ts(filtered);
                    auto res = lrParser.parse(lalrTable.action, lalrTable.goto_table,
                                              lr0.augmented, ts);
                    printResult("LALR(1)", res.accepted, res.error_msg);
                }
                if (doLL1 && !ll1Table.hasConflicts()) {
                    TokenStream ts(filtered);
                    auto res = ll1Parser.parse(ll1Table, g, ts);
                    printResult("LL(1)", res.accepted, res.error_msg);
                }
            }
        }

        std::cout << "\n=== Completado ===\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
