// Ejecutable de consola: lee un script .sql, lo interpreta y escribe el JSON.
// Adaptado del CLI del Proyecto 2: agrega --db, --analyze, --catalog y --reset.
#include "sql/json.h"
#include "sql/service.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string readAll(std::istream &input) {
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void usage(const char *program) {
  std::cerr << "Uso:\n"
            << "  " << program << " [--db carpeta] [--analyze] --file script.sql\n"
            << "  " << program << " [--db carpeta] [--analyze] --stdin [nombre.sql]\n"
            << "  " << program << " [--db carpeta] --catalog\n"
            << "  " << program << " [--db carpeta] --reset\n";
}

} // namespace

int main(int argc, char **argv) {
  sql::RunOptions options;
  std::string command;
  std::string source;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--db" && index + 1 < argc) {
      options.database = argv[++index];
    } else if (argument == "--analyze") {
      options.execute = false;
    } else if (argument == "--catalog" || argument == "--reset") {
      command = argument;
    } else if (argument == "--file" && index + 1 < argc) {
      command = argument;
      options.filename = argv[++index];
    } else if (argument == "--stdin") {
      command = argument;
      if (index + 1 < argc && std::string(argv[index + 1]).rfind("--", 0) != 0)
        options.filename = argv[++index];
    } else {
      usage(argv[0]);
      return 2;
    }
  }

  if (command == "--catalog" || command == "--reset") {
    std::string error;
    if (command == "--reset" && !sql::resetDatabase(options.database, error)) {
      std::cerr << error << '\n';
      return 2;
    }
    const std::string json = sql::describeDatabase(options.database, error);
    if (json.empty()) {
      std::cerr << error << '\n';
      return 2;
    }
    std::cout << json << '\n';
    return 0;
  }

  if (command == "--file") {
    const std::string &filename = options.filename;
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".sql") {
      std::cerr << "El archivo de entrada debe usar la extensión .sql.\n";
      return 2;
    }
    std::ifstream file(filename);
    if (!file) {
      std::cerr << "No se pudo abrir " << filename << ".\n";
      return 2;
    }
    source = readAll(file);
  } else if (command == "--stdin") {
    source = readAll(std::cin);
  } else {
    usage(argv[0]);
    return 2;
  }

  const sql::RunResult result = sql::runScript(source, options);
  std::cout << sql::runToJson(result) << '\n';
  return result.ok ? 0 : 1;
}
