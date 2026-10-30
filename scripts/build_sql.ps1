# Variante de Windows: genera ANTLR, compila el intérprete SQL y lo prueba.
$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build/sql"
$AntlrVersion = "4.13.2"
$AntlrJar = Join-Path $ProjectRoot "tools/antlr-$AntlrVersion-complete.jar"

if (-not (Test-Path $AntlrJar)) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $AntlrJar) | Out-Null
    Invoke-WebRequest "https://www.antlr.org/download/antlr-$AntlrVersion-complete.jar" -OutFile $AntlrJar
}

$GeneratedDir = Join-Path $ProjectRoot "backend/sql/generated"
$GrammarDir = Join-Path $ProjectRoot "backend/sql/grammar"
New-Item -ItemType Directory -Force $GeneratedDir | Out-Null
Push-Location $GrammarDir
try {
    & java -jar $AntlrJar -Dlanguage=Cpp -visitor -no-listener `
        -o $GeneratedDir -Xexact-output-dir SQL.g4
} finally {
    Pop-Location
}

# ANTLR4_LOCAL_SOURCE permite usar un runtime C++ ya descargado (sin internet).
$ExtraArgs = @()
if ($env:ANTLR4_LOCAL_SOURCE) {
    $ExtraArgs += "-DANTLR4_LOCAL_SOURCE=$env:ANTLR4_LOCAL_SOURCE"
}

cmake -S $ProjectRoot -B $BuildDir -DCMAKE_BUILD_TYPE=Release @ExtraArgs
cmake --build $BuildDir --config Release --parallel
ctest --test-dir $BuildDir -C Release --output-on-failure
