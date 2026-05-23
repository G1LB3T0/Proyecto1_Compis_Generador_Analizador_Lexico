#pragma once
#include "types.h"
#include <string>

// ─────────────────────────────────────────────
// Módulo 4 – Generador de Código
//
// Serializa el AFD minimizado en un archivo C++
// compilable que implementa el analizador léxico.
//
// El archivo generado:
//   • Embebe la tabla de transiciones como array 2D
//   • Implementa la función de escaneo (máximo lexema)
//   • Inyecta las acciones semánticas en un switch
//   • Incluye el header/trailer del usuario
//   • Tiene un main() que lee el archivo de entrada
// ─────────────────────────────────────────────

class CodeGenerator {
public:
    // Genera el archivo <output_base>.cpp
    void generate(const DFA& dfa,
                  const YALexSpec& spec,
                  const std::string& output_base);
};
