// Traduce los nombres técnicos de tokens a categorías legibles en el IDE.
// Misma interfaz que en el Proyecto 2; cambian las categorías (ahora SQL).
#pragma once

#include <string>

namespace sql {

std::string classifyToken(const std::string &antlr_type,
                          const std::string &lexeme);

} // namespace sql
