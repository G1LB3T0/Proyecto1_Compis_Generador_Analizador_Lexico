// Lee y escribe tablas (.csv), índices (.idx) y el catálogo (metadata.txt).
#include "sql/storage.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace sql {

namespace {

struct CsvField {
  std::string text;
  bool quoted = false;
};

// Separa un archivo CSV completo en registros; respeta comillas, comillas
// dobles escapadas ("") y saltos de línea dentro de un texto.
std::vector<std::vector<CsvField>> parseCsv(const std::string &content) {
  std::vector<std::vector<CsvField>> records;
  std::vector<CsvField> record;
  CsvField field;
  bool in_quotes = false;
  bool field_started = false;
  auto end_field = [&]() {
    record.push_back(field);
    field = CsvField{};
    field_started = false;
  };
  auto end_record = [&]() {
    if (field_started || !record.empty())
      end_field();
    if (!record.empty())
      records.push_back(record);
    record.clear();
  };
  for (size_t index = 0; index < content.size(); ++index) {
    const char character = content[index];
    if (in_quotes) {
      if (character == '"') {
        if (index + 1 < content.size() && content[index + 1] == '"') {
          field.text += '"';
          ++index;
        } else {
          in_quotes = false;
        }
      } else {
        field.text += character;
      }
      continue;
    }
    if (character == '"') {
      in_quotes = true;
      field.quoted = true;
      field_started = true;
    } else if (character == ',') {
      end_field();
      field_started = true; // el siguiente campo existe aunque esté vacío
    } else if (character == '\n') {
      end_record();
    } else if (character != '\r') {
      field.text += character;
      field_started = true;
    }
  }
  end_record();
  return records;
}

std::string quoteCsv(const std::string &text) {
  std::string result = "\"";
  for (char character : text) {
    if (character == '"')
      result += '"';
    result += character;
  }
  return result + '"';
}

// Interpreta un campo según el tipo declarado de la columna.
Value decodeField(const CsvField &field, const DataType &type) {
  if (!field.quoted && field.text.empty())
    return Value::makeNull();
  switch (type.base) {
  case BaseType::Int:
    return Value::makeInt(std::stoll(field.text));
  case BaseType::Float:
    return Value::makeFloat(std::stod(field.text));
  case BaseType::Boolean:
    return Value::makeBool(field.text == "TRUE");
  case BaseType::Date:
    return Value::makeDate(field.text);
  default:
    return Value::makeText(field.text);
  }
}

bool readFile(const std::string &path, std::string &content) {
  std::ifstream file(path, std::ios::binary);
  if (!file)
    return false;
  std::ostringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();
  return true;
}

// Escritura atómica: si el proceso se interrumpe no queda un archivo a medias.
bool writeFile(const std::string &path, const std::string &content,
               std::string &error) {
  const std::string temporary = path + ".tmp";
  {
    std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
    if (!file) {
      error = "no se pudo escribir " + temporary;
      return false;
    }
    file << content;
  }
  std::remove(path.c_str());
  if (std::rename(temporary.c_str(), path.c_str()) != 0) {
    error = "no se pudo reemplazar " + path;
    return false;
  }
  return true;
}

} // namespace

std::string encodeCsvValue(const Value &value) {
  switch (value.kind) {
  case Value::Kind::Null:
    return "";
  case Value::Kind::Text:
    return quoteCsv(value.text);
  default:
    return value.toString();
  }
}

void IndexData::add(const Value &key, long long rowid) {
  if (!key.isNull())
    entries[key].insert(rowid);
}

void IndexData::remove(const Value &key, long long rowid) {
  auto found = entries.find(key);
  if (found == entries.end())
    return;
  found->second.erase(rowid);
  if (found->second.empty())
    entries.erase(found);
}

size_t IndexData::size() const {
  size_t total = 0;
  for (const auto &[key, rowids] : entries)
    total += rowids.size();
  return total;
}

Storage::Storage(std::string root) : root_(std::move(root)) {}

