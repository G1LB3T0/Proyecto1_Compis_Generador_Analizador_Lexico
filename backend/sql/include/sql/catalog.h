// Catálogo (diccionario de datos): esquemas de tablas, tipos, restricciones e
// índices. Se persiste en database/metadata.txt.
#pragma once

#include "sql/value.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sql {

struct ColumnSchema {
  std::string name;
  DataType type;
  bool primary_key = false;
  bool not_null = false;
  bool unique = false;
  std::string ref_table; // clave foránea: REFERENCES ref_table(ref_column)
  std::string ref_column;
};

struct TableSchema {
  std::string name;
  std::vector<ColumnSchema> columns;
  long long next_rowid = 1; // identificador interno estable de cada registro

  int columnIndex(const std::string &column) const;
  const ColumnSchema *column(const std::string &column) const;
};

struct IndexSchema {
  std::string name;
  std::string table;
  std::string column;
  bool unique = false;
  bool automatic = false; // creado por PRIMARY KEY o UNIQUE
};

class Catalog {
public:
  // Lee metadata.txt; si no existe el catálogo queda vacío.
  bool load(const std::string &path, std::string &error);
  std::string serialize() const;

  const TableSchema *table(const std::string &name) const;
  TableSchema *table(const std::string &name);
  const IndexSchema *index(const std::string &name) const;
  // Índice (manual o automático) definido sobre tabla.columna, si existe.
  const IndexSchema *indexOn(const std::string &table,
                             const std::string &column) const;
  std::vector<const IndexSchema *> indexesOf(const std::string &table) const;
  // Tablas cuyas claves foráneas apuntan a `table` (excluyendo a ella misma).
  std::vector<std::string> referencingTables(const std::string &table) const;

  void addTable(TableSchema schema);
  void dropTable(const std::string &name);
  void addIndex(IndexSchema index);
  void dropIndex(const std::string &name);

  const std::map<std::string, TableSchema> &tables() const { return tables_; }
  const std::map<std::string, IndexSchema> &indexes() const { return indexes_; }

private:
  std::map<std::string, TableSchema> tables_;
  std::map<std::string, IndexSchema> indexes_;
  std::vector<std::string> table_order_; // orden de creación, para mostrar
};

// Columna visible dentro de una consulta: a qué tabla/alias pertenece.
struct BoundColumn {
  std::string alias; // alias de la tabla en FROM/JOIN
  std::string table;
  std::string column;
  DataType type;
};

// Fila combinada de una consulta: columnas de FROM seguidas de las de cada
// JOIN, en ese orden. `slot` en el AST indexa esta lista.
struct RowLayout {
  std::vector<BoundColumn> columns;
  std::vector<std::string> aliases; // una entrada por tabla participante
  std::vector<size_t> offsets;      // primera posición de cada tabla
};

} // namespace sql
