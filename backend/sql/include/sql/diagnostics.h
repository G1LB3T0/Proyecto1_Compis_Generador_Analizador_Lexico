// Declara el listener que convierte los errores de ANTLR en diagnósticos del IDE.
// Reutilizado del Proyecto 2 (compiscript/diagnostics.h); misma interfaz.
#pragma once

#include "antlr4-runtime.h"
#include "sql/model.h"

#include <string>
#include <vector>

namespace sql {

class CollectingErrorListener final : public antlr4::BaseErrorListener {
public:
  explicit CollectingErrorListener(std::string category);

  void syntaxError(antlr4::Recognizer *recognizer,
                   antlr4::Token *offending_symbol, size_t line,
                   size_t char_position_in_line, const std::string &message,
                   std::exception_ptr exception) override;

  const std::vector<Diagnostic> &diagnostics() const { return diagnostics_; }

private:
  std::string category_;
  std::vector<Diagnostic> diagnostics_;
};

} // namespace sql
