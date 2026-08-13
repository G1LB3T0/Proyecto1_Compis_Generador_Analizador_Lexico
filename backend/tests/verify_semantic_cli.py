"""Prueba de integración del avance semántico.

Requiere que yapar_cli ya esté compilado en la raíz del repositorio. Ejecuta
el ejemplo con SLR y LALR, verifica invariantes y compara sus modelos. Los IDs
son deterministas, por lo que ambos JSON deben ser exactamente iguales.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EXAMPLES = ROOT / "backend" / "examples"


def executable() -> Path:
    candidates = [ROOT / "yapar_cli.exe", ROOT / "yapar_cli"]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("Compile yapar_cli antes de ejecutar esta prueba")


def run(method: str, source: Path) -> dict:
    completed = subprocess.run(
        [
            str(executable()),
            "--yal",
            str(EXAMPLES / "semantica.yal"),
            "--yalp",
            str(EXAMPLES / "semantica.yalp"),
            "--method",
            method,
            "--input",
            source.read_text(encoding="utf-8"),
        ],
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
    )
    return json.loads(completed.stdout)


def assert_integrity(semantic: dict) -> None:
    scopes = semantic["scopes"]
    symbols = semantic["symbols"]
    types = semantic["types"]
    for scope in scopes:
        assert scope["parent_id"] < scope["id"]
        for symbol_id in scope["symbol_ids"]:
            assert symbols[symbol_id]["scope_id"] == scope["id"]
    for symbol in symbols:
        assert 0 <= symbol["type_id"] < len(types)
        assert 0 <= symbol["scope_id"] < len(scopes)


def main() -> int:
    valid_slr = run("slr", EXAMPLES / "semantic_valid.txt")
    valid_lalr = run("lalr", EXAMPLES / "semantic_valid.txt")
    assert valid_slr["result"]["accepted"]
    assert valid_slr["semantic"]["ok"]
    assert valid_slr["semantic"] == valid_lalr["semantic"]
    assert_integrity(valid_slr["semantic"])

    invalid = run("slr", EXAMPLES / "semantic_errors.txt")
    codes = [item["code"] for item in invalid["semantic"]["diagnostics"]]
    assert invalid["result"]["accepted"]  # sintaxis válida
    assert not invalid["semantic"]["ok"]  # semántica inválida
    assert codes.count("SEM010") == 1
    assert codes.count("SEM002") == 2
    assert codes.count("SEM003") == 2
    print("verify_semantic_cli: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