std::string Storage::metadataPath() const {
  return (fs::path(root_) / "metadata.txt").string();
}

std::string Storage::tablePath(const std::string &table) const {
  return (fs::path(root_) / (table + ".csv")).string();
}

std::string Storage::indexPath(const std::string &index) const {
  return (fs::path(root_) / "indexes" / (index + ".idx")).string();
}

std::string Storage::relative(const std::string &path) const {
  return fs::path(path).lexically_relative(root_).generic_string();
}

bool Storage::open(std::string &error) {
  std::error_code code;
  fs::create_directories(fs::path(root_) / "indexes", code);
  if (code) {
    error = "no se pudo crear la carpeta " + root_ + ": " + code.message();
    return false;
  }
  tables_.clear();
  indexes_.clear();
  return catalog_.load(metadataPath(), error);
}

TableData *Storage::table(const std::string &name, std::string &error) {
  auto cached = tables_.find(name);
  if (cached != tables_.end())
    return &cached->second;
  const TableSchema *schema = catalog_.table(name);
  if (!schema) {
    error = "la tabla " + name + " no existe en el catálogo";
    return nullptr;
  }
  TableData data;
  std::string content;
  if (readFile(tablePath(name), content)) {
    const auto records = parseCsv(content);
    // records[0] es el encabezado: __rowid, columnas...
    for (size_t index = 1; index < records.size(); ++index) {
      const auto &record = records[index];
      if (record.size() != schema->columns.size() + 1) {
        error = name + ".csv registro " + std::to_string(index) +
                ": se esperaban " + std::to_string(schema->columns.size() + 1) +
                " campos";
        return nullptr;
      }
      Row row;
      try {
        for (size_t column = 0; column < schema->columns.size(); ++column)
          row.push_back(decodeField(record[column + 1], schema->columns[column].type));
        data.rows[std::stoll(record[0].text)] = std::move(row);
      } catch (...) {
        error = name + ".csv registro " + std::to_string(index) +
                ": valor ilegible";
        return nullptr;
      }
    }
  }
  return &(tables_[name] = std::move(data));
}

IndexData *Storage::index(const std::string &name, std::string &error) {
  auto cached = indexes_.find(name);
  if (cached != indexes_.end())
    return &cached->second;
  const IndexSchema *schema = catalog_.index(name);
  if (!schema) {
    error = "el índice " + name + " no existe en el catálogo";
    return nullptr;
  }
  const TableSchema *table_schema = catalog_.table(schema->table);
  const ColumnSchema *column = table_schema ? table_schema->column(schema->column) : nullptr;
  if (!column) {
    error = "el índice " + name + " apunta a una columna inexistente";
    return nullptr;
  }
  std::string content;
  if (!readFile(indexPath(name), content)) {
    // Si el archivo se perdió, se reconstruye a partir de la tabla.
    TableData *data = table(schema->table, error);
    if (!data)
      return nullptr;
    return &(indexes_[name] = buildIndex(*schema, *data));
  }
  IndexData data;
  // Cada registro: clave,rowid rowid rowid
  std::string filtered;
  std::istringstream lines(content);
  std::string line;
  while (std::getline(lines, line)) {
    if (!line.empty() && line[0] != '#')
      filtered += line + '\n';
  }
  try {
    for (const auto &record : parseCsv(filtered)) {
      if (record.size() != 2)
        continue;
      const Value key = decodeField(record[0], column->type);
      std::istringstream rowids(record[1].text);
      long long rowid = 0;
      while (rowids >> rowid)
        data.add(key, rowid);
    }
  } catch (...) {
    error = "indexes/" + name + ".idx contiene una clave ilegible";
    return nullptr;
  }
  return &(indexes_[name] = std::move(data));
}

void Storage::putTable(const std::string &name, TableData data) {
  tables_[name] = std::move(data);
}

void Storage::putIndex(const std::string &name, IndexData data) {
  indexes_[name] = std::move(data);
}

bool Storage::saveCatalog(std::string &error) const {
  return catalog_.save(metadataPath(), error);
}

