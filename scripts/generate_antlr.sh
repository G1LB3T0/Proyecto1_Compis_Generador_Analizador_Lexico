#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANTLR_VERSION="4.13.2"
ANTLR_JAR="${PROJECT_ROOT}/tools/antlr-${ANTLR_VERSION}-complete.jar"
GRAMMAR_DIR="${PROJECT_ROOT}/backend/compiscript/grammar"
OUTPUT="${PROJECT_ROOT}/backend/compiscript/generated"

if [[ ! -f "${ANTLR_JAR}" ]]; then
  echo "Falta ${ANTLR_JAR}"
  echo "Descarga oficial: https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar"
  exit 1
fi

mkdir -p "${OUTPUT}"
(
  cd "${GRAMMAR_DIR}"
  java -jar "${ANTLR_JAR}" -Dlanguage=Cpp -visitor -no-listener \
    -o "${OUTPUT}" -Xexact-output-dir Compiscript.g4
)

echo "Lexer, parser y Visitor C++ generados en ${OUTPUT}"
