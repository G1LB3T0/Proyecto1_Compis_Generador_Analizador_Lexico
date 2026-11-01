#include "sql/file_transaction.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace sql {
namespace {

struct Entry {
  std::string path;
  bool existed = false;
  bool write = false;
};

void checkedWrite(const fs::path &path, const std::string &content) {
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file)
    throw std::runtime_error("no se pudo escribir " + path.string());
  file << content;
  file.close();
  if (!file)
    throw std::runtime_error("no se pudo completar la escritura de " + path.string());
}

void cleanup(const fs::path &root, const std::vector<Entry> &entries) {
  for (const auto &entry : entries) {
    if (entry.write) {
      const fs::path temporary = (root / entry.path).string() + ".tmp";
      // No borrar directorios ajenos que hayan causado un fallo de escritura.
      if (fs::is_regular_file(temporary))
        fs::remove(temporary);
    }
  }
  // Renombrar antes de limpiar impide confundir una limpieza interrumpida
  // (con respaldos ya borrados) con una instalación pendiente de rollback.
  fs::rename(root / ".transaction", root / ".transaction.done");
  fs::remove_all(root / ".transaction.done");
}

} // namespace

FileTransaction::FileTransaction(std::string root) : root_(std::move(root)) {}
void FileTransaction::clear() { changes_.clear(); }
void FileTransaction::write(const std::string &relative, std::string content) {
  changes_[relative] = std::move(content);
}
void FileTransaction::remove(const std::string &relative) { changes_[relative] = std::nullopt; }

bool FileTransaction::recover(const std::string &root_name, std::string &error) {
  const fs::path root(root_name), journal = root / ".transaction";
  try {
    if (fs::exists(root / ".transaction.done"))
      fs::remove_all(root / ".transaction.done");
    if (!fs::exists(journal))
      return true;
    std::vector<Entry> entries;
    std::ifstream manifest(journal / "manifest");
    std::string line;
    while (std::getline(manifest, line)) {
      std::istringstream record(line);
      Entry entry;
      if (!(record >> entry.existed >> entry.write >> entry.path) ||
          fs::path(entry.path).is_absolute() || entry.path.find("..") != std::string::npos) {
        if (fs::exists(journal / "ready"))
          throw std::runtime_error("el diario de recuperación contiene una entrada inválida");
        break; // escritura del manifiesto interrumpida antes de tocar originales
      }
      entries.push_back(entry);
    }
    // READY solo aparece después de terminar todos los respaldos y temporales.
    if (fs::exists(journal / "ready") && !fs::exists(journal / "committed")) {
      if (entries.empty())
        throw std::runtime_error("falta el manifiesto del diario de recuperación");
      for (const auto &entry : entries) {
        const fs::path target = root / entry.path;
        if (entry.existed)
          fs::copy_file(journal / "backup" / entry.path, target,
                        fs::copy_options::overwrite_existing);
        else
          fs::remove(target);
      }
    }
    cleanup(root, entries);
    return true;
  } catch (const std::exception &failure) {
    error = "no se pudo recuperar la sentencia: " + std::string(failure.what());
    return false; // conservar el diario y sus respaldos para reintentar
  }
}

bool FileTransaction::commit(std::string &error) {
  if (changes_.empty())
    return true;
  const fs::path root(root_), journal = root / ".transaction";
  try {
    if (!recover(root_, error))
      return false;
    std::vector<Entry> entries;
    std::ostringstream manifest;
    for (const auto &[path, content] : changes_) {
      const fs::path target = root / path;
      const bool existed = fs::exists(target);
      if (existed && !fs::is_regular_file(target))
        throw std::runtime_error("el destino no es un archivo: " + target.string());
      entries.push_back({path, existed, content.has_value()});
      manifest << existed << ' ' << content.has_value() << ' ' << path << '\n';
    }
    fs::create_directory(journal);
    checkedWrite(journal / "manifest", manifest.str());
    for (const auto &entry : entries) {
      const fs::path target = root / entry.path;
      if (entry.existed) {
        const fs::path backup = journal / "backup" / entry.path;
        fs::create_directories(backup.parent_path());
        fs::copy_file(target, backup);
      }
      if (entry.write)
        checkedWrite(target.string() + ".tmp", *changes_.at(entry.path));
    }
    // Preparar el contenido del marcador antes de instalar archivos: una
    // escritura fallida nunca debe dejar un COMMITTED vacío y dar por válida
    // una sentencia que se está intentando revertir.
    checkedWrite(journal / "commit-marker", "committed\n");
    checkedWrite(journal / "ready", "ready\n");
    for (const auto &entry : entries) {
      const fs::path target = root / entry.path;
      // El respaldo ya existe; funciona también donde rename no reemplaza archivos.
      fs::remove(target);
      if (entry.write)
        fs::rename(target.string() + ".tmp", target);
    }
    fs::rename(journal / "commit-marker", journal / "committed");
    changes_.clear();
    // La confirmación ya ocurrió. Una limpieza pendiente se completa al abrir.
    try {
      cleanup(root, entries);
    } catch (const std::exception &) {
      // El marcador COMMITTED sigue distinguiendo esta sentencia confirmada.
    }
    return true;
  } catch (const std::exception &failure) {
    error = "no se pudo guardar la sentencia: " + std::string(failure.what());
    std::string recovery_error;
    if (!recover(root_, recovery_error))
      error += "; " + recovery_error;
    changes_.clear();
    return false;
  }
}

} // namespace sql
