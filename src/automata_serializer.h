#pragma once
#include "types.h"
#include <string>

// ─────────────────────────────────────────────
// AutomataSerializer
//
// Responde la pregunta del profe:
//   "¿cómo vas a persistir el autómata después
//    de la primera pasada?"
//
// Serializa/deserializa el DFA minimizado +
// el YALexSpec a un archivo JSON legible.
//
// Formato del JSON:
// {
//   "entrypoint": "gettoken",
//   "num_states": 42,
//   "start_state": 0,
//   "states": [
//     {
//       "id": 0,
//       "is_accepting": false,
//       "accepting_rule": -1,
//       "transitions": [-1, -1, 3, ...]   // 256 entradas
//     }, ...
//   ],
//   "rules": [
//     {
//       "original_regex": "ws+",
//       "expanded_regex": "([' '...])+",
//       "action": "/* ignorar */"
//     }, ...
//   ]
// }
// ─────────────────────────────────────────────

class AutomataSerializer {
public:
    // Guarda el DFA + spec en <path>.
    // Lanza std::runtime_error si no puede escribir.
    void save(const DFA& dfa,
              const YALexSpec& spec,
              const std::string& path);

    // Carga el DFA + spec desde <path>.
    // Lanza std::runtime_error si el archivo no existe
    // o tiene formato incorrecto.
    void load(const std::string& path,
              DFA& out_dfa,
              YALexSpec& out_spec);

    // Devuelve true si ya existe un archivo persistido en <path>.
    // Úsalo en main() para decidir si construir o cargar.
    bool exists(const std::string& path) const;
};
