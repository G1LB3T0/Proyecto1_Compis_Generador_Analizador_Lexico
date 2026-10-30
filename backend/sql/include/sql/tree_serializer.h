// Convierte el árbol sintáctico de ANTLR a la estructura JSON del gráfico.
// Reutilizado del Proyecto 2 (compiscript/tree_serializer) sin cambios de lógica.
#pragma once

#include "antlr4-runtime.h"

#include <string>

namespace sql {

std::string parseTreeToJson(antlr4::tree::ParseTree *tree,
                            antlr4::Parser &parser);

} // namespace sql
