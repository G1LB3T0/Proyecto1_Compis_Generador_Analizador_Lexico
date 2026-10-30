// Tipos de datos SQL (INT, FLOAT, VARCHAR(n), BOOLEAN, DATE) y valores en
// tiempo de ejecución. Los usa la semántica, el intérprete y el almacenamiento.
#pragma once

#include <optional>
#include <string>

namespace sql {

enum class BaseType { Int, Float, Varchar, Boolean, Date, Null, Unknown };

// Tipo declarado de una columna o inferido para una expresión.
struct DataType {
  BaseType base = BaseType::Unknown;
  int length = 0; // solo para VARCHAR(n)

  static DataType integer() { return {BaseType::Int, 0}; }
  static DataType real() { return {BaseType::Float, 0}; }
  static DataType varchar(int n) { return {BaseType::Varchar, n}; }
  static DataType boolean() { return {BaseType::Boolean, 0}; }
  static DataType date() { return {BaseType::Date, 0}; }
  static DataType null() { return {BaseType::Null, 0}; }
  static DataType unknown() { return {BaseType::Unknown, 0}; }

  bool isNumeric() const {
    return base == BaseType::Int || base == BaseType::Float;
  }
  bool isNull() const { return base == BaseType::Null; }
  bool isUnknown() const { return base == BaseType::Unknown; }
  // Texto usado en metadata.txt y en el IDE: INT, VARCHAR(30), DATE...
  std::string toString() const;
  // Lee el texto producido por toString().
  static std::optional<DataType> parse(const std::string &text);
};

// Valor de una celda o de una expresión evaluada. NULL es un valor más.
struct Value {
  enum class Kind { Null, Int, Float, Text, Boolean, Date };
  Kind kind = Kind::Null;
  long long integer = 0;
  double real = 0.0;
  bool boolean = false;
  std::string text; // VARCHAR y DATE (YYYY-MM-DD)

  static Value makeNull() { return {}; }
  static Value makeInt(long long v);
  static Value makeFloat(double v);
  static Value makeText(std::string v);
  static Value makeBool(bool v);
  static Value makeDate(std::string v);

  bool isNull() const { return kind == Kind::Null; }
  bool isNumeric() const { return kind == Kind::Int || kind == Kind::Float; }
  double asDouble() const {
    return kind == Kind::Int ? static_cast<double>(integer) : real;
  }
  // Representación legible: NULL, 18, 3.5, Ana, TRUE, 2024-03-01.
  std::string toString() const;
  // Nombre del tipo del valor en mensajes de error.
  std::string typeName() const;
};

// Orden total entre valores comparables; NULL queda antes que todo.
int compareValues(const Value &left, const Value &right);

// Comparador para std::map: es el orden de las claves de un índice.
struct ValueLess {
  bool operator()(const Value &left, const Value &right) const {
    return compareValues(left, right) < 0;
  }
};

bool valuesEqual(const Value &left, const Value &right);

// Valida el formato YYYY-MM-DD y que el día exista en el calendario.
bool isValidDate(const std::string &text);

// Convierte un valor al tipo de una columna antes de guardarlo. Devuelve
// std::nullopt y describe el problema en `error` si no es compatible.
std::optional<Value> coerceToType(const Value &value, const DataType &type,
                                  std::string &error);

std::string formatFloat(double value);

} // namespace sql
