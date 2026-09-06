// Declara la serialización de los resultados del analizador a JSON.
#pragma once

#include "compiscript/model.h"

#include <string>

namespace compiscript {

std::string quoteJson(const std::string &value);
std::string analysisToJson(const AnalysisResult &result);

} // namespace compiscript
