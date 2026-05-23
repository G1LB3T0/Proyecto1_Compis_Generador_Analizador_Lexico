#pragma once
#include "types.h"
#include <string>

// ─────────────────────────────────────────────
// Módulo 1 – Procesador de Especificaciones
//
// Responsabilidades:
//   • Leer el archivo .yal línea por línea
//   • Gestionar tabla de símbolos (let definitions)
//   • Expandir macros en las expresiones regulares
//   • Reportar errores de formato
// ─────────────────────────────────────────────

class YALexParser {
public:
    // Parsear un archivo .yal y retornar la especificación
    YALexSpec parse(const std::string& filename);

private:
    // Lee el archivo completo como string
    std::string read_file(const std::string& filename);

    // Elimina comentarios (* ... *)
    void remove_comments(std::string& content);

    // Extrae el contenido de un bloque { ... } (maneja llaves anidadas)
    std::string extract_braced(const std::string& content, size_t& pos);

    // Expande referencias a macros en una regex string
    std::string expand_defs(const std::string& regex,
                            const std::map<std::string, std::string>& defs,
                            int depth = 0);

    // Utilitarios de string
    std::string trim(const std::string& s);
    bool starts_with(const std::string& s, size_t pos, const std::string& prefix);

    // Avanza saltando espacio en blanco (incluye newlines)
    void skip_whitespace(const std::string& content, size_t& pos);

    // Lee la parte regex de una regla (hasta encontrar '{' de la acción)
    std::string read_regex(const std::string& content, size_t& pos);
};
