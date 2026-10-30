// Punto de entrada que coordina lexer, parser, AST, semántica e interpretación.
// Misma idea que compiscript/service.h del Proyecto 2, con nuevas fases.
#pragma once

#include "sql/model.h"

#include <string>

namespace sql {

struct RunOptions {
  std::string database = "database"; // carpeta raíz del almacenamiento
  std::string filename = "script.sql";
  bool execute = true; // false = solo analizar (no toca los archivos)
};

RunResult runScript(const std::string &source, const RunOptions &options = {});

// Catálogo y archivos actuales sin ejecutar nada (vista "Base de datos").
std::string describeDatabase(const std::string &database, std::string &error);

// Elimina metadata, tablas e índices de la carpeta indicada.
bool resetDatabase(const std::string &database, std::string &error);

} // namespace sql
