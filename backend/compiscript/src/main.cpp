// Ejecutable de consola: lee un .cps, solicita su análisis y escribe el JSON.
#include "compiscript/json.h"
#include "compiscript/service.h"

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
  std::cerr << "Uso:\n  " << program << " --file programa.cps\n  " << program
            << " --stdin [nombre.cps]\n";
}

} // namespace

int main(int argc, char **argv) {
  std::string source;
  std::string filename = "program.cps";
  if (argc >= 3 && std::string(argv[1]) == "--file") {
    filename = argv[2];
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".cps") {
      std::cerr << "El archivo de entrada debe usar la extensión .cps.\n";
      return 2;
    }
    std::ifstream file(filename);
    if (!file) {
      std::cerr << "No se pudo abrir " << filename << ".\n";
      return 2;
    }
    source = readAll(file);
  } else if (argc >= 2 && std::string(argv[1]) == "--stdin") {
    if (argc >= 3)
      filename = argv[2];
    source = readAll(std::cin);
  } else {
    usage(argv[0]);
    return 2;
  }

  const compiscript::AnalysisResult result =
      compiscript::analyzeSource(source, filename);
  std::cout << compiscript::analysisToJson(result) << '\n';
  return result.ok ? 0 : 1;
}
