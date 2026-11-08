"""Adaptador web del IDE; el compilador se ejecuta íntegramente en C++."""

from pathlib import Path
import io
import json
import os
import subprocess
import sys

from flask import Flask, jsonify, render_template, request, send_file

from mips_runtime import MipsRuntimeError, machine_code_package, run_mips, simulator_status


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
def compile_source(data: dict) -> tuple[dict, int]:
    if not COMPISCRIPT_CLI.is_file():
        return {
            "ok": False,
            "error": "No existe compiscript_cli. Ejecuta ./scripts/build_compiscript.sh.",
        }, 503
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
        return {"ok": False, "error": "La compilación excedió 30 segundos."}, 504
    except OSError as error:
        return {"ok": False, "error": f"No se pudo iniciar el compilador: {error}"}, 503
    if not process.stdout.strip():
        return {
            "ok": False,
            "error": process.stderr.strip() or "El CLI C++ no produjo salida.",
        }, 500
    try:
        return json.loads(process.stdout), 200
    except json.JSONDecodeError as error:
        return {
            "ok": False,
            "error": f"El CLI C++ produjo JSON inválido: {error}",
            "detail": process.stdout[:500],
        }, 500


def source_payload() -> dict | None:
    data = request.get_json(silent=True)
    return data if isinstance(data, dict) else None


@app.post("/api/analyze")
def analyze():
    data = source_payload()
    if data is None:
        return jsonify({"ok": False, "error": "Envía un objeto JSON con source y filename."}), 400
    result, status = compile_source(data)
    return jsonify(result), status


@app.get("/api/simulator")
def simulator():
    status = simulator_status()
    return jsonify({key: status[key] for key in ("available", "simulator", "target", "error")})


@app.post("/api/run")
def run_program():
    data = source_payload()
    if data is None:
        return jsonify({"ok": False, "error": "Envía código Compiscript como JSON."}), 400
    analysis, status = compile_source(data)
    if status != 200:
        return jsonify(analysis), status
    if not analysis.get("mips_ok"):
        return jsonify({"ok": False, "analysis": analysis,
                        "error": "Corrige los errores de compilación antes de ejecutar."}), 400
    try:
        execution = run_mips(analysis["mips"]["text"])
    except MipsRuntimeError as error:
        return jsonify({"ok": False, "analysis": analysis, "error": str(error)}), 503
    return jsonify({"ok": execution["ok"], "analysis": analysis, "execution": execution})


@app.post("/api/build")
def build_program():
    data = source_payload()
    if data is None:
        return jsonify({"ok": False, "error": "Envía código Compiscript como JSON."}), 400
    analysis, status = compile_source(data)
    if status != 200:
        return jsonify(analysis), status
    if not analysis.get("mips_ok"):
        return jsonify({"ok": False, "analysis": analysis,
                        "error": "Corrige los errores de compilación antes de ensamblar."}), 400
    try:
        package = machine_code_package(analysis["mips"]["text"], analysis["filename"])
    except MipsRuntimeError as error:
        return jsonify({"ok": False, "error": str(error)}), 503
    return send_file(io.BytesIO(package), mimetype="application/zip", as_attachment=True,
                     download_name=f"{Path(analysis['filename']).stem}.mips.zip")


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
    status = simulator_status()
    if not status["available"]:
        report["ok"] = False
        report["error"] = status["error"]
        report["mips_tests_skipped"] = True
        return jsonify(report), 503
    try:
        mips_process = subprocess.run(
            [sys.executable, str(BASE_DIR / "backend" / "tests" / "mips_execution_tests.py"),
             "--cli", str(COMPISCRIPT_CLI), "--mars", status["mars"], "--java", status["java"], "--json"],
            capture_output=True, text=True, timeout=180, cwd=BASE_DIR, check=False,
        )
        mips_report = json.loads(mips_process.stdout)
    except (OSError, subprocess.TimeoutExpired, json.JSONDecodeError) as error:
        report["ok"] = False
        report["error"] = f"La batería MIPS no pudo completarse: {error}"
        return jsonify(report), 500
    report["ok"] = report["ok"] and mips_report["ok"] and mips_process.returncode == 0
    report["categories"].extend(mips_report.get("categories", []))
    for key in ("total", "passed", "failed", "valid_cases", "expected_error_cases"):
        report["summary"][key] = report["summary"].get(key, 0) + mips_report.get("summary", {}).get(key, 0)
    report["mips_summary"] = mips_report.get("summary", {})
    return jsonify(report)


if __name__ == "__main__":
    app.run(debug=True, port=5050)
