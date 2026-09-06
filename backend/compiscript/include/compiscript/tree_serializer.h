// Convierte el árbol sintáctico de ANTLR a la estructura JSON del gráfico.
#pragma once

#include "antlr4-runtime.h"

#include <string>

namespace compiscript {

std::string parseTreeToJson(antlr4::tree::ParseTree *tree,
                            antlr4::Parser &parser);

} // namespace compiscript
