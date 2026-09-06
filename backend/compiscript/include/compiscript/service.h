// Expone el punto de entrada que coordina lexer, parser y análisis semántico.
#pragma once

#include "compiscript/model.h"

#include <string>

namespace compiscript {

AnalysisResult analyzeSource(const std::string &source,
                             const std::string &filename = "program.cps");

} // namespace compiscript
