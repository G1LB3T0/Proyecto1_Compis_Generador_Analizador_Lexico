// Implementa tipos y valores SQL: texto, comparación y conversión a columnas.
#include "sql/value.h"

#include <cmath>
#include <cstdio>
#include <sstream>

namespace sql {

std::string DataType::toString() const {
  switch (base) {
  case BaseType::Int:
    return "INT";
  case BaseType::Float:
    return "FLOAT";
  case BaseType::Varchar:
    return "VARCHAR(" + std::to_string(length) + ")";
  case BaseType::Boolean:
    return "BOOLEAN";
  case BaseType::Date:
    return "DATE";
  case BaseType::Null:
    return "NULL";
  case BaseType::Unknown:
    break;
  }
  return "desconocido";
}

std::optional<DataType> DataType::parse(const std::string &text) {
  if (text == "INT")
    return integer();
  if (text == "FLOAT")
    return real();
  if (text == "BOOLEAN")
    return boolean();
  if (text == "DATE")
    return date();
  if (text.rfind("VARCHAR(", 0) == 0 && text.back() == ')') {
    try {
      return varchar(std::stoi(text.substr(8, text.size() - 9)));
    } catch (...) {
      return std::nullopt;
    }
  }
  return std::nullopt;
}

Value Value::makeInt(long long v) {
  Value value;
  value.kind = Kind::Int;
  value.integer = v;
  return value;
}

Value Value::makeFloat(double v) {
  Value value;
  value.kind = Kind::Float;
  value.real = v;
  return value;
}

Value Value::makeText(std::string v) {
  Value value;
  value.kind = Kind::Text;
  value.text = std::move(v);
  return value;
}

Value Value::makeBool(bool v) {
  Value value;
  value.kind = Kind::Boolean;
  value.boolean = v;
  return value;
}

Value Value::makeDate(std::string v) {
  Value value;
  value.kind = Kind::Date;
  value.text = std::move(v);
  return value;
}

std::string formatFloat(double value) {
  if (std::isfinite(value) && value == std::floor(value) &&
      std::fabs(value) < 1e15) {
    std::ostringstream output;
    output << static_cast<long long>(value) << ".0";
    return output.str();
  }
  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), "%.10g", value);
  return buffer;
}

std::string Value::toString() const {
  switch (kind) {
  case Kind::Null:
    return "NULL";
  case Kind::Int:
    return std::to_string(integer);
  case Kind::Float:
    return formatFloat(real);
  case Kind::Text:
  case Kind::Date:
    return text;
  case Kind::Boolean:
    return boolean ? "TRUE" : "FALSE";
  }
  return "";
}

std::string Value::typeName() const {
  switch (kind) {
  case Kind::Null:
    return "NULL";
  case Kind::Int:
    return "INT";
  case Kind::Float:
    return "FLOAT";
  case Kind::Text:
    return "VARCHAR";
  case Kind::Boolean:
    return "BOOLEAN";
  case Kind::Date:
    return "DATE";
  }
  return "desconocido";
}

namespace {

// Orden entre familias distintas: solo se usa para que std::map sea estable.
int familyRank(const Value &value) {
  switch (value.kind) {
  case Value::Kind::Null:
    return 0;
  case Value::Kind::Boolean:
    return 1;
  case Value::Kind::Int:
  case Value::Kind::Float:
    return 2;
  case Value::Kind::Date:
    return 3;
  case Value::Kind::Text:
    return 4;
  }
  return 5;
}

} // namespace

int compareValues(const Value &left, const Value &right) {
  const int left_rank = familyRank(left);
  const int right_rank = familyRank(right);
  if (left_rank != right_rank)
    return left_rank < right_rank ? -1 : 1;
  switch (left.kind) {
  case Value::Kind::Null:
    return 0;
  case Value::Kind::Boolean:
    return left.boolean == right.boolean ? 0 : (left.boolean ? 1 : -1);
  case Value::Kind::Int:
  case Value::Kind::Float:
    if (left.kind == Value::Kind::Int && right.kind == Value::Kind::Int)
      return left.integer == right.integer ? 0
                                           : (left.integer < right.integer ? -1 : 1);
    if (left.asDouble() == right.asDouble())
      return 0;
    return left.asDouble() < right.asDouble() ? -1 : 1;
  case Value::Kind::Date:
  case Value::Kind::Text:
    // YYYY-MM-DD se ordena correctamente de forma lexicográfica.
    return left.text == right.text ? 0 : (left.text < right.text ? -1 : 1);
  }
  return 0;
}

bool valuesEqual(const Value &left, const Value &right) {
  return compareValues(left, right) == 0;
}

bool isValidDate(const std::string &text) {
  if (text.size() != 10 || text[4] != '-' || text[7] != '-')
    return false;
  for (size_t index : {0, 1, 2, 3, 5, 6, 8, 9}) {
    if (text[index] < '0' || text[index] > '9')
      return false;
  }
  const int year = std::stoi(text.substr(0, 4));
  const int month = std::stoi(text.substr(5, 2));
  const int day = std::stoi(text.substr(8, 2));
  if (month < 1 || month > 12 || day < 1)
    return false;
  static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
  const int limit = month == 2 && leap ? 29 : days[month - 1];
  return day <= limit;
}

std::optional<Value> coerceToType(const Value &value, const DataType &type,
                                  std::string &error) {
  if (value.isNull())
    return value;
  switch (type.base) {
  case BaseType::Int:
    if (value.kind == Value::Kind::Int)
      return value;
    break;
  case BaseType::Float:
    if (value.kind == Value::Kind::Float)
      return value;
    if (value.kind == Value::Kind::Int)
      return Value::makeFloat(static_cast<double>(value.integer));
    break;
  case BaseType::Varchar:
    if (value.kind == Value::Kind::Text) {
      if (static_cast<int>(value.text.size()) > type.length) {
        error = "el texto '" + value.text + "' tiene " +
                std::to_string(value.text.size()) +
                " caracteres y excede " + type.toString();
        return std::nullopt;
      }
      return value;
    }
    break;
  case BaseType::Boolean:
    if (value.kind == Value::Kind::Boolean)
      return value;
    break;
  case BaseType::Date:
    if (value.kind == Value::Kind::Date)
      return value;
    if (value.kind == Value::Kind::Text) {
      if (isValidDate(value.text))
        return Value::makeDate(value.text);
      error = "'" + value.text + "' no es una fecha válida con formato YYYY-MM-DD";
      return std::nullopt;
    }
    break;
  case BaseType::Null:
  case BaseType::Unknown:
    return value;
  }
  error = "el valor " + value.toString() + " de tipo " + value.typeName() +
          " no es compatible con " + type.toString();
  return std::nullopt;
}

} // namespace sql
