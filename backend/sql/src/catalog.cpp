// Mantiene el diccionario de datos y lo guarda como texto legible.
//
// Formato de metadata.txt (una instrucción por línea):
//   TABLE estudiantes
//     NEXT_ROWID 4
//     COLUMN id INT PRIMARY_KEY NOT_NULL
//     COLUMN nombre VARCHAR(60) NOT_NULL
//     COLUMN carrera_id INT REFERENCES carreras.id
//   END
//   INDEX pk_estudiantes ON estudiantes.id UNIQUE AUTO
#include "sql/catalog.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace sql {

int TableSchema::columnIndex(const std::string &column) const {
  for (size_t index = 0; index < columns.size(); ++index) {
    if (columns[index].name == column)
      return static_cast<int>(index);
  }
  return -1;
}

const ColumnSchema *TableSchema::column(const std::string &column) const {
  const int index = columnIndex(column);
  return index < 0 ? nullptr : &columns[index];
}

const TableSchema *Catalog::table(const std::string &name) const {
  auto found = tables_.find(name);
  return found == tables_.end() ? nullptr : &found->second;
}

TableSchema *Catalog::table(const std::string &name) {
  auto found = tables_.find(name);
  return found == tables_.end() ? nullptr : &found->second;
}

const IndexSchema *Catalog::index(const std::string &name) const {
  auto found = indexes_.find(name);
  return found == indexes_.end() ? nullptr : &found->second;
}

const IndexSchema *Catalog::indexOn(const std::string &table,
                                    const std::string &column) const {
  for (const auto &[name, index] : indexes_) {
    if (index.table == table && index.column == column)
      return &index;
  }
  return nullptr;
}

std::vector<const IndexSchema *> Catalog::indexesOf(const std::string &table) const {
  std::vector<const IndexSchema *> result;
  for (const auto &[name, index] : indexes_) {
    if (index.table == table)
      result.push_back(&index);
  }
  return result;
}

std::vector<std::string> Catalog::referencingTables(const std::string &table) const {
  std::vector<std::string> result;
  for (const auto &[name, schema] : tables_) {
    if (name == table)
      continue;
    for (const auto &column : schema.columns) {
      if (column.ref_table == table) {
        result.push_back(name);
        break;
      }
    }
  }
  return result;
}

void Catalog::addTable(TableSchema schema) {
  if (!tables_.count(schema.name))
    table_order_.push_back(schema.name);
  tables_[schema.name] = std::move(schema);
}

void Catalog::dropTable(const std::string &name) {
  tables_.erase(name);
  table_order_.erase(std::remove(table_order_.begin(), table_order_.end(), name),
                     table_order_.end());
  for (auto iterator = indexes_.begin(); iterator != indexes_.end();) {
    if (iterator->second.table == name)
      iterator = indexes_.erase(iterator);
    else
      ++iterator;
  }
}

void Catalog::addIndex(IndexSchema index) { indexes_[index.name] = std::move(index); }

void Catalog::dropIndex(const std::string &name) { indexes_.erase(name); }

bool Catalog::load(const std::string &path, std::string &error) {
  tables_.clear();
  indexes_.clear();
  table_order_.clear();
  std::ifstream file(path);
  if (!file)
    return true; // base de datos nueva

  TableSchema current;
  bool inside_table = false;
  std::string line;
  size_t number = 0;
  while (std::getline(file, line)) {
    ++number;
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    std::istringstream words(line);
    std::string keyword;
    if (!(words >> keyword) || keyword[0] == '#')
      continue;
    if (keyword == "TABLE") {
      current = TableSchema{};
      words >> current.name;
      inside_table = true;
    } else if (keyword == "NEXT_ROWID" && inside_table) {
      words >> current.next_rowid;
    } else if (keyword == "COLUMN" && inside_table) {
      ColumnSchema column;
      std::string type;
      words >> column.name >> type;
      auto parsed = DataType::parse(type);
      if (!parsed) {
        error = "metadata.txt línea " + std::to_string(number) +
                ": tipo desconocido " + type;
        return false;
      }
      column.type = *parsed;
      std::string flag;
      while (words >> flag) {
        if (flag == "PRIMARY_KEY")
          column.primary_key = true;
        else if (flag == "NOT_NULL")
          column.not_null = true;
        else if (flag == "UNIQUE")
          column.unique = true;
        else if (flag == "REFERENCES") {
          std::string target;
          words >> target;
          const size_t dot = target.find('.');
          column.ref_table = target.substr(0, dot);
          column.ref_column = dot == std::string::npos ? "" : target.substr(dot + 1);
        }
      }
      current.columns.push_back(column);
    } else if (keyword == "END" && inside_table) {
      addTable(current);
      inside_table = false;
    } else if (keyword == "INDEX") {
      IndexSchema index;
      std::string on, target, flag;
      words >> index.name >> on >> target;
      const size_t dot = target.find('.');
      index.table = target.substr(0, dot);
      index.column = dot == std::string::npos ? "" : target.substr(dot + 1);
      while (words >> flag) {
        if (flag == "UNIQUE")
          index.unique = true;
        else if (flag == "AUTO")
          index.automatic = true;
      }
      addIndex(index);
    } else {
      error = "metadata.txt línea " + std::to_string(number) +
              ": instrucción no reconocida '" + keyword + "'";
      return false;
    }
  }
  return true;
}

bool Catalog::save(const std::string &path, std::string &error) const {
  std::ostringstream output;
  output << "# Catalogo del interprete SQL (generado automaticamente).\n"
         << "# TABLE/COLUMN describen esquemas, tipos y restricciones;\n"
         << "# INDEX describe los indices guardados en indexes/<nombre>.idx.\n";
  for (const auto &name : table_order_) {
    const TableSchema &schema = tables_.at(name);
    output << "\nTABLE " << schema.name << '\n'
           << "  NEXT_ROWID " << schema.next_rowid << '\n';
    for (const auto &column : schema.columns) {
      output << "  COLUMN " << column.name << ' ' << column.type.toString();
      if (column.primary_key)
        output << " PRIMARY_KEY";
      if (column.not_null)
        output << " NOT_NULL";
      if (column.unique)
        output << " UNIQUE";
      if (!column.ref_table.empty())
        output << " REFERENCES " << column.ref_table << '.' << column.ref_column;
      output << '\n';
    }
    output << "END\n";
  }
  if (!indexes_.empty())
    output << '\n';
  for (const auto &[name, index] : indexes_) {
    output << "INDEX " << index.name << " ON " << index.table << '.'
           << index.column;
    if (index.unique)
      output << " UNIQUE";
    if (index.automatic)
      output << " AUTO";
    output << '\n';
  }

  // Escritura atómica: primero un temporal y luego se reemplaza el original.
  const std::string temporary = path + ".tmp";
  {
    std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
    if (!file) {
      error = "no se pudo escribir " + temporary;
      return false;
    }
    file << output.str();
  }
  std::remove(path.c_str());
  if (std::rename(temporary.c_str(), path.c_str()) != 0) {
    error = "no se pudo reemplazar " + path;
    return false;
  }
  return true;
}

} // namespace sql
