#!/usr/bin/env bash
# Regenera los archivos C++ de ANTLR desde una gramática.
#   ./scripts/generate_antlr.sh              -> backend/sql/grammar/SQL.g4 (Proyecto 3)
#   ./scripts/generate_antlr.sh compiscript  -> Compiscript.g4 (Proyecto 2)
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANTLR_VERSION="4.13.2"
ANTLR_JAR="${PROJECT_ROOT}/tools/antlr-${ANTLR_VERSION}-complete.jar"

case "${1:-sql}" in
  sql)
    MODULE="sql"
    GRAMMAR="SQL.g4"
    ;;
  compiscript)
    MODULE="compiscript"
    GRAMMAR="Compiscript.g4"
    ;;
  *)
    echo "Uso: $0 [sql|compiscript]"
    exit 2
    ;;
esac

GRAMMAR_DIR="${PROJECT_ROOT}/backend/${MODULE}/grammar"
OUTPUT="${PROJECT_ROOT}/backend/${MODULE}/generated"

if [[ ! -f "${ANTLR_JAR}" ]]; then
  echo "Falta ${ANTLR_JAR}"
  echo "Descarga oficial: https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar"
  exit 1
fi

mkdir -p "${OUTPUT}"
(
  cd "${GRAMMAR_DIR}"
  java -jar "${ANTLR_JAR}" -Dlanguage=Cpp -visitor -no-listener \
    -o "${OUTPUT}" -Xexact-output-dir "${GRAMMAR}"
)

echo "Lexer, parser y Visitor C++ generados en ${OUTPUT}"
