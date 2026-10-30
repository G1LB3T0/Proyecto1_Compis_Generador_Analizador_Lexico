// Serializa resultados, catálogo y archivos para el IDE. quoteJson se
// reutiliza sin cambios del Proyecto 2 (compiscript/src/json.cpp).
#include "sql/json.h"

#include <iomanip>
#include <sstream>

namespace sql {

std::string quoteJson(const std::string &value) {
  std::ostringstream output;
  output << '"';
  for (unsigned char character : value) {
    switch (character) {
    case '"':
      output << "\\\"";
      break;
    case '\\':
      output << "\\\\";
      break;
    case '\b':
      output << "\\b";
      break;
    case '\f':
      output << "\\f";
      break;
    case '\n':
      output << "\\n";
      break;
    case '\r':
      output << "\\r";
      break;
    case '\t':
      output << "\\t";
      break;
    default:
      if (character < 0x20) {
        output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
               << static_cast<int>(character) << std::dec;
      } else {
        output << character;
      }
    }
  }
  output << '"';
  return output.str();
}

namespace {

const char *boolean(bool value) { return value ? "true" : "false"; }

std::string stringArray(const std::vector<std::string> &values) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < values.size(); ++index) {
    if (index)
      output << ',';
    output << quoteJson(values[index]);
  }
  output << ']';
  return output.str();
}

// Una celda viaja como texto ya formateado; NULL viaja como null de JSON.
std::string cell(const Value &value) {
  return value.isNull() ? "null" : quoteJson(value.toString());
}

void diagnostics(std::ostringstream &output, const std::vector<Diagnostic> &items,
                 bool &first) {
  for (const auto &item : items) {
    if (!first)
      output << ',';
    first = false;
    output << "{\"code\":" << quoteJson(item.code)
           << ",\"category\":" << quoteJson(item.category)
           << ",\"severity\":" << quoteJson(item.severity)
           << ",\"message\":" << quoteJson(item.message) << ",\"line\":" << item.line
           << ",\"column\":" << item.column << ",\"statement\":" << item.statement
           << '}';
  }
}

} // namespace

std::string databaseToJson(Storage &storage) {
  std::ostringstream output;
  output << "{\"path\":" << quoteJson(storage.root()) << ",\"tables\":[";
  bool first_table = true;
  for (const auto &[name, schema] : storage.catalog().tables()) {
    std::string error;
    const TableData *data = storage.table(name, error);
    if (!first_table)
      output << ',';
    first_table = false;
    output << "{\"name\":" << quoteJson(name)
           << ",\"file\":" << quoteJson(name + ".csv")
           << ",\"rows\":" << (data ? data->rows.size() : 0)
           << ",\"next_rowid\":" << schema.next_rowid
           << ",\"error\":" << quoteJson(error) << ",\"columns\":[";
    for (size_t index = 0; index < schema.columns.size(); ++index) {
      const ColumnSchema &column = schema.columns[index];
      if (index)
        output << ',';
      output << "{\"name\":" << quoteJson(column.name)
             << ",\"type\":" << quoteJson(column.type.toString())
             << ",\"primary_key\":" << boolean(column.primary_key)
             << ",\"not_null\":" << boolean(column.not_null)
             << ",\"unique\":" << boolean(column.unique) << ",\"references\":"
             << quoteJson(column.ref_table.empty()
                              ? ""
                              : column.ref_table + "(" + column.ref_column + ")")
             << '}';
    }
    output << "],\"indexes\":[";
    bool first_index = true;
    for (const IndexSchema *index : storage.catalog().indexesOf(name)) {
      std::string index_error;
      const IndexData *index_data = storage.index(index->name, index_error);
      if (!first_index)
        output << ',';
      first_index = false;
      output << "{\"name\":" << quoteJson(index->name)
             << ",\"column\":" << quoteJson(index->column)
             << ",\"unique\":" << boolean(index->unique)
             << ",\"automatic\":" << boolean(index->automatic)
             << ",\"file\":" << quoteJson("indexes/" + index->name + ".idx")
             << ",\"keys\":" << (index_data ? index_data->entries.size() : 0)
             << ",\"entries\":" << (index_data ? index_data->size() : 0) << '}';
    }
    output << "]}";
  }
  output << "],\"files\":[";
  const auto files = storage.files();
  for (size_t index = 0; index < files.size(); ++index) {
    if (index)
      output << ',';
    output << "{\"path\":" << quoteJson(files[index].path)
           << ",\"bytes\":" << files[index].bytes
           << ",\"truncated\":" << boolean(files[index].truncated)
           << ",\"content\":" << quoteJson(files[index].content) << '}';
  }
  output << "]}";
  return output.str();
}

std::string runToJson(const RunResult &result) {
  std::ostringstream output;
  const size_t diagnostic_count =
      result.lexical_diagnostics.size() + result.syntax_diagnostics.size() +
      result.semantic_diagnostics.size() + result.runtime_diagnostics.size();
  output << "{\"ok\":" << boolean(result.ok) << ",\"mode\":" << quoteJson(result.mode)
         << ",\"filename\":" << quoteJson(result.filename)
         << ",\"phases\":{\"lexical\":" << boolean(result.lexical_ok)
         << ",\"syntax\":" << boolean(result.syntax_ok)
         << ",\"semantic\":" << boolean(result.semantic_ok)
         << ",\"semantic_skipped\":" << boolean(result.semantic_skipped)
         << ",\"execution\":" << boolean(result.execution_ok)
         << ",\"execution_skipped\":" << boolean(result.execution_skipped)
         << ",\"execution_reason\":" << quoteJson(result.execution_reason) << '}'
         << ",\"summary\":{\"tokens\":" << result.tokens.size()
         << ",\"statements\":" << result.statements.size()
         << ",\"diagnostics\":" << diagnostic_count << '}';

  output << ",\"tokens\":[";
  for (size_t index = 0; index < result.tokens.size(); ++index) {
    const TokenInfo &token = result.tokens[index];
    if (index)
      output << ',';
    output << "{\"type\":" << quoteJson(token.type)
           << ",\"category\":" << quoteJson(token.category)
           << ",\"text\":" << quoteJson(token.text) << ",\"line\":" << token.line
           << ",\"column\":" << token.column << '}';
  }
  output << "],\"tree\":" << (result.tree_json.empty() ? "null" : result.tree_json)
         << ",\"ast\":" << (result.ast_json.empty() ? "null" : result.ast_json);

  output << ",\"diagnostics\":[";
  bool first = true;
  diagnostics(output, result.lexical_diagnostics, first);
  diagnostics(output, result.syntax_diagnostics, first);
  diagnostics(output, result.semantic_diagnostics, first);
  diagnostics(output, result.runtime_diagnostics, first);
  output << ']';

  output << ",\"statements\":[";
  for (size_t index = 0; index < result.statements.size(); ++index) {
    const StatementResult &statement = result.statements[index];
    if (index)
      output << ',';
    output << "{\"index\":" << statement.index << ",\"kind\":" << quoteJson(statement.kind)
           << ",\"sql\":" << quoteJson(statement.sql) << ",\"line\":" << statement.line
           << ",\"column\":" << statement.column
           << ",\"status\":" << quoteJson(statement.status)
           << ",\"message\":" << quoteJson(statement.message)
           << ",\"columns\":" << stringArray(statement.columns)
           << ",\"column_types\":" << stringArray(statement.column_types)
           << ",\"affected\":" << statement.affected
           << ",\"plan\":" << stringArray(statement.plan)
           << ",\"files\":" << stringArray(statement.files) << ",\"rows\":[";
    for (size_t row = 0; row < statement.rows.size(); ++row) {
      if (row)
        output << ',';
      output << '[';
      for (size_t column = 0; column < statement.rows[row].size(); ++column) {
        if (column)
          output << ',';
        output << cell(statement.rows[row][column]);
      }
      output << ']';
    }
    output << "]}";
  }
  output << "],\"database\":"
         << (result.database_json.empty() ? "null" : result.database_json) << '}';
  return output.str();
}

} // namespace sql
