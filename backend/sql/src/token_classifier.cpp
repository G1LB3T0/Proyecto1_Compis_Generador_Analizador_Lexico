// Agrupa los tokens reconocidos por ANTLR en categorías fáciles de presentar.
// Adaptado del Proyecto 2: las palabras reservadas ahora son las de SQL.
#include "sql/token_classifier.h"

#include <set>

namespace sql {

std::string classifyToken(const std::string &antlr_type,
                          const std::string &lexeme) {
  if (antlr_type == "IDENTIFIER")
    return "Identificador";
  if (antlr_type == "INT_LITERAL")
    return "Literal entero";
  if (antlr_type == "FLOAT_LITERAL")
    return "Literal decimal";
  if (antlr_type == "STRING_LITERAL")
    return "Literal de texto";

  static const std::set<std::string> data_types = {
      "K_INT", "K_FLOAT", "K_VARCHAR", "K_BOOLEAN", "K_DATE"};
  static const std::set<std::string> boolean_literals = {"K_TRUE", "K_FALSE"};
  static const std::set<std::string> aggregates = {"K_COUNT", "K_SUM", "K_AVG",
                                                   "K_MIN", "K_MAX"};
  static const std::set<std::string> logical = {"K_AND", "K_OR", "K_NOT"};
  static const std::set<std::string> constraints = {
      "K_PRIMARY", "K_KEY", "K_FOREIGN", "K_REFERENCES", "K_UNIQUE"};
  static const std::set<std::string> arithmetic = {"PLUS", "MINUS", "STAR",
                                                   "SLASH", "PERCENT"};
  static const std::set<std::string> comparison = {"EQ", "NEQ", "LT",
                                                   "LTE", "GT", "GTE"};

  if (data_types.count(antlr_type))
    return "Tipo de dato";
  if (boolean_literals.count(antlr_type))
    return "Literal booleano";
  if (antlr_type == "K_NULL")
    return "Literal nulo";
  if (aggregates.count(antlr_type))
    return "Función de agregación";
  if (logical.count(antlr_type))
    return "Operador lógico";
  if (constraints.count(antlr_type))
    return "Restricción";
  if (antlr_type.rfind("K_", 0) == 0)
    return "Palabra reservada";
  if (arithmetic.count(antlr_type))
    return lexeme == "*" ? "Asterisco / multiplicación" : "Operador aritmético";
  if (comparison.count(antlr_type))
    return "Operador de comparación";
  if (antlr_type == "COMMA")
    return "Separador";
  if (antlr_type == "SEMI")
    return "Fin de sentencia";
  if (antlr_type == "DOT")
    return "Calificador tabla.columna";
  if (antlr_type == "LPAREN" || antlr_type == "RPAREN")
    return "Delimitador";
  return "Símbolo";
}

} // namespace sql
