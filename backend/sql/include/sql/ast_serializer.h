// Convierte el AST a JSON con el mismo formato de nodos que el parse tree
// ({kind, name, text, line, column, children}) para dibujarlo en el IDE.
#pragma once

#include "sql/ast.h"

#include <string>

namespace sql {

std::string astToJson(const Script &script);

} // namespace sql
