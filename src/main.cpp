#include <iostream>
#include <string>
#include <stdexcept>
#include "yalex_parser.h"
#include "dfa_builder.h"
#include "code_gen.h"
#include "dot_gen.h"
#include "automata_serializer.h" 

static void print_usage(const char* prog) {
    std::cerr << "Uso: " << prog << " <input.yal> -o <output_base>\n";
    std::cerr << "     " << prog << " <input.yal> -o <output_base> --rebuild\n";
    std::cerr << "     (--rebuild fuerza reconstruir aunque exista .json)\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) { print_usage(argv[0]); return 1; }

    std::string input_file, output_base;
    bool force_rebuild = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if      (arg == "-o" && i+1 < argc) output_base = argv[++i];
        else if (arg == "--rebuild")         force_rebuild = true;
        else if (arg[0] != '-')             input_file = arg;
    }
    if (input_file.empty() || output_base.empty()) { print_usage(argv[0]); return 1; }

    // El autómata se persiste en: output_base + ".json"
    // Ejemplo: si output_base = "build/lexer" → "build/lexer.json"
    std::string json_path = output_base + ".json";

    std::cout << "=== Generador de Analizadores Lexicos YALex -> C++ ===\n";
    std::cout << "Archivo: " << input_file << "\n\n";

    try {
        AutomataSerializer serializer;
        DFA       dfa;
        YALexSpec spec;
        RegexNode* expr_tree = nullptr;
        int pos_count = 0;

        if (!force_rebuild && serializer.exists(json_path)) {

            // ═══════════════════════════════════════════════
            //  SEGUNDA PASADA: cargar autómata desde disco
            //  dfa_builder SE OMITE completamente
            // ═══════════════════════════════════════════════
            std::cout << "--- Autómata encontrado en disco: " << json_path << " ---\n";
            std::cout << "--- Cargando (omitiendo construcción)... ---\n\n";
            serializer.load(json_path, dfa, spec);

        } else {

            // ═══════════════════════════════════════════════
            //  PRIMERA PASADA: construir todo desde cero
            // ═══════════════════════════════════════════════

            // FASE 1: yalex_parser
            std::cout << "--- FASE 1: yalex_parser.cpp ---\n";
            YALexParser yalex_parser;
            spec = yalex_parser.parse(input_file);

            // FASE 2+3: regex_parser + dfa_builder
            std::cout << "--- FASE 2+3: regex_parser + dfa_builder ---\n";
            DFABuilder builder;
            dfa = builder.build(spec, expr_tree, pos_count);

            // PERSISTENCIA: guardar el AFD en disco para la próxima vez
            std::cout << "--- PERSISTENCIA: automata_serializer.cpp ---\n";
            serializer.save(dfa, spec, json_path);
            std::cout << "  (proxima ejecucion omitira las fases 1-3)\n\n";
        }

        // VISUALIZACIÓN (solo en primera pasada si tenemos el árbol)
        if (expr_tree) {
            std::cout << "--- VISUALIZACION: dot_gen.cpp ---\n";
            DotGenerator dot_gen;
            dot_gen.generateExprTree(expr_tree, output_base + "_tree.dot", spec);
            dot_gen.generateDFA(dfa, spec, output_base + "_dfa.dot");
        }

        // FASE 4: code_gen (siempre se ejecuta, en ambas pasadas)
        std::cout << "--- FASE 4: code_gen.cpp ---\n";
        CodeGenerator code_gen;
        code_gen.generate(dfa, spec, output_base);

        freeTree(expr_tree);

        std::cout << "=== Completado ===\n";
        std::cout << "Archivos generados:\n";
        std::cout << "  " << output_base << ".cpp    <- compilar esto\n";
        std::cout << "  " << json_path   << "        <- autómata persistido\n";
        if (expr_tree) {
            std::cout << "  " << output_base << "_tree.dot\n";
            std::cout << "  " << output_base << "_dfa.dot\n";
        }
        std::cout << "\nCompilar lexer generado:\n";
        std::cout << "  g++ -O2 -std=c++17 -o lexer " << output_base << ".cpp\n";
        std::cout << "Ejecutar:\n";
        std::cout << "  ./lexer test_cpp.cpp\n";

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}