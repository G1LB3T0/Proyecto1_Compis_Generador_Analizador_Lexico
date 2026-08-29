#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$SCRIPT_DIR/src"
COMPILER="${CXX:-c++}"

sources=(
  yalex_parser.cpp
  regex_node.cpp
  regex_parser.cpp
  dfa_builder.cpp
  yapar_parser.cpp
  first_follow.cpp
  lr0_automaton.cpp
  slr_table.cpp
  lalr_table.cpp
  ll1_table.cpp
  grammar_transform.cpp
  lr_parser.cpp
  ll1_parser.cpp
  parse_tree.cpp
  ast.cpp
  semantic_model.cpp
  semantic_analyzer.cpp
  semantic_json.cpp
  yapar_cli.cpp
)

args=()
for source in "${sources[@]}"; do
  args+=("$SRC_DIR/$source")
done

"$COMPILER" -std=c++17 -O2 -Wall -Wextra -pedantic \
  "${args[@]}" -o "$ROOT_DIR/yapar_cli"

echo "Compilado: $ROOT_DIR/yapar_cli"
