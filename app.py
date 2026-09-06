"""Adaptador web del IDE; el compilador se ejecuta íntegramente en C++."""

from pathlib import Path
import json
import os
import subprocess

from flask import Flask, jsonify, render_template, request


app = Flask(
    __name__,
    template_folder="frontend/templates",
    static_folder="frontend/static",
)

BASE_DIR = Path(__file__).resolve().parent
COMPISCRIPT_CLI = BASE_DIR / ("compiscript_cli.exe" if os.name == "nt" else "compiscript_cli")
TEST_BINARY = BASE_DIR / "build" / "compiscript" / ("compiscript_tests.exe" if os.name == "nt" else "compiscript_tests")
EXAMPLES_DIR = BASE_DIR / "backend" / "examples" / "compiscript"
EXAMPLES_DIR.mkdir(parents=True, exist_ok=True)


# Restringe la lectura y escritura a nombres .cps dentro de ejemplos.
def safe_example_path(filename: str) -> Path | None:
    if not filename or Path(filename).name != filename or not filename.endswith(".cps"):
        return None
    candidate = (EXAMPLES_DIR / filename).resolve()
    return candidate if candidate.parent == EXAMPLES_DIR.resolve() else None


@app.get("/")
def index():
    return render_template("index.html")


@app.get("/api/files")
def list_files():
    files = sorted(path.name for path in EXAMPLES_DIR.glob("*.cps") if path.is_file())
    return jsonify({"ok": True, "files": files})


@app.get("/api/file/<path:filename>")
def get_file(filename: str):
    path = safe_example_path(filename)
    if path is None or not path.is_file():
        return jsonify({"ok": False, "error": "Archivo .cps no encontrado"}), 404
    return jsonify({"ok": True, "filename": path.name, "content": path.read_text(encoding="utf-8")})


@app.post("/api/save")
def save_file():
    data = request.get_json(force=True)
    path = safe_example_path(str(data.get("filename", "")))
    if path is None:
        return jsonify({"ok": False, "error": "Usa un nombre simple con extensión .cps"}), 400
    path.write_text(str(data.get("source", "")), encoding="utf-8")
    return jsonify({"ok": True, "filename": path.name})


# Puente hacia el núcleo C++: Flask no implementa reglas del compilador.
@app.post("/api/analyze")
def analyze():
    if not COMPISCRIPT_CLI.is_file():
        return jsonify({
            "ok": False,
            "error": "No existe compiscript_cli. Ejecuta ./scripts/build_compiscript.sh.",
        }), 503
    data = request.get_json(force=True)
    source = str(data.get("source", ""))
    filename = Path(str(data.get("filename", "program.cps"))).name
    if not filename.endswith(".cps"):
        filename += ".cps"
    try:
        process = subprocess.run(
            [str(COMPISCRIPT_CLI), "--stdin", filename],
            input=source,
            capture_output=True,
            text=True,
            timeout=30,
            cwd=BASE_DIR,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "El análisis excedió 30 segundos."}), 504
    if not process.stdout.strip():
        return jsonify({
            "ok": False,
            "error": process.stderr.strip() or "El CLI C++ no produjo salida.",
        }), 500
    try:
        return jsonify(json.loads(process.stdout))
    except json.JSONDecodeError as error:
        return jsonify({
            "ok": False,
            "error": f"El CLI C++ produjo JSON inválido: {error}",
            "detail": process.stdout[:500],
        }), 500


# Ejecuta el binario C++ de pruebas y entrega su reporte al frontend.
@app.post("/api/tests")
def run_tests():
    if not TEST_BINARY.is_file():
        return jsonify({
            "ok": False,
            "error": "La batería no está compilada. Ejecuta ./scripts/build_compiscript.sh.",
        }), 503
    try:
        process = subprocess.run(
            [str(TEST_BINARY), "--json"], capture_output=True, text=True, timeout=30,
            cwd=BASE_DIR, check=False,
        )
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "Las pruebas excedieron 30 segundos."}), 504
    if not process.stdout.strip():
        return jsonify({
            "ok": False,
            "error": process.stderr.strip() or "La batería C++ no produjo un reporte.",
        }), 500
    try:
        report = json.loads(process.stdout)
    except json.JSONDecodeError as error:
        return jsonify({
            "ok": False,
            "error": f"La batería C++ produjo JSON inválido: {error}",
            "detail": process.stdout[:500],
        }), 500
    report["returncode"] = process.returncode
    if process.stderr.strip():
        report["stderr"] = process.stderr.strip()
    return jsonify(report)


if __name__ == "__main__":
    app.run(debug=True, port=5050)
