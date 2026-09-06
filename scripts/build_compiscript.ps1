$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build/compiscript"
$AntlrVersion = "4.13.2"
$AntlrJar = Join-Path $ProjectRoot "tools/antlr-$AntlrVersion-complete.jar"

if (-not (Test-Path $AntlrJar)) {
    New-Item -ItemType Directory -Force (Split-Path -Parent $AntlrJar) | Out-Null
    Invoke-WebRequest "https://www.antlr.org/download/antlr-$AntlrVersion-complete.jar" -OutFile $AntlrJar
}

$GeneratedDir = Join-Path $ProjectRoot "backend/compiscript/generated"
$GrammarDir = Join-Path $ProjectRoot "backend/compiscript/grammar"
New-Item -ItemType Directory -Force $GeneratedDir | Out-Null
Push-Location $GrammarDir
try {
    & java -jar $AntlrJar -Dlanguage=Cpp -visitor -no-listener `
        -o $GeneratedDir -Xexact-output-dir Compiscript.g4
} finally {
    Pop-Location
}

cmake -S $ProjectRoot -B $BuildDir -DCMAKE_BUILD_TYPE=Release
cmake --build $BuildDir --config Release --parallel
ctest --test-dir $BuildDir -C Release --output-on-failure
