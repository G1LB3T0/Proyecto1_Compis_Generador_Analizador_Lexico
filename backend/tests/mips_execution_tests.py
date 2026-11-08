#!/usr/bin/env python3
"""Pruebas de extremo a extremo: Compiscript -> TAC -> MIPS -> MARS.

No interpreta Compiscript ni TAC: los resultados observados proceden del
ensamblador ejecutado por MARS. Cada caso válido exige además un volcado binario
no vacío de .text, para verificar que el ensamblado produjo código máquina.

Uso:
  python3 backend/tests/mips_execution_tests.py --cli ./compiscript_cli --mars tools/Mars.jar
  python3 backend/tests/mips_execution_tests.py --json --filter recurs
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import math
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time
from typing import Any


ROOT = Path(__file__).resolve().parents[2]


@dataclass(frozen=True)
class Case:
    name: str
    category: str
    source: str
    output: str = ""
    diagnostic: str = ""
    runtime_error: str = ""


def cases() -> list[Case]:
    """Salidas esperadas calculadas a partir de los programas, no del TAC."""
    return [
        Case("Programa vacío", "Proceso completo", "", ""),
        Case("Precedencia y aritmética entera", "Expresiones",
             "print(2 + 3 * 4); print((8 + 2) * 3 - 4 % 2); print(-7 % 3);", "14\n30\n-1\n"),
        Case("División, promoción y negación flotante", "Expresiones",
             "print(9 / 2); print(2 + 3.5); print(-2.25 * 2);", "4.5\n5.5\n-4.5\n"),
        Case("Comparaciones numéricas mixtas", "Expresiones",
             "print(1 < 2.5); print(2.0 == 2); print(2.5 != 2); print(3 >= 3.0); print(4 <= 2); print(5 > 4);",
             "true\ntrue\ntrue\ntrue\nfalse\ntrue\n"),
        Case("Resto de números flotantes", "Expresiones",
             "print(5.5 % 2.0); print(-5.5 % 2.0);", "1.5\n-1.5\n"),
        Case("Resto flotante con cociente mayor a integer32", "Expresiones",
             "print(10000000000.5 % 3.0); print(-10000000000.5 % 3.0);", "1.5\n-1.5\n"),
        Case("Cortocircuito con efectos laterales", "Control de flujo",
             """let visitas = 0;
             function tocar(): boolean { visitas = visitas + 1; return true; }
             let a = false && tocar(); let b = true || tocar();
             print(visitas); print(a); print(b); print(!b);""", "0\nfalse\ntrue\nfalse\n"),
        Case("If, else y ternario", "Control de flujo",
             "let x = 3; if (x < 2) print(10); else print(20); print(true ? 7 : 8); print(false ? 1 : 2.5);",
             "20\n7\n2.5\n"),
        Case("While, do while y for", "Control de flujo",
             """let suma = 0; let i = 0;
             while (i < 3) { suma = suma + i; i = i + 1; }
             do { suma = suma + 5; } while (false);
             for (let j = 0; j < 4; j = j + 1) { suma = suma + j; }
             print(suma);""", "14\n"),
        Case("For sin condición con incremento", "Control de flujo",
             "for (let i = 0; ; i = i + 1) { if (i == 3) { break; } print(i); }", "0\n1\n2\n"),
        Case("Break y continue anidados", "Control de flujo",
             """let total = 0;
             for (let i = 0; i < 4; i = i + 1) {
               if (i == 1) { continue; }
               for (let j = 0; j < 4; j = j + 1) {
                 if (j == 2) { break; }
                 total = total + i + j;
               }
             }
             print(total);""", "13\n"),
        Case("Switch sin caída implícita", "Control de flujo",
             "switch (false) { case true: print(1); case false: print(2); default: print(3); }", "2\n"),
        Case("Factorial recursivo", "Llamadas y retornos",
             "function factorial(n: integer): integer { if (n <= 1) return 1; return n * factorial(n - 1); } print(factorial(6));",
             "720\n"),
        Case("Fibonacci y valores vivos entre llamadas", "Llamadas y retornos",
             "function fib(n: integer): integer { if (n < 2) return n; return fib(n - 1) + fib(n - 2); } print(fib(10));",
             "55\n"),
        Case("Recursión mutua", "Llamadas y retornos",
             """function par(n: integer): boolean { if (n == 0) return true; return impar(n - 1); }
             function impar(n: integer): boolean { if (n == 0) return false; return par(n - 1); }
             print(par(10)); print(impar(7));""", "true\ntrue\n"),
        Case("Seis parámetros y orden posicional", "Llamadas y retornos",
             "function suma(a: integer,b: integer,c: integer,d: integer,e: integer,f: integer): integer { return a+b+c+d+e+f; } print(suma(1,2,3,4,5,6));",
             "21\n"),
        Case("Llamadas en argumentos", "Llamadas y retornos",
             "function mas(x: integer): integer { return x+1; } function unir(a: integer,b: integer): integer { return a*10+b; } print(unir(mas(1),unir(mas(2),mas(3))));",
             "54\n"),
        Case("Temporales vivos alrededor de llamadas", "Asignación de registros",
             "function triple(x: integer): integer { return x * 3; } print((1 + 2) * triple(4) + (5 + 6) * triple(2));",
             "102\n"),
        Case("Presión de temporales con doce argumentos", "Asignación de registros",
             """function suma12(a:integer,b:integer,c:integer,d:integer,e:integer,f:integer,g:integer,h:integer,i:integer,j:integer,k:integer,l:integer): integer {
               return a+b+c+d+e+f+g+h+i+j+k+l;
             }
             print(suma12(1+1,2+2,3+3,4+4,5+5,6+6,7+7,8+8,9+9,10+10,11+11,12+12));""", "156\n"),
        Case("Locales preservados durante recursión", "Llamadas y retornos",
             "function acumular(n: integer): integer { if (n == 0) return 0; let local = n * 3; return local + acumular(n-1); } print(acumular(5));",
             "45\n"),
        Case("Procedimiento sin retorno y función sin parámetros", "Llamadas y retornos",
             "let x = 1; function incrementar() { x = x + 1; } function valor(): integer { return x; } incrementar(); print(valor()); incrementar(); print(valor());",
             "2\n3\n"),
        Case("Paso y retorno flotantes con promoción", "Llamadas y retornos",
             "function mitad(x: float): float { return x / 2; } print(mitad(9)); print(mitad(2.5));", "4.5\n1.25\n"),
        Case("Evaluación de argumentos de izquierda a derecha", "Orden de evaluación",
             "function par(a:integer,b:integer):integer { return a*10+b; } let x = 1; print(par(x,x=2)); print(x);", "12\n2\n"),
        Case("Operando izquierdo conservado ante efecto lateral", "Orden de evaluación",
             "let x = 2; function cambiar():integer { x = 5; return 3; } print(x + cambiar()); print(x);", "5\n5\n"),
        Case("Objetivo indexado conservado ante efecto lateral", "Orden de evaluación",
             "let datos=[10,20]; let i=0; function cambiar():integer { i=1; return 7; } datos[i]=cambiar(); print(datos[0]); print(datos[1]);", "7\n20\n"),
        Case("Sombreado entre bloques y función", "Ámbitos y closures",
             "let x=1; function f():integer { let x=2; { let x=3; print(x); } return x; } print(f()); print(x);", "3\n2\n1\n"),
        Case("Closure modifica parámetro del entorno", "Ámbitos y closures",
             "function exterior(base:integer):integer { function agregar(n:integer):integer { base=base+n; return base; } agregar(2); return agregar(3); } print(exterior(10));", "15\n"),
        Case("Closure accede a dos niveles léxicos", "Ámbitos y closures",
             "function exterior(a:integer):integer { function medio(b:integer):integer { function interior(c:integer):integer { return a+b+c; } return interior(3); } return medio(2); } print(exterior(10));", "15\n"),
        Case("Llamada entre funciones hermanas", "Ámbitos y closures",
             "function exterior(a:integer):integer { function primera(b:integer):integer { return a+b; } function segunda(c:integer):integer { return primera(c)*2; } return segunda(3); } print(exterior(10));", "26\n"),
        Case("Recursión de función anidada con captura", "Ámbitos y closures",
             "function exterior(base:integer):integer { function suma(n:integer):integer { if(n==0)return base; return n+suma(n-1); } return suma(4); } print(exterior(7));", "17\n"),
        Case("Concatenación y comparación por contenido", "Cadenas",
             'function unir(a:string,b:string):string { return a+b; } let s=unir("compi","script"); print(s); print(s=="compiscript"); print(s!="otro");',
             "compiscript\ntrue\ntrue\n"),
        Case("Orden lexicográfico de cadenas", "Cadenas",
             'print("a"<"b"); print("ab"<="ab"); print("b">"aa"); print("a">="b");',
             "true\ntrue\ntrue\nfalse\n"),
        Case("Cadena vacía y escapes", "Cadenas",
             r'print(""+"hola"); print("a\tb"); print("comillas: \" y barra: \\");',
             'hola\na\tb\ncomillas: " y barra: ' + chr(92) + '\n'),
        Case("Cadenas UTF-8 y concatenación Unicode", "Cadenas",
             'print("café"+" ☕"); print("漢字"=="漢"+"字"); print("Compilación 😊");',
             "café ☕\ntrue\nCompilación 😊\n"),
        Case("Creación, escritura y foreach de lista", "Listas",
             "let datos=[1,2,3]; datos[1]=7; let suma=0; foreach(dato in datos) { suma=suma+dato; } print(suma);", "11\n"),
        Case("Lista anidada y acceso como destino", "Listas",
             "let datos=[[1,2],[3,4]]; datos[1][0]=datos[0][1]+5; print(datos[1][0]); print(datos[0][0]);", "7\n1\n"),
        Case("Promoción de elementos de lista flotante", "Listas",
             "let datos:float[]=[1,2.5,3]; let suma:float=0; foreach(dato in datos) { suma=suma+dato; } print(suma);", "6.5\n"),
        Case("Lista pasada por referencia", "Listas",
             "function cambiar(datos:integer[]) { datos[0]=42; } let lista=[1,2]; cambiar(lista); print(lista[0]);", "42\n"),
        Case("Impresión de listas anidadas y null", "Listas",
             "print([[1,2],[3,4]]); print([true,false]); print(null);", "[[1, 2], [3, 4]]\n[true, false]\nnull\n"),
        Case("Constructor, campos y métodos", "Clases y objetos",
             "class Caja { let valor:integer; function constructor(v:integer) { this.valor=v; } function sumar(n:integer):integer { this.valor=this.valor+n; return this.valor; } } let a=new Caja(5); let b=new Caja(20); print(a.sumar(3)); print(b.valor); print(a.valor);", "8\n20\n8\n"),
        Case("Inicializadores de campos por instancia", "Clases y objetos",
             "class Caja { let datos:integer[]=[1,2]; let valor:integer=4; function aumentar():integer { this.valor=this.valor+1; return this.valor; } } let a=new Caja(); let b=new Caja(); a.datos[0]=9; print(b.datos[0]); print(a.aumentar()); print(b.valor);", "1\n5\n4\n"),
        Case("Campos heredados e inicializadores", "Clases y objetos",
             "class Base { let x:integer=7; function valor():integer { return this.x; } } class Hija:Base { let y:integer=5; function total():integer { return this.x+this.y; } } let h=new Hija(); print(h.valor()); print(h.total());", "7\n12\n"),
        Case("Inicializador de campo consulta this", "Clases y objetos",
             "class A { let x:integer=4; let y:integer=this.x+3; } let a=new A(); print(a.y);", "7\n"),
        Case("Receptor preservado entre llamadas a métodos", "Clases y objetos",
             "class A { let x:integer=1; function add(n:integer):integer { this.x=this.x+n; return this.x; } function ambos():integer { return this.add(2)+this.add(3); } } let a=new A(); print(a.ambos()); print(a.x);", "9\n6\n"),
        Case("Receptor implícito al llamar método por nombre", "Clases y objetos",
             "class C { let x:integer=7; function leer():integer { return x; } function valor():integer { return leer(); } } let c=new C(); print(c.valor());", "7\n"),
        Case("Función anidada dentro de método captura this", "Ámbitos y closures",
             "class A { let x:integer=4; function valor():integer { function interior():integer { return this.x+1; } return interior(); } } let a=new A(); print(a.valor());", "5\n"),
        Case("Inicializador de clase local captura parámetro exterior", "Ámbitos y closures",
             "function f(n:integer):integer { class C { let x:integer=n; } let c=new C(); return c.x; } print(f(7));", "7\n"),
        Case("Lista de objetos", "Clases y objetos",
             "class A { let x:integer; function constructor(n:integer) { this.x=n; } } let objetos=[new A(2),new A(3)]; print(objetos[0].x+objetos[1].x);", "5\n"),
        Case("Prueba de fuego: algoritmos del compilador", "Algoritmos del compilador",
             (ROOT / "backend/examples/compiscript/algoritmo_compilador.cps").read_text(encoding="utf-8"),
             "2\n-1\n7\n0\n1\n0\n2\n2\n3\n2\n3\n"),
        Case("Error léxico impide generar ensamblador", "Errores de compilación", "let x=1; @", diagnostic="LEX001"),
        Case("Error sintáctico impide generar ensamblador", "Errores de compilación", "let x = ;", diagnostic="SYN001"),
        Case("Error semántico impide generar ensamblador", "Errores de compilación", "let x:integer=true;", diagnostic="SEM_ASSIGN_TYPE"),
        Case("Nombre no declarado", "Errores de compilación", "print(desconocido);", diagnostic="SEM_SCOPE_UNDECLARED"),
        Case("Aridad incorrecta", "Errores de compilación", "function f(x:integer):integer{return x;} print(f());", diagnostic="SEM_FUNCTION_ARITY"),
        Case("Literal entero fuera de rango MIPS32", "Errores de generación",
             "print(2147483648);", diagnostic="MIPS_INTEGER_RANGE"),
        Case("C++ del compilador requiere portarse a Compiscript", "Errores de compilación",
             '#include <set>\nint bestRule() { return -1; }', diagnostic="LEX001"),
        Case("Break de función no puede salir del ciclo exterior", "Errores de compilación",
             "while(true) { function f() { break; } break; }", diagnostic="SEM_FLOW_BREAK"),
        Case("División por cero sin capturar", "Errores de ejecución", "print(1/0);", runtime_error="división por cero"),
        Case("Índice superior fuera de rango", "Errores de ejecución", "let a=[1,2]; print(a[2]);", runtime_error="índice fuera de rango"),
        Case("Índice negativo fuera de rango", "Errores de ejecución", "let a=[1,2]; a[-1]=3;", runtime_error="índice fuera de rango"),
        Case("Referencia nula al leer campo", "Errores de ejecución", "class A { let x:integer; } let a:A=null; print(a.x);", runtime_error="referencia nula"),
        Case("Try catch captura división por cero", "Excepciones",
             'try { print(1/0); } catch(error) { print(error); } print("continua");', "división por cero\ncontinua\n"),
        Case("Try catch captura error de función llamada", "Excepciones",
             'function fallar():integer { let a=[1]; return a[2]; } try { print(fallar()); } catch(error) { print(error); } print(9);', "índice fuera de rango\n9\n"),
        Case("Retorno dentro de try restaura manejadores", "Excepciones",
             'function valor():integer { try { return 1; } catch(e) { return 2; } } print(valor()); try { print(1/0); } catch(e) { print("capturado"); }',
             "1\ncapturado\n"),
        Case("Manejadores anidados recuperan el entorno correcto", "Excepciones",
             'try { try { print(1/0); } catch(interior) { print("interior"); } let a=[1]; print(a[3]); } catch(exterior) { print("exterior"); } print(9);',
             "interior\nexterior\n9\n"),
        Case("Continue dentro de try restaura manejadores", "Excepciones",
             'for(let i=0;i<3;i=i+1) { try { if(i<2)continue; print(i); } catch(e) { print(99); } } try { print(1/0); } catch(e) { print("final"); }',
             "2\nfinal\n"),
    ]


def canonical_lines(value: str) -> list[str]:
    # Se conserva el interior de cada línea (incluye espacios y tabuladores).
    return value.replace("\r\n", "\n").replace("\r", "\n").split("\n")


NUMBER = re.compile(r"^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?$")


def output_matches(actual: str, expected: str) -> bool:
    obtained, wanted = canonical_lines(actual), canonical_lines(expected)
    if len(obtained) != len(wanted):
        return False
    for left, right in zip(obtained, wanted):
        if left == right:
            continue
        if not (NUMBER.fullmatch(left) and NUMBER.fullmatch(right)):
            return False
        # Enteros se comparan exactamente; decimales admiten formato de MARS.
        if "." not in right and "e" not in right.lower():
            if float(left) != int(right):
                return False
        elif not math.isclose(float(left), float(right), rel_tol=1e-9, abs_tol=1e-10):
            return False
    return True


def run_case(case: Case, args: argparse.Namespace, directory: Path, index: int) -> dict[str, Any]:
    started = time.monotonic()
    item: dict[str, Any] = {
        "category": case.category, "rule": case.category, "name": case.name,
        "kind": "Error esperado" if case.diagnostic or case.runtime_error else "Caso válido",
        "source": case.source, "expected": case.diagnostic or case.runtime_error or case.output,
        "explanation": "Compila el programa y comprueba los resultados del ensamblador ejecutado en MARS.",
        "passed": False, "actual": "", "codes": [], "stdout": "", "stderr": "",
        "assembly": "", "machine_code_bytes": 0,
    }
    try:
        compilation = subprocess.run(
            [str(args.cli), "--stdin", f"mips_case_{index}.cps"], input=case.source,
            capture_output=True, text=True, encoding="utf-8", timeout=args.timeout,
            cwd=ROOT, check=False,
        )
        try:
            result = json.loads(compilation.stdout)
        except json.JSONDecodeError as error:
            item["actual"] = f"El CLI no produjo JSON válido: {error}; {compilation.stderr.strip()}"
            return item
        diagnostics = result.get("diagnostics", [])
        item["codes"] = [str(diagnostic.get("code", "")) for diagnostic in diagnostics]
        mips = result.get("mips", {})
        if case.diagnostic:
            empty = not str(mips.get("text", "")).strip()
            stopped = not mips.get("ok") and empty
            if not case.diagnostic.startswith("MIPS_"):
                stopped = stopped and bool(mips.get("skipped"))
            located = any(d.get("code") == case.diagnostic and d.get("line", 0) > 0 and
                          d.get("column", 0) > 0 for d in diagnostics)
            item["passed"] = compilation.returncode != 0 and not result.get("ok") and stopped and located
            item["actual"] = f"Diagnósticos: {', '.join(item['codes'])}; MIPS sin salida: {stopped}; ubicación: {located}"
            return item
        if compilation.returncode != 0 or not result.get("ok") or not mips.get("ok"):
            item["actual"] = "Compilación fallida: " + "; ".join(
                f"{d.get('code')}: {d.get('message')}" for d in diagnostics)
            if not mips:
                item["actual"] += " (el CLI aún no incluye el resultado MIPS)"
            return item
        assembly = str(mips.get("text", ""))
        if not assembly.strip():
            item["actual"] = "MIPS se marcó exitoso, pero el ensamblador está vacío."
            return item
        item["assembly"] = assembly
        asm_path = directory / f"case_{index}.asm"
        binary_path = directory / f"case_{index}.bin"
        asm_path.write_text(assembly, encoding="utf-8")
        simulation = subprocess.run(
            [args.java, "-Djava.awt.headless=true", "-jar", str(args.mars),
             "nc", "sm", "me", "ae12", "se13", str(args.max_steps),
             "dump", ".text", "Binary", str(binary_path), str(asm_path)],
            capture_output=True, text=True, encoding="utf-8", errors="replace",
            timeout=args.timeout, cwd=directory, check=False,
        )
        item["stdout"] = simulation.stdout
        item["stderr"] = simulation.stderr
        item["returncode"] = simulation.returncode
        item["step_limit"] = "maximum step limit" in (simulation.stdout + simulation.stderr).lower()
        item["machine_code_bytes"] = binary_path.stat().st_size if binary_path.is_file() else 0
        if case.runtime_error:
            item["passed"] = (simulation.returncode == 2 and not item["step_limit"] and
                              output_matches(simulation.stdout, "Error de ejecución: " + case.runtime_error + "\n"))
        else:
            item["passed"] = (simulation.returncode == 0 and not item["step_limit"] and item["machine_code_bytes"] > 0 and
                              output_matches(simulation.stdout, case.output))
        item["actual"] = (f"Salida: {simulation.stdout!r}; código: {simulation.returncode}; "
                          f"código máquina: {item['machine_code_bytes']} bytes")
        if simulation.stderr.strip() and not item["passed"]:
            item["actual"] += f"; MARS: {simulation.stderr.strip()}"
    except subprocess.TimeoutExpired:
        item["actual"] = f"La compilación o simulación excedió {args.timeout} segundos."
    except OSError as error:
        item["actual"] = f"No se pudo ejecutar la herramienta: {error}"
    finally:
        item["duration_ms"] = round((time.monotonic() - started) * 1000)
    return item


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", type=Path,
                        default=ROOT / ("compiscript_cli.exe" if sys.platform == "win32" else "compiscript_cli"))
    parser.add_argument("--mars", type=Path, default=ROOT / "tools/Mars.jar")
    parser.add_argument("--java", default="java")
    parser.add_argument("--json", action="store_true", help="Reporte JSON para CTest o el IDE.")
    parser.add_argument("--filter", default="", help="Filtrar por parte del nombre o categoría.")
    parser.add_argument("--timeout", type=float, default=15.0)
    parser.add_argument("--max-steps", type=int, default=1000000)
    args = parser.parse_args()
    if not 32 <= args.max_steps <= 10_000_000:
        parser.error("--max-steps debe estar entre 32 y 10000000 (MARS reserva 1..31 para registros).")
    if args.timeout <= 0:
        parser.error("--timeout debe ser positivo.")
    args.cli, args.mars = args.cli.resolve(), args.mars.resolve()
    missing = [str(path) for path in (args.cli, args.mars) if not path.is_file()]
    if missing:
        error = "No se encontraron herramientas requeridas: " + ", ".join(missing)
        if args.json:
            print(json.dumps({"ok": False, "error": error, "total": 0, "passed": 0, "failed": 0, "tests": []}, ensure_ascii=False))
        else:
            print(error, file=sys.stderr)
        return 2
    selected = [case for case in cases() if args.filter.casefold() in f"{case.category} {case.name}".casefold()]
    if not selected:
        parser.error("El filtro no seleccionó ninguna prueba.")
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="compiscript-mips-tests-") as temporary:
        results = [run_case(case, args, Path(temporary), index) for index, case in enumerate(selected)]
    passed = sum(bool(item["passed"]) for item in results)
    report = {
        "ok": passed == len(results), "total": len(results), "passed": passed,
        "failed": len(results) - passed, "simulator": "MARS", "tests": results,
        "duration_ms": round((time.monotonic() - started) * 1000),
        "machine_code_bytes": sum(item["machine_code_bytes"] for item in results),
    }
    report["summary"] = {
        "total": report["total"], "passed": passed, "failed": report["failed"],
        "valid_cases": sum(item["kind"] == "Caso válido" for item in results),
        "expected_error_cases": sum(item["kind"] == "Error esperado" for item in results),
    }
    report["categories"] = []
    for name in dict.fromkeys(item["category"] for item in results):
        group = [item for item in results if item["category"] == name]
        report["categories"].append({
            "name": name, "total": len(group),
            "passed": sum(bool(item["passed"]) for item in group), "tests": group,
        })
    if args.json:
        print(json.dumps(report, ensure_ascii=False))
    else:
        for item in results:
            print(f"{'PASS' if item['passed'] else 'FAIL'} {item['category']}: {item['name']}")
            if not item["passed"]:
                print("  " + item["actual"])
        print(f"{passed}/{len(results)} pruebas MIPS aprobadas; {report['machine_code_bytes']} bytes de código máquina ensamblados.")
    return 0 if report["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
