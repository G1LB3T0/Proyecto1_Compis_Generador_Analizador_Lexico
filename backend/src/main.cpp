#include <iostream>
#include <string>
#include <stdexcept>
#include "yalex_parser.h"
#include "dfa_builder.h"
#include "code_gen.h"
#include "dot_gen.h"
#include "automata_serializer.h"
#include "yapar_parser.h"    // Avance 2
#include "first_follow.h"    // Avance 2

static void print_usage(const char* prog) {
    std::cerr << "Uso: " << prog << " <input.yal> -o <output_base> [opciones]\n";
    std::cerr << "     " << prog << " <input.yal> -o <output_base> --rebuild\n";
    std::cerr << "     " << prog << " <input.yal> -o <output_base> --yalp <grammar.yalp>\n";
    std::cerr << "Opciones:\n";
    std::cerr << "  --rebuild       fuerza reconstruir el automata\n";
    std::cerr << "  --yalp <file>   calcula FIRST y FOLLOW de la gramatica\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) { print_usage(argv[0]); return 1; }

    std::string input_file, output_base, yalp_file;
    bool force_rebuild = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if      (arg == "-o" && i+1 < argc)      output_base = argv[++i];
        else if (arg == "--rebuild")              force_rebuild = true;
        else if (arg == "--yalp" && i+1 < argc)  yalp_file = argv[++i];
        else if (arg[0] != '-')                  input_file = arg;
    }
    if (input_file.empty() || output_base.empty()) {
        print_usage(argv[0]); return 1;
    }

    std::string json_path = output_base + ".json";

    std::cout << "=== Generador de Analizadores Lexicos YALex -> C++ ===\n";
    std::cout << "Archivo: " << input_file << "\n\n";

    try {
        AutomataSerializer serializer;
        DFA       dfa;
        YALexSpec spec;
        RegexNode* expr_tree = nullptr;
        int pos_count = 0;

        // ── Decisión: cargar o construir ──────────────────
        if (!force_rebuild && serializer.exists(json_path)) {
            std::cout << "--- Autómata en disco: " << json_path << " ---\n";
            serializer.load(json_path, dfa, spec);
        } else {
            // FASE 1
            std::cout << "--- FASE 1: yalex_parser.cpp ---\n";
            YALexParser yalex_parser;
            spec = yalex_parser.parse(input_file);

            // FASE 2+3
            std::cout << "--- FASE 2+3: regex_parser + dfa_builder ---\n";
            DFABuilder builder;
            dfa = builder.build(spec, expr_tree, pos_count);

            // PERSISTENCIA
            std::cout << "--- PERSISTENCIA: automata_serializer.cpp ---\n";
            serializer.save(dfa, spec, json_path);
            std::cout << "  (proxima ejecucion omitira las fases 1-3)\n\n";
        }

        // VISUALIZACIÓN
        if (expr_tree) {
            std::cout << "--- VISUALIZACION: dot_gen.cpp ---\n";
            DotGenerator dot_gen;
            dot_gen.generateExprTree(expr_tree, output_base+"_tree.dot", spec);
            dot_gen.generateDFA(dfa, spec, output_base+"_dfa.dot");
        }

        // FASE 4
        std::cout << "--- FASE 4: code_gen.cpp ---\n";
        CodeGenerator code_gen;
        code_gen.generate(dfa, spec, output_base);

        freeTree(expr_tree);

        // ── AVANCE 2: FIRST y FOLLOW ──────────────────────
        if (!yalp_file.empty()) {
            std::cout << "--- FIRST y FOLLOW ---\n";

            // Lee la gramática del .yalp
            YaparParser yapar;
            Grammar g = yapar.parse(yalp_file);

            // Imprime la gramática
            g.print();

            // Calcula FIRST y FOLLOW
            FirstFollow ff;
            ff.compute(g);

            // Imprime los conjuntos
            ff.print();
        }

        std::cout << "=== Completado ===\n";
        std::cout << "  " << output_base << ".cpp\n";
        std::cout << "  " << json_path   << "\n";
        if (!yalp_file.empty())
            std::cout << "  FIRST y FOLLOW calculados para: " << yalp_file << "\n";
        std::cout << "\nCompilar lexer:\n";
        std::cout << "  g++ -O2 -std=c++17 -o lexer " << output_base << ".cpp\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}