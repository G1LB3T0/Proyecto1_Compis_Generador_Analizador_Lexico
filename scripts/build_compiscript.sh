#!/usr/bin/env bash
# Descarga dependencias faltantes, regenera ANTLR, compila y ejecuta las pruebas.
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/compiscript"
ANTLR_VERSION="4.13.2"
ANTLR_JAR="${PROJECT_ROOT}/tools/antlr-${ANTLR_VERSION}-complete.jar"

if [[ ! -f "${ANTLR_JAR}" ]]; then
  mkdir -p "${PROJECT_ROOT}/tools"
  curl --fail --location \
    --output "${ANTLR_JAR}" \
    "https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar"
fi

"${PROJECT_ROOT}/scripts/generate_antlr.sh"

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "Listo: ${PROJECT_ROOT}/compiscript_cli"
