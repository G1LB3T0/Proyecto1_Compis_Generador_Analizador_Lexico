// Lee y escribe tablas (.csv), índices (.idx) y el catálogo (metadata.txt).
#include "sql/storage.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace sql {

// El bloqueo del sistema operativo se libera incluso si el CLI termina de
// forma inesperada. Otro proceso nunca recupera un diario que sigue activo.
struct StorageLock {
#ifdef _WIN32
  HANDLE handle = INVALID_HANDLE_VALUE;
  explicit StorageLock(const fs::path &path) {
    handle = CreateFileW(path.wstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                         nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
      throw std::runtime_error("la base está ocupada o no se pudo bloquear");
  }
  ~StorageLock() { CloseHandle(handle); }
#else
  int descriptor = -1;
  explicit StorageLock(const fs::path &path) {
    descriptor = ::open(path.c_str(), O_CREAT | O_RDWR, 0600);
    if (descriptor < 0)
      throw std::runtime_error("no se pudo abrir el bloqueo de la base");
    if (flock(descriptor, LOCK_EX | LOCK_NB) != 0) {
      ::close(descriptor);
      throw std::runtime_error("la base está ocupada por otra ejecución");
    }
  }
  ~StorageLock() { ::close(descriptor); }
#endif
};

namespace {

struct CsvField {
  std::string text;
  bool quoted = false;
};

// Separa un archivo CSV completo en registros; respeta comillas, comillas
// dobles escapadas ("") y saltos de línea dentro de un texto.
std::vector<std::vector<CsvField>> parseCsv(
    const std::string &content,
    std::vector<std::pair<size_t, size_t>> *positions = nullptr) {
  std::vector<std::vector<CsvField>> records;
  std::vector<CsvField> record;
  CsvField field;
  bool in_quotes = false;
  bool field_started = false;
  size_t record_start = 0;
  auto end_field = [&]() {
    record.push_back(field);
    field = CsvField{};
    field_started = false;
  };
  auto end_record = [&](size_t end) {
    if (field_started || !record.empty())
      end_field();
    if (!record.empty()) {
      records.push_back(record);
      if (positions)
        positions->push_back({record_start, end - record_start});
    }
    record.clear();
    record_start = end;
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
      end_record(index + 1);
    } else if (character != '\r') {
      field.text += character;
      field_started = true;
    }
  }
  if (in_quotes)
    throw std::runtime_error("campo CSV con comillas sin cerrar");
  end_record(content.size());
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
    if (const auto value = parseFloat(field.text))
      return Value::makeFloat(*value);
    throw std::runtime_error("FLOAT inválido");
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

Storage::Storage(std::string root) : root_(std::move(root)), transaction_(root_) {}
Storage::~Storage() = default;

std::string Storage::metadataPath() const {
  return (fs::path(root_) / "metadata.txt").string();
}

std::string Storage::tablePath(const std::string &table) const {
  return (fs::path(root_) / (table + ".csv")).string();
}

std::string Storage::indexPath(const std::string &index) const {
  return (fs::path(root_) / "indexes" / (index + ".idx")).string();
}

std::string Storage::positionsPath(const std::string &table) const {
  return (fs::path(root_) / (table + ".rows")).string();
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
  if (!lock_) {
    try {
      lock_ = std::make_unique<StorageLock>(fs::path(root_) / ".lock");
    } catch (const std::exception &failure) {
      error = failure.what();
      return false;
    }
  }
  tables_.clear();
  indexes_.clear();
  positions_.clear();
  reads_ = {};
  transaction_.clear();
  if (!FileTransaction::recover(root_, error))
    return false;
  return catalog_.load(metadataPath(), error);
}

void Storage::beginTransaction() { transaction_.clear(); }
bool Storage::commitTransaction(std::string &error) { return transaction_.commit(error); }

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
  if (!readFile(tablePath(name), content)) {
    error = "no se pudo leer " + name + ".csv";
    return nullptr;
  }
  RowDirectory directory;
  try {
    std::vector<std::pair<size_t, size_t>> offsets;
    const auto records = parseCsv(content, &offsets);
    reads_.bytes += content.size();
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
        const long long rowid = std::stoll(record[0].text);
        if (!data.rows.emplace(rowid, std::move(row)).second)
          throw std::runtime_error("rowid duplicado");
        directory[rowid] = {offsets[index].first, offsets[index].second};
        ++reads_.rows;
      } catch (...) {
        error = name + ".csv registro " + std::to_string(index) +
                ": valor ilegible";
        return nullptr;
      }
    }
  } catch (const std::exception &failure) {
    error = name + ".csv: " + failure.what();
    return nullptr;
  }
  positions_[name] = std::move(directory);
  return &(tables_[name] = std::move(data));
}

const Storage::RowDirectory *Storage::positions(const std::string &name,
                                               std::string &error) {
  auto cached = positions_.find(name);
  if (cached != positions_.end())
    return &cached->second;
  if (!catalog_.table(name)) {
    error = "la tabla " + name + " no existe";
    return nullptr;
  }
  std::ifstream file(positionsPath(name));
  if (!file) {
    // Compatibilidad: las bases antiguas se leen una vez; el siguiente DML
    // persiste el directorio junto con la tabla.
    if (!table(name, error))
      return nullptr;
    return &positions_.at(name);
  }
  std::string marker;
  unsigned version = 0;
  size_t bytes = 0;
  std::error_code code;
  const auto actual_bytes = fs::file_size(tablePath(name), code);
  if (!(file >> marker >> version >> bytes) || marker != "ROWS" || version != 1 ||
      code || bytes != actual_bytes) {
    error = name + ".rows no corresponde al CSV actual";
    return nullptr;
  }
  RowDirectory directory;
  long long rowid;
  RowPosition position;
  while (file >> rowid) {
    if (!(file >> position.offset >> position.bytes) || rowid <= 0 ||
        position.offset > bytes || position.bytes == 0 ||
        position.bytes > bytes - position.offset ||
        !directory.emplace(rowid, position).second) {
      error = name + ".rows contiene una posición inválida";
      return nullptr;
    }
  }
  if (!file.eof()) {
    error = name + ".rows contiene un rowid inválido";
    return nullptr;
  }
  return &(positions_[name] = std::move(directory));
}

