// Agrupa los tokens reconocidos por ANTLR en categorías fáciles de presentar.
#include "compiscript/token_classifier.h"

#include <set>

namespace compiscript {

std::string classifyToken(const std::string &antlr_type,
                          const std::string &lexeme) {
  if (antlr_type == "Identifier")
    return "Identificador";
  if (antlr_type == "IntegerLiteral")
    return "Literal entero";
  if (antlr_type == "FloatLiteral")
    return "Literal decimal";
  if (antlr_type == "StringLiteral")
    return "Literal de texto";

  static const std::set<std::string> data_types = {
      "boolean", "integer", "float", "string", "void"};
  static const std::set<std::string> boolean_literals = {"true", "false"};
  static const std::set<std::string> reserved_words = {
      "let",      "var",      "const",    "function", "class",
      "if",       "else",     "while",    "do",       "for",
      "foreach",  "in",       "break",    "continue", "return",
      "try",      "catch",    "switch",   "case",     "default",
      "new",      "this",     "print"};
  static const std::set<std::string> arithmetic_operators = {
      "+", "-", "*", "/", "%"};
  static const std::set<std::string> logical_operators = {"&&", "||", "!"};
  static const std::set<std::string> comparison_operators = {
      "==", "!=", "<", "<=", ">", ">="};
  static const std::set<std::string> delimiters = {
      "(", ")", "{", "}", "[", "]"};

  if (data_types.count(lexeme))
    return "Tipo de dato";
  if (boolean_literals.count(lexeme))
    return "Literal booleano";
  if (lexeme == "null")
    return "Literal nulo";
  if (reserved_words.count(lexeme))
    return "Palabra reservada";
  if (arithmetic_operators.count(lexeme))
    return "Operador aritmético";
  if (logical_operators.count(lexeme))
    return "Operador lógico";
  if (comparison_operators.count(lexeme))
    return "Operador de comparación";
  if (lexeme == "=")
    return "Operador de asignación";
  if (lexeme == "?")
    return "Operador ternario";
  if (lexeme == ":" || lexeme == ",")
    return "Separador";
  if (lexeme == ";")
    return "Fin de sentencia";
  if (lexeme == ".")
    return "Acceso a miembro";
  if (delimiters.count(lexeme))
    return "Delimitador";
  return "Símbolo";
}

} // namespace compiscript
