#!/usr/bin/env python3
"""Compila .cps o ejecuta .asm en MARS, con exportación opcional de binarios."""

import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mips_runtime import MipsRuntimeError, machine_code_package, run_mips


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=Path, help="Archivo Compiscript .cps o ensamblador .asm")
    parser.add_argument("--cli", type=Path, default=ROOT / ("compiscript_cli.exe" if sys.platform == "win32" else "compiscript_cli"))
    parser.add_argument("--output", "-o", type=Path, help="Guardar ensamblador generado")
    parser.add_argument("--package", type=Path, help="Guardar ZIP con ensamblador y segmentos binarios")
    parser.add_argument("--assemble-only", action="store_true")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--max-steps", type=int, default=1_000_000)
    parser.add_argument("--delayed-branches", action="store_true")
    args = parser.parse_args()
    try:
        if args.file.suffix == ".cps":
            compiled = subprocess.run([str(args.cli.resolve()), "--file", str(args.file.resolve())],
                                      text=True, capture_output=True, timeout=30, check=False)
            result = json.loads(compiled.stdout)
            if compiled.returncode or not result.get("mips_ok"):
                print(json.dumps(result, indent=2, ensure_ascii=False), file=sys.stderr)
                return 1
            assembly = result["mips"]["text"]
        elif args.file.suffix == ".asm":
            assembly = args.file.read_text(encoding="utf-8")
        else:
            parser.error("La entrada debe tener extensión .cps o .asm.")
        if args.output:
            args.output.write_text(assembly, encoding="utf-8")
        package = None
        if args.package or args.assemble_only:
            package = machine_code_package(assembly, args.file.name)
            if args.package:
                args.package.write_bytes(package)
        if args.assemble_only:
            report = {"ok": True, "phase": "assembly", "simulator": "MARS 4.5",
                      "package_bytes": len(package), "stdout": "", "stderr": ""}
        else:
            report = run_mips(assembly, max_steps=args.max_steps, delayed_branches=args.delayed_branches)
        if args.json:
            print(json.dumps(report, indent=2, ensure_ascii=False))
        else:
            print(report["stdout"], end="")
            if report["stderr"]:
                print(report["stderr"], file=sys.stderr, end="")
            if not report["ok"]:
                print(report.get("error", "Error MIPS"), file=sys.stderr)
        return 0 if report["ok"] else 1
    except (OSError, ValueError, subprocess.TimeoutExpired, MipsRuntimeError) as error:
        print(str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
