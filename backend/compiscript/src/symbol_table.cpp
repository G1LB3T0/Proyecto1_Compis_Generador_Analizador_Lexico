// Reglas compartidas para construir listas y comprobar compatibilidad de tipos.
#include "compiscript/model.h"

namespace compiscript {

Type listOf(const Type &element) { return Type{element.name + "[]"}; }

Type commonType(const Type &left, const Type &right) {
  if (left.isUnknown())
    return right;
  if (right.isUnknown())
    return left;
  if (left == right)
    return left;
  if (left.isNumeric() && right.isNumeric())
    return TYPE_FLOAT;
  return TYPE_ERROR;
}

bool assignable(const Type &target, const Type &value,
                const std::set<std::string> &class_names) {
  if (target.isUnknown() || value.isUnknown())
    return true;
  if (target == value)
    return true;
  if (target == TYPE_FLOAT && value == TYPE_INTEGER)
    return true;
  if (target.isList() && value.isList()) {
    return assignable(target.element(), value.element(), class_names);
  }
  if (value == TYPE_NULL && (target.isList() || target == TYPE_STRING ||
                             class_names.count(target.name) != 0)) {
    return true;
  }
  return false;
}

} // namespace compiscript
