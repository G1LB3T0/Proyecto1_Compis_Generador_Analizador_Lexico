// Captura errores léxicos y sintácticos de ANTLR con línea y columna.
// Reutilizado del Proyecto 2 (compiscript/src/diagnostics.cpp); se agregó la
// traducción al español de los mensajes estándar de ANTLR.
#include "sql/diagnostics.h"

#include <utility>

namespace sql {

namespace {

std::string replaceAll(std::string text, const std::string &from,
                       const std::string &to) {
  for (size_t position = text.find(from); position != std::string::npos;
       position = text.find(from, position + to.size()))
    text.replace(position, from.size(), to);
  return text;
}

// ANTLR produce mensajes en inglés con formatos fijos; se traducen los más
// comunes para que el IDE explique el problema en el idioma del curso.
std::string translate(std::string message) {
  struct Pattern {
    const char *english;
    const char *spanish;
  };
  static const Pattern patterns[] = {
      {"token recognition error at: ", "Símbolo no reconocido: "},
      {"mismatched input ", "Entrada inesperada "},
      {"extraneous input ", "Entrada sobrante "},
      {"no viable alternative at input ", "No se reconoce una sentencia válida en "},
      {"missing ", "Falta "},
  };
  for (const auto &pattern : patterns) {
    if (message.rfind(pattern.english, 0) == 0) {
      message = pattern.spanish + message.substr(std::string(pattern.english).size());
      break;
    }
  }
  message = replaceAll(message, " expecting ", "; se esperaba ");
  message = replaceAll(message, " at '", " antes de '");
  return replaceAll(message, "<EOF>", "fin del script");
}

} // namespace

CollectingErrorListener::CollectingErrorListener(std::string category)
    : category_(std::move(category)) {}

void CollectingErrorListener::syntaxError(antlr4::Recognizer *, antlr4::Token *,
                                          size_t line,
                                          size_t char_position_in_line,
                                          const std::string &message,
                                          std::exception_ptr) {
  const std::string prefix = category_ == "léxico" ? "LEX" : "SYN";
  diagnostics_.push_back({prefix + "001", category_, "error", translate(message),
                          line, char_position_in_line + 1});
}

} // namespace sql
