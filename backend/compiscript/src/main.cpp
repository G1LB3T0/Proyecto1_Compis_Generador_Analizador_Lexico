// CLI: compila .cps y entrega JSON de todas las fases o ensamblador MIPS.
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
            << " --stdin [nombre.cps]\n  " << program
            << " --file programa.cps --emit-mips [-o programa.asm]\n  " << program
            << " --emit-mips programa.cps [-o programa.asm]\n";
}

} // namespace

int main(int argc, char **argv) {
  std::string source;
  std::string filename = "program.cps";
  std::string output_path;
  bool from_stdin = false;
  bool input_selected = false;
  bool emit_mips = false;
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--help" || argument == "-h") {
      usage(argv[0]);
      return 0;
    }
    if (argument == "--file" && index + 1 < argc && !input_selected) {
      filename = argv[++index];
      input_selected = true;
    } else if (argument == "--stdin" && !input_selected) {
      from_stdin = input_selected = true;
      if (index + 1 < argc && argv[index + 1][0] != '-')
        filename = argv[++index];
    } else if (argument == "--emit-mips") {
      emit_mips = true;
    } else if ((argument == "-o" || argument == "--output") && index + 1 < argc) {
      output_path = argv[++index];
      emit_mips = true;
    } else if (!input_selected && !argument.empty() && argument[0] != '-') {
      filename = argument;
      input_selected = true;
    } else {
      usage(argv[0]);
      return 2;
    }
  }
  if (!input_selected || filename.size() < 4 || filename.substr(filename.size() - 4) != ".cps") {
    usage(argv[0]);
    return 2;
  }
  if (!from_stdin) {
    std::ifstream file(filename);
    if (!file) {
      std::cerr << "No se pudo abrir " << filename << ".\n";
      return 2;
    }
    source = readAll(file);
  } else {
    source = readAll(std::cin);
  }

  const compiscript::AnalysisResult result =
      compiscript::analyzeSource(source, filename);
  if (emit_mips) {
    if (!result.mips_ok) {
      std::cerr << compiscript::analysisToJson(result) << '\n';
      return 1;
    }
    if (output_path.empty()) {
      std::cout << result.mips.text;
    } else {
      std::ofstream output(output_path, std::ios::binary);
      output << result.mips.text;
      if (!output) {
        std::cerr << "No se pudo escribir " << output_path << ".\n";
        return 2;
      }
    }
  } else {
    std::cout << compiscript::analysisToJson(result) << '\n';
  }
  return result.ok ? 0 : 1;
}
