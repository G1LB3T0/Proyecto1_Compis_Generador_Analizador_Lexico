#!/usr/bin/env python3
"""Obtiene el JAR oficial de MARS 4.5 y verifica su contenido."""

import hashlib
from pathlib import Path
import sys
import urllib.request


ROOT = Path(__file__).resolve().parents[1]
DESTINATION = ROOT / "tools" / "Mars.jar"
URL = "https://github.com/dpetersanderson/MARS/releases/download/v.4.5.1/Mars4_5.jar"
SHA256 = "ac340b676ba2b62246b9df77e62f81ad4447bcfd329ab539716bcd09950b7096"


def main() -> int:
    if DESTINATION.is_file() and hashlib.sha256(DESTINATION.read_bytes()).hexdigest() == SHA256:
        print("MARS 4.5 disponible y verificado.")
        return 0
    DESTINATION.parent.mkdir(parents=True, exist_ok=True)
    try:
        with urllib.request.urlopen(URL, timeout=60) as response:
            payload = response.read()
        if hashlib.sha256(payload).hexdigest() != SHA256:
            raise ValueError("El SHA-256 del JAR descargado no coincide con el esperado.")
        pending = DESTINATION.with_suffix(".jar.part")
        pending.write_bytes(payload)
        pending.replace(DESTINATION)
    except (OSError, ValueError) as error:
        print(f"No se pudo preparar MARS: {error}", file=sys.stderr)
        return 1
    print(f"MARS 4.5 descargado y verificado: {DESTINATION}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
