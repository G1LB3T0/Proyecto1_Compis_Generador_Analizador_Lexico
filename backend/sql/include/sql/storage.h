// Motor de almacenamiento en archivos de texto (sin ningún DBMS).
//
//   database/
//   ├── metadata.txt          catálogo: tablas, tipos, restricciones, índices
//   ├── <tabla>.csv           registros; la 1.ª columna __rowid es interna
//   └── indexes/<indice>.idx  valor -> rowids de los registros con ese valor
#pragma once

#include "sql/catalog.h"
#include "sql/file_transaction.h"
#include "sql/value.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace sql {

struct StorageLock;

using Row = std::vector<Value>;

// Registros de una tabla en memoria, ordenados por rowid (orden de inserción).
struct TableData {
  std::map<long long, Row> rows;
};

// Índice ordenado: clave = valor de la columna, dato = rowids que lo tienen.
// Al ser un árbol ordenado (std::map) sirve para =, IN y rangos <, >, <=, >=.
struct IndexData {
  std::map<Value, std::set<long long>, ValueLess> entries;

  void add(const Value &key, long long rowid);
  void remove(const Value &key, long long rowid);
  size_t size() const;
};

struct StoredFile {
  std::string path; // relativo a la carpeta de la base de datos
  size_t bytes = 0;
  std::string content;
  bool truncated = false;
};

struct StorageReads {
  size_t rows = 0;
  size_t bytes = 0;
};

class Storage {
public:
  explicit Storage(std::string root);
  ~Storage();

  // Crea las carpetas si faltan y carga metadata.txt.
  bool open(std::string &error);

  Catalog &catalog() { return catalog_; }
  const Catalog &catalog() const { return catalog_; }
  const std::string &root() const { return root_; }

  std::string metadataPath() const;
  std::string tablePath(const std::string &table) const;
  std::string indexPath(const std::string &index) const;
  std::string positionsPath(const std::string &table) const;
  std::string relative(const std::string &path) const;

  // Carga perezosa desde disco; las siguientes llamadas usan la copia en memoria.
  TableData *table(const std::string &name, std::string &error);
  IndexData *index(const std::string &name, std::string &error);
  // Lee candidatos mediante posiciones de byte, sin cargar todo el CSV.
  bool readRows(const std::string &name, const std::set<long long> &rowids,
                TableData &result, std::string &error);
  size_t rowCount(const std::string &name, std::string &error);
  const StorageReads &reads() const { return reads_; }

  void beginTransaction();
  bool commitTransaction(std::string &error);

  void putTable(const std::string &name, TableData data);
  void putIndex(const std::string &name, IndexData data);

  bool saveCatalog(std::string &error);
  bool saveTable(const std::string &name, std::string &error);
  bool saveIndex(const std::string &name, std::string &error);
  bool removeTable(const std::string &name, std::string &error);
  bool removeIndex(const std::string &name, std::string &error);

  // Construye un índice recorriendo una vez todos los registros de la tabla.
  IndexData buildIndex(const IndexSchema &schema, const TableData &data) const;

  // Borra metadata, tablas e índices (botón "Reiniciar base de datos").
  bool reset(std::string &error);

  // Archivos actuales con su contenido, para mostrarlos en el IDE.
  std::vector<StoredFile> files(size_t max_bytes = 20000) const;

private:
  struct RowPosition {
    size_t offset = 0;
    size_t bytes = 0;
  };
  using RowDirectory = std::map<long long, RowPosition>;
  const RowDirectory *positions(const std::string &name, std::string &error);
  std::string root_;
  std::unique_ptr<StorageLock> lock_;
  FileTransaction transaction_;
  Catalog catalog_;
  std::map<std::string, TableData> tables_;
  std::map<std::string, IndexData> indexes_;
  std::map<std::string, RowDirectory> positions_;
  StorageReads reads_;
};

// Codificación CSV de un valor: NULL es un campo vacío y el texto siempre va
// entre comillas, así '' (texto vacío) se distingue de NULL.
std::string encodeCsvValue(const Value &value);

} // namespace sql
