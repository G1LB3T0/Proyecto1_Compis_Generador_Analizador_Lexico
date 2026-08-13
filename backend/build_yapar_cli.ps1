param(
    [string]$Compiler = "g++"
)

$ErrorActionPreference = "Stop"
$Backend = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $Backend
$Src = Join-Path $Backend "src"
$Output = Join-Path $Root "yapar_cli.exe"

if (-not (Get-Command $Compiler -ErrorAction SilentlyContinue)) {
    throw "No se encontró '$Compiler'. Instale un compilador C++17 o indique -Compiler."
}

# Lista explícita: evita compilar main.cpp y yapar_main.cpp, que contienen
# otros puntos de entrada. Cualquier módulo nuevo del CLI debe agregarse aquí.
$Sources = @(
    "yalex_parser.cpp",
    "regex_node.cpp",
    "regex_parser.cpp",
    "dfa_builder.cpp",
    "yapar_parser.cpp",
    "first_follow.cpp",
    "lr0_automaton.cpp",
    "slr_table.cpp",
    "lalr_table.cpp",
    "ll1_table.cpp",
    "grammar_transform.cpp",
    "lr_parser.cpp",
    "ll1_parser.cpp",
    "parse_tree.cpp",
    "ast.cpp",
    "semantic_model.cpp",
    "semantic_analyzer.cpp",
    "semantic_json.cpp",
    "yapar_cli.cpp"
) | ForEach-Object { Join-Path $Src $_ }

& $Compiler -std=c++17 -O2 -Wall -Wextra -pedantic @Sources -o $Output
if ($LASTEXITCODE -ne 0) {
    throw "La compilación de yapar_cli falló con código $LASTEXITCODE."
}

Write-Host "Compilado: $Output"
