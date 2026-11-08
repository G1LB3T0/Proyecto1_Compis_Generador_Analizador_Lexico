"""Ejecución y ensamblado MIPS reales con MARS; no interpreta Compiscript."""

from __future__ import annotations

import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parent
DEFAULT_MAX_STEPS = 1_000_000


class MipsRuntimeError(RuntimeError):
    """Dependencia o resultado del simulador que impide ejecutar el programa."""


def simulator_status() -> dict:
    java = shutil.which(os.environ.get("JAVA", "java"))
    mars = Path(os.environ.get("MARS_JAR", str(ROOT / "tools" / "Mars.jar"))).expanduser().resolve()
    error = ""
    if not java:
        error = "Java no está disponible. Instala un JRE/JDK para ejecutar MARS."
    elif not mars.is_file():
        error = "Falta MARS. Ejecuta python3 scripts/setup_mars.py."
    return {"available": not error, "simulator": "MARS 4.5", "target": "MIPS32",
            "java": java, "mars": str(mars), "error": error}


def _command(status: dict) -> list[str]:
    if not status["available"]:
        raise MipsRuntimeError(status["error"])
    return [status["java"], "-Djava.awt.headless=true", "-jar", status["mars"],
            "nc", "sm", "me", "ae65", "se70"]


def run_mips(assembly: str, *, stdin: str = "", timeout: float = 15,
             max_steps: int = DEFAULT_MAX_STEPS, delayed_branches: bool = False) -> dict:
    """Ensambla y ejecuta; distingue fallo de ensamblado, ejecución y límites."""
    status = simulator_status()
    command = _command(status)
    if not assembly.strip():
        raise MipsRuntimeError("No hay código ensamblador para ejecutar.")
    # MARS 4.5 interpreta los números 0..31 como nombres de registros.
    if not 32 <= max_steps <= 10_000_000:
        raise MipsRuntimeError("El límite de instrucciones debe estar entre 32 y 10000000.")
    if delayed_branches:
        command.append("db")
    with tempfile.TemporaryDirectory(prefix="compiscript-mips-") as temporary:
        source = Path(temporary) / "program.asm"
        source.write_text(assembly, encoding="utf-8")
        try:
            process = subprocess.run(command + [str(max_steps), str(source)],
                                     input=stdin, text=True, capture_output=True,
                                     timeout=timeout, cwd=temporary, check=False)
        except subprocess.TimeoutExpired as error:
            def decoded(value):
                return value.decode("utf-8", errors="replace") if isinstance(value, bytes) else (value or "")
            return {"ok": False, "simulator": status["simulator"], "phase": "execution",
                    "stdout": decoded(error.stdout), "stderr": decoded(error.stderr),
                    "returncode": None, "timed_out": True, "step_limit": False,
                    "error": f"La ejecución excedió {timeout:g} segundos."}
        except OSError as error:
            raise MipsRuntimeError(f"No se pudo iniciar Java/MARS: {error}") from error
    messages = process.stderr + process.stdout
    step_limit = "maximum step limit" in messages.lower()
    ok = process.returncode == 0 and not step_limit
    error = ""
    if step_limit:
        error = f"La ejecución alcanzó el límite de {max_steps} instrucciones."
    elif process.returncode:
        error = ("MARS rechazó el ensamblador." if process.returncode == 65
                 else "El programa terminó con un error de ejecución.")
    return {"ok": ok, "simulator": status["simulator"],
            "phase": "assembly" if process.returncode == 65 else "execution",
            "stdout": process.stdout, "stderr": process.stderr,
            "returncode": process.returncode, "timed_out": False,
            "step_limit": step_limit, "error": error}


def machine_code_package(assembly: str, filename: str = "program.cps") -> bytes:
    """Paquete con ensamblador y segmentos binarios producidos por MARS."""
    status = simulator_status()
    command = _command(status)
    stem = Path(filename).stem or "program"
    # Los nombres internos son fijos: un nombre enviado por HTTP no crea rutas.
    with tempfile.TemporaryDirectory(prefix="compiscript-assemble-") as temporary:
        directory = Path(temporary)
        source = directory / "program.asm"
        text = directory / "program.text.bin"
        data = directory / "program.data.bin"
        source.write_text(assembly, encoding="utf-8")
        try:
            process = subprocess.run(command + ["a", "dump", ".text", "Binary", str(text),
                                      "dump", ".data", "Binary", str(data), str(source)],
                                     text=True, capture_output=True, timeout=30,
                                     cwd=temporary, check=False)
        except (OSError, subprocess.TimeoutExpired) as error:
            raise MipsRuntimeError(f"No se pudo ensamblar el programa: {error}") from error
        if process.returncode or not text.is_file() or not text.stat().st_size:
            raise MipsRuntimeError(process.stderr.strip() or process.stdout.strip()
                                   or "MARS no produjo el segmento de código máquina.")
        manifest = {"program": stem, "architecture": "MIPS32", "simulator": "MARS 4.5",
                    "format": "raw MARS segment dumps; not ELF or a native host executable",
                    "text_address": "0x00400000", "data_address": "0x10010000",
                    "text_bytes": text.stat().st_size,
                    "data_bytes": data.stat().st_size if data.is_file() else 0,
                    "run": "java -jar /ruta/a/Mars.jar nc sm ae2 se3 program.asm"}
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w", zipfile.ZIP_DEFLATED) as archive:
            archive.writestr("program.asm", assembly)
            archive.writestr("program.text.bin", text.read_bytes())
            if data.is_file():
                archive.writestr("program.data.bin", data.read_bytes())
            archive.writestr("manifest.json", json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
            archive.writestr("LEEME.txt", "Código máquina MIPS32 ensamblado con MARS.\n"
                             "Los archivos .bin son volcados de segmentos, no un archivo ELF.\n"
                             "Para ejecutar abre program.asm en MARS, ensambla y ejecuta, o usa:\n"
                             "java -jar /ruta/a/Mars.jar nc sm ae2 se3 program.asm\n")
        return buffer.getvalue()
