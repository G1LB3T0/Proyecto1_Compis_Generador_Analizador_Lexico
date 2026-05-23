import subprocess
import json
import os
from flask import Flask, request, jsonify, render_template

app = Flask(__name__,
            template_folder='frontend/templates',
            static_folder='frontend/static')

BASE_DIR    = os.path.dirname(os.path.abspath(__file__))
YAPAR_CLI   = os.path.join(BASE_DIR, "yapar_cli")
EXAMPLES    = os.path.join(BASE_DIR, "backend", "examples")


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/files")
def list_files():
    try:
        yal  = sorted(f for f in os.listdir(EXAMPLES) if f.endswith(".yal"))
        yalp = sorted(f for f in os.listdir(EXAMPLES) if f.endswith(".yalp"))
        return jsonify({"yal": yal, "yalp": yalp})
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/api/file/<path:filename>")
def get_file(filename):
    filepath = os.path.join(EXAMPLES, filename)
    if not os.path.isfile(filepath):
        return jsonify({"ok": False, "error": "Archivo no encontrado"}), 404
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()
        return jsonify({"ok": True, "content": content})
    except Exception as e:
        return jsonify({"ok": False, "error": str(e)}), 500


@app.route("/api/save", methods=["POST"])
def save_file():
    data     = request.get_json(force=True)
    filename = data.get("filename", "")
    content  = data.get("content", "")
    if not filename:
        return jsonify({"ok": False, "error": "Falta nombre de archivo"}), 400
    filepath = os.path.join(EXAMPLES, filename)
    try:
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(content)
        return jsonify({"ok": True})
    except Exception as e:
        return jsonify({"ok": False, "error": str(e)}), 500


@app.route("/api/run", methods=["POST"])
def run():
    data    = request.get_json(force=True)
    yal     = data.get("yal", "")
    yalp    = data.get("yalp", "")
    method  = data.get("method", "slr")
    inp     = data.get("input", None)

    if not yal or not yalp:
        return jsonify({"ok": False, "error": "Faltan archivos yal/yalp"}), 400

    yal_path  = os.path.join(EXAMPLES, yal)
    yalp_path = os.path.join(EXAMPLES, yalp)

    cmd = [YAPAR_CLI, "--yal", yal_path, "--yalp", yalp_path, "--method", method]
    if inp is not None:
        cmd += ["--input", inp]

    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=30,
                              cwd=BASE_DIR)
        stdout = proc.stdout.strip()
        if not stdout:
            err = proc.stderr.strip() or "Sin salida del CLI"
            return jsonify({"ok": False, "error": err}), 500
        return jsonify(json.loads(stdout))
    except subprocess.TimeoutExpired:
        return jsonify({"ok": False, "error": "Timeout (>30 s)"}), 500
    except json.JSONDecodeError as e:
        return jsonify({"ok": False,
                        "error": f"JSON inválido: {e}\nOutput: {stdout[:300]}"}), 500
    except Exception as e:
        return jsonify({"ok": False, "error": str(e)}), 500


if __name__ == "__main__":
    app.run(debug=True, port=5050)
