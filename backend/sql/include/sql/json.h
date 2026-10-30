// Declara la serialización de los resultados del intérprete a JSON.
// quoteJson se reutiliza del Proyecto 2; el resto describe resultados SQL.
#pragma once

#include "sql/model.h"
#include "sql/storage.h"

#include <string>

namespace sql {

std::string quoteJson(const std::string &value);
std::string databaseToJson(Storage &storage);
std::string runToJson(const RunResult &result);

} // namespace sql
