"""Adaptador web del IDE; el intérprete SQL se ejecuta íntegramente en C++."""

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
SQL_CLI = BASE_DIR / ("sql_cli.exe" if os.name == "nt" else "sql_cli")
TEST_BINARY = BASE_DIR / "build" / "sql" / ("sql_tests.exe" if os.name == "nt" else "sql_tests")
EXAMPLES_DIR = BASE_DIR / "backend" / "examples" / "sql"
DATABASE_DIR = BASE_DIR / "database"
EXAMPLES_DIR.mkdir(parents=True, exist_ok=True)


# Restringe la lectura y escritura a nombres .sql dentro de ejemplos.
def safe_example_path(filename: str) -> Path | None:
    if not filename or Path(filename).name != filename or not filename.endswith(".sql"):
        return None
    candidate = (EXAMPLES_DIR / filename).resolve()
    return candidate if candidate.parent == EXAMPLES_DIR.resolve() else None


# Ejecuta el CLI C++ y devuelve su JSON; Flask no interpreta SQL.
def run_cli(arguments: list[str], source: str | None = None):
    if not SQL_CLI.is_file():
        return jsonify({
            "ok": False,
            "error": "No existe sql_cli. Ejecuta scripts/build_sql.ps1 o scripts/build_sql.sh.",
        }), 503
    try:
        process = subprocess.run(
            [str(SQL_CLI), "--db", str(DATABASE_DIR), *arguments],
            input=source,
            capture_output=True,
            text=True,
            encoding="utf-8",
            timeout=30,
            cwd=BASE_DIR,
            check=False,
        )
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "La ejecución excedió 30 segundos."}), 504
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


@app.get("/")
def index():
    return render_template("index.html")


@app.get("/api/files")
def list_files():
    files = sorted(path.name for path in EXAMPLES_DIR.glob("*.sql") if path.is_file())
    return jsonify({"ok": True, "files": files})


@app.get("/api/file/<path:filename>")
def get_file(filename: str):
    path = safe_example_path(filename)
    if path is None or not path.is_file():
        return jsonify({"ok": False, "error": "Archivo .sql no encontrado"}), 404
    return jsonify({"ok": True, "filename": path.name, "content": path.read_text(encoding="utf-8")})


@app.post("/api/save")
def save_file():
    data = request.get_json(force=True)
    path = safe_example_path(str(data.get("filename", "")))
    if path is None:
        return jsonify({"ok": False, "error": "Usa un nombre simple con extensión .sql"}), 400
    path.write_text(str(data.get("source", "")), encoding="utf-8")
    return jsonify({"ok": True, "filename": path.name})


# Analiza y, en modo "execute", interpreta el script sobre database/.
@app.post("/api/run")
def run_script():
    data = request.get_json(force=True)
    filename = Path(str(data.get("filename", "script.sql"))).name
    if not filename.endswith(".sql"):
        filename += ".sql"
    arguments = ["--stdin", filename]
    if data.get("mode") == "analyze":
        arguments.insert(0, "--analyze")
    return run_cli(arguments, str(data.get("source", "")))


@app.get("/api/database")
def database():
    return run_cli(["--catalog"])


@app.post("/api/database/reset")
def reset_database():
    return run_cli(["--reset"])


# Ejecuta el binario C++ de pruebas y entrega su reporte al frontend.
@app.post("/api/tests")
def run_tests():
    if not TEST_BINARY.is_file():
        return jsonify({
            "ok": False,
            "error": "La batería no está compilada. Ejecuta scripts/build_sql.ps1 o scripts/build_sql.sh.",
        }), 503
    try:
        process = subprocess.run(
            [str(TEST_BINARY), "--json"], capture_output=True, text=True, encoding="utf-8",
            timeout=60, cwd=BASE_DIR, check=False,
        )
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "Las pruebas excedieron 60 segundos."}), 504
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