bool Storage::saveTable(const std::string &name, std::string &error) {
  const TableSchema *schema = catalog_.table(name);
  TableData *data = table(name, error);
  if (!schema || !data)
    return false;
  std::ostringstream output;
  output << "__rowid";
  for (const auto &column : schema->columns)
    output << ',' << column.name;
  output << '\n';
  for (const auto &[rowid, row] : data->rows) {
    output << rowid;
    for (const auto &value : row)
      output << ',' << encodeCsvValue(value);
    output << '\n';
  }
  return writeFile(tablePath(name), output.str(), error);
}

bool Storage::saveIndex(const std::string &name, std::string &error) {
  const IndexSchema *schema = catalog_.index(name);
  IndexData *data = index(name, error);
  if (!schema || !data)
    return false;
  std::ostringstream output;
  output << "# INDEX " << name << " ON " << schema->table << '(' << schema->column
         << ')' << (schema->unique ? " UNIQUE" : "") << '\n'
         << "# valor,rowids (ordenado por valor)\n";
  for (const auto &[key, rowids] : data->entries) {
    output << encodeCsvValue(key) << ',';
    bool first = true;
    for (long long rowid : rowids) {
      output << (first ? "" : " ") << rowid;
      first = false;
    }
    output << '\n';
  }
  return writeFile(indexPath(name), output.str(), error);
}

bool Storage::removeTable(const std::string &name, std::string &error) {
  tables_.erase(name);
  std::error_code code;
  fs::remove(tablePath(name), code);
  if (code) {
    error = "no se pudo eliminar " + tablePath(name) + ": " + code.message();
    return false;
  }
  return true;
}

bool Storage::removeIndex(const std::string &name, std::string &error) {
  indexes_.erase(name);
  std::error_code code;
  fs::remove(indexPath(name), code);
  if (code) {
    error = "no se pudo eliminar " + indexPath(name) + ": " + code.message();
    return false;
  }
  return true;
}

IndexData Storage::buildIndex(const IndexSchema &schema, const TableData &data) const {
  IndexData index;
  const TableSchema *table_schema = catalog_.table(schema.table);
  const int column = table_schema ? table_schema->columnIndex(schema.column) : -1;
  if (column < 0)
    return index;
  for (const auto &[rowid, row] : data.rows)
    index.add(row[column], rowid);
  return index;
}

bool Storage::reset(std::string &error) {
  std::error_code code;
  for (const auto &entry : fs::directory_iterator(root_, code)) {
    const auto extension = entry.path().extension().string();
    if (entry.is_regular_file() &&
        (extension == ".csv" || entry.path().filename() == "metadata.txt"))
      fs::remove(entry.path(), code);
  }
  for (const auto &entry : fs::directory_iterator(fs::path(root_) / "indexes", code)) {
    if (entry.is_regular_file() && entry.path().extension() == ".idx")
      fs::remove(entry.path(), code);
  }
  if (code) {
    error = "no se pudo reiniciar la base de datos: " + code.message();
    return false;
  }
  return open(error);
}

std::vector<StoredFile> Storage::files(size_t max_bytes) const {
  std::vector<StoredFile> result;
  std::vector<fs::path> paths;
  std::error_code code;
  if (fs::exists(metadataPath(), code))
    paths.push_back(metadataPath());
  for (const auto &[name, schema] : catalog_.tables()) {
    if (fs::exists(tablePath(name), code))
      paths.push_back(tablePath(name));
  }
  for (const auto &[name, schema] : catalog_.indexes()) {
    if (fs::exists(indexPath(name), code))
      paths.push_back(indexPath(name));
  }
  for (const auto &path : paths) {
    StoredFile file;
    file.path = relative(path.string());
    std::string content;
    readFile(path.string(), content);
    file.bytes = content.size();
    if (content.size() > max_bytes) {
      content.resize(max_bytes);
      file.truncated = true;
    }
    file.content = std::move(content);
    result.push_back(std::move(file));
  }
  return result;
}

} // namespace sql
