#pragma once

#include <string>

namespace compiscript {

std::string classifyToken(const std::string &antlr_type,
                          const std::string &lexeme);

} // namespace compiscript
