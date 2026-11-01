#pragma once

#include <map>
#include <optional>
#include <string>

namespace sql {

// Una sentencia confirma juntos CSV, posiciones, índices y catálogo.
// El diario permite recuperar una instalación interrumpida al volver a abrir.
class FileTransaction {
public:
  explicit FileTransaction(std::string root);
  void clear();
  void write(const std::string &relative, std::string content);
  void remove(const std::string &relative);
  bool commit(std::string &error);
  static bool recover(const std::string &root, std::string &error);

private:
  std::string root_;
  std::map<std::string, std::optional<std::string>> changes_;
};

} // namespace sql
