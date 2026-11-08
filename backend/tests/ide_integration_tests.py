#!/usr/bin/env python3
"""Prueba el adaptador web con el compilador C++ y MARS reales.

Requiere instalar requirements.txt y compilar el proyecto antes de ejecutarse.
No inicia un servidor ni escribe archivos fuente en el repositorio.
"""

import io
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from app import app
from mips_runtime import run_mips


class IdeIntegrationTests(unittest.TestCase):
    def setUp(self):
        self.client = app.test_client()

    def test_editor_and_assets(self):
        page = self.client.get("/")
        self.assertEqual(page.status_code, 200)
        self.assertIn(b'id="mips-panel"', page.data)
        self.assertIn(b'id="run-button"', page.data)
        with self.client.get("/static/js/app.js") as asset:
            self.assertEqual(asset.status_code, 200)

    def test_simulator_available(self):
        self.assertTrue(self.client.get("/api/simulator").json["available"])

    def test_compiler_returns_all_phases(self):
        result = self.client.post("/api/analyze", json={"source": "print(2+3*4);", "filename": "test.cps"})
        self.assertEqual(result.status_code, 200)
        for phase in ("syntax_ok", "semantic_ok", "tac_ok", "mips_ok"):
            self.assertTrue(result.json[phase], result.json.get("diagnostics"))
        self.assertIn(".text", result.json["mips"]["text"])

    def test_execute_actual_mips(self):
        result = self.client.post("/api/run", json={"source": "print(2+3*4);", "filename": "test.cps"})
        self.assertTrue(result.json["ok"], result.json)
        self.assertEqual(result.json["execution"]["stdout"], "14\n")
        self.assertEqual(result.json["execution"]["returncode"], 0)

    def test_syntax_error_prevents_execution(self):
        result = self.client.post("/api/run", json={"source": "let x = ;"})
        self.assertEqual(result.status_code, 400)
        self.assertTrue(result.json["analysis"]["mips"]["skipped"])
        self.assertNotIn("execution", result.json)

    def test_semantic_error_prevents_machine_code(self):
        result = self.client.post("/api/build", json={"source": "let x:integer = true;"})
        self.assertEqual(result.status_code, 400)
        self.assertTrue(result.json["analysis"]["mips"]["skipped"])

    def test_runtime_error_is_separate_from_assembly_error(self):
        result = self.client.post("/api/run", json={"source": "let cero=0; print(1/cero);"})
        self.assertTrue(result.json["analysis"]["mips_ok"])
        self.assertFalse(result.json["ok"])
        self.assertEqual(result.json["execution"]["phase"], "execution")
        self.assertNotEqual(result.json["execution"]["returncode"], 0)

    def test_download_assembled_machine_code(self):
        result = self.client.post("/api/build", json={"source": "print(42);", "filename": "test.cps"})
        self.assertEqual(result.status_code, 200, result.json if result.is_json else "")
        self.assertEqual(result.mimetype, "application/zip")
        with zipfile.ZipFile(io.BytesIO(result.data)) as package:
            manifest = json.loads(package.read("manifest.json"))
            self.assertGreater(len(package.read("program.text.bin")), 0)
            self.assertEqual(manifest["architecture"], "MIPS32")
            self.assertEqual(manifest["text_bytes"], len(package.read("program.text.bin")))

    def test_invalid_payload_is_rejected(self):
        for route in ("/api/analyze", "/api/run", "/api/build"):
            self.assertEqual(self.client.post(route, json=[]).status_code, 400)
            self.assertEqual(self.client.post(route, data="invalid", content_type="application/json").status_code, 400)

    def test_missing_simulator_is_reported(self):
        with patch.dict("os.environ", {"MARS_JAR": str(ROOT / "build" / "nonexistent-mars.jar")}):
            result = self.client.post("/api/run", json={"source": "print(1);"})
        self.assertEqual(result.status_code, 503)
        self.assertIn("MARS", result.json["error"])

    def test_infinite_loop_has_instruction_limit(self):
        result = run_mips(".text\n.globl main\nmain: j main\nnop\n", max_steps=32)
        self.assertFalse(result["ok"])
        self.assertTrue(result["step_limit"])
        self.assertFalse(result["timed_out"])

    def test_invalid_assembly_is_reported(self):
        result = run_mips(".text\n.globl main\nmain: invalid_instruction\n")
        self.assertFalse(result["ok"])
        self.assertEqual(result["phase"], "assembly")


if __name__ == "__main__":
    unittest.main(verbosity=2)