size_t Storage::rowCount(const std::string &name, std::string &error) {
  const auto cached = tables_.find(name);
  if (cached != tables_.end())
    return cached->second.rows.size();
  const auto *directory = positions(name, error);
  return directory ? directory->size() : 0;
}

bool Storage::readRows(const std::string &name, const std::set<long long> &rowids,
                       TableData &result, std::string &error) {
  result.rows.clear();
  auto cached = tables_.find(name);
  if (cached != tables_.end()) {
    for (long long rowid : rowids) {
      auto found = cached->second.rows.find(rowid);
      if (found == cached->second.rows.end()) {
        error = "el índice apunta a un rowid inexistente en " + name;
        return false;
      }
      result.rows.insert(*found);
    }
    return true;
  }
  const auto *directory = positions(name, error);
  if (!directory)
    return false;
  if (tables_.count(name)) // base antigua recién cargada
    return readRows(name, rowids, result, error);
  const TableSchema &schema = *catalog_.table(name);
  std::ifstream file(tablePath(name), std::ios::binary);
  if (!file) {
    error = "no se pudo abrir " + name + ".csv";
    return false;
  }
  try {
    for (long long rowid : rowids) {
      auto position = directory->find(rowid);
      if (position == directory->end())
        throw std::runtime_error("el índice apunta a un rowid inexistente");
      std::string record(position->second.bytes, '\0');
      file.seekg(static_cast<std::streamoff>(position->second.offset));
      if (!file.read(record.data(), static_cast<std::streamsize>(record.size())))
        throw std::runtime_error("no se pudo leer la posición de la fila");
      reads_.bytes += record.size();
      const auto fields = parseCsv(record);
      if (fields.size() != 1 || fields[0].size() != schema.columns.size() + 1 ||
          std::stoll(fields[0][0].text) != rowid)
        throw std::runtime_error("la posición no corresponde a la fila solicitada");
      Row row;
      for (size_t column = 0; column < schema.columns.size(); ++column)
        row.push_back(decodeField(fields[0][column + 1], schema.columns[column].type));
      result.rows[rowid] = std::move(row);
      ++reads_.rows;
    }
    return true;
  } catch (const std::exception &failure) {
    error = name + ".csv: " + failure.what();
    return false;
  }
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
  // Solo el encabezado tiene comentarios. Una línea dentro de un campo
  // entrecomillado puede empezar con # o estar vacía y es parte de la clave.
  size_t start = 0;
  while (start < content.size() && content[start] == '#') {
    const size_t end = content.find('\n', start);
    start = end == std::string::npos ? content.size() : end + 1;
  }
  try {
    for (const auto &record : parseCsv(content.substr(start))) {
      if (record.size() != 2)
        throw std::runtime_error("entrada de índice inválida");
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
  positions_.erase(name);
}

void Storage::putIndex(const std::string &name, IndexData data) {
  indexes_[name] = std::move(data);
}

bool Storage::saveCatalog(std::string &) {
  transaction_.write("metadata.txt", catalog_.serialize());
  return true;
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
  RowDirectory directory;
  std::ostringstream offsets;
  for (const auto &[rowid, row] : data->rows) {
    const size_t begin = static_cast<size_t>(output.tellp());
    output << rowid;
    for (const auto &value : row)
      output << ',' << encodeCsvValue(value);
    output << '\n';
    const size_t bytes = static_cast<size_t>(output.tellp()) - begin;
    offsets << rowid << ' ' << begin << ' ' << bytes << '\n';
    directory[rowid] = {begin, bytes};
  }
  const std::string csv = output.str();
  transaction_.write(name + ".csv", csv);
  transaction_.write(name + ".rows", "ROWS 1 " + std::to_string(csv.size()) + "\n" + offsets.str());
  positions_[name] = std::move(directory);
  return true;
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
  transaction_.write("indexes/" + name + ".idx", output.str());
  return true;
}

bool Storage::removeTable(const std::string &name, std::string &) {
  tables_.erase(name);
  positions_.erase(name);
  transaction_.remove(name + ".csv");
  transaction_.remove(name + ".rows");
  return true;
}

bool Storage::removeIndex(const std::string &name, std::string &) {
  indexes_.erase(name);
  transaction_.remove("indexes/" + name + ".idx");
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
        (extension == ".csv" || extension == ".rows" || entry.path().filename() == "metadata.txt"))
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
    if (fs::exists(positionsPath(name), code))
      paths.push_back(positionsPath(name));
  }
  for (const auto &[name, schema] : catalog_.indexes()) {
    if (fs::exists(indexPath(name), code))
      paths.push_back(indexPath(name));
  }
  for (const auto &path : paths) {
    StoredFile file;
    file.path = relative(path.string());
    file.bytes = static_cast<size_t>(fs::file_size(path, code));
    if (code)
      continue;
    std::string content(std::min(file.bytes, max_bytes), '\0');
    std::ifstream input(path, std::ios::binary);
    input.read(content.data(), static_cast<std::streamsize>(content.size()));
    content.resize(static_cast<size_t>(input.gcount()));
    file.truncated = file.bytes > content.size();
    file.content = std::move(content);
    result.push_back(std::move(file));
  }
  return result;
}

} // namespace sql
