# Compiscript — Compilador TAC y MIPS32

Proyecto 3 de Compiladores 2, construido sobre la fase semántica y TAC,
desarrollado por:

- Joel Jaquez — #23369
- Luis Gonzalez — #23353

Proyecto de **Compiladores 2** que implementa el frontend y la generación de
código intermedio y MIPS de Compiscript con ANTLR 4.13.2 y C++17. El sistema reconoce
archivos `.cps`, construye el árbol sintáctico, ejecuta el análisis semántico y,
si el programa es válido, genera código de tres direcciones (TAC), temporales,
layout de símbolos y registros de activación. Un backend traduce el TAC a
ensamblador MIPS32 que se puede ejecutar en MARS desde el IDE o la consola.

El analizador es C++. Python solo se conserva como el adaptador Flask que sirve
el IDE y comunica el navegador con el ejecutable `compiscript_cli`; ninguna
regla léxica, sintáctica, semántica o de generación TAC/MIPS está implementada
en Python. El adaptador también invoca el simulador real para ejecutar y
ensamblar el programa.

## Estado del proyecto

- Lexer, parser y Visitor generados desde `Compiscript.g4` con el target C++ de
  ANTLR.
- Árbol sintáctico concreto navegable en el IDE.
- Tabla de símbolos jerárquica para ámbitos globales, clases, funciones y
  bloques.
- Validación de tipos, ámbitos, funciones, closures, flujo, clases, herencia,
  constructores y listas.
- Diagnósticos léxicos, sintácticos y semánticos con línea y columna.
- TAC estructurado como cuádruplos para expresiones, control de flujo,
  funciones, closures, clases y listas.
- Asignación y reciclaje de temporales con métricas observables.
- Tabla de símbolos extendida y registros de activación por función o método.
- Backend MIPS32 con protocolo de llamadas y retornos, registros físicos y
  almacenamiento de respaldo en pila.
- Ejecución del ensamblador en MARS y descarga de segmentos de código máquina.
- Batería C++ de escenarios válidos e inválidos de semántica y TAC, ejecutable
  desde CTest o el IDE.
- Ejemplos `.cps` para demostración.

## Arquitectura

```text
Código .cps
    │
    ▼
CompiscriptLexer (ANTLR, C++)
    │ tokens
    ▼
CompiscriptParser (ANTLR, C++)
    │ parse tree
    ├──────────────► serialización del árbol ► IDE
    │
    ▼
SemanticAnalyzer (Visitor C++)
    │
    ├── sistema de tipos y compatibilidad
    ├── resolución léxica de nombres
    ├── funciones, recursión y closures
    ├── flujo de control y código muerto
    ├── clases, herencia, miembros y constructores
    └── listas e índices
    │
    │ resultado válido + asociaciones semánticas
    ▼
TacGenerator (segundo Visitor C++)
    │
    ├── cuádruplos y representación textual TAC
    ├── etiquetas y saltos
    ├── llamadas, parámetros y closures
    ├── temporales con reciclaje
    └── offsets y registros de activación abstractos
    │
    ▼
MipsGenerator (backend C++)
    │
    ├── instrucciones MIPS32 y rutinas auxiliares
    ├── parámetros, llamadas, retornos y enlaces de acceso
    └── registros físicos, pila y frames efectivos
    │
    ▼
JSON: diagnósticos + símbolos + TAC + MIPS + frames
    │
    ▼
Flask (adaptador) ► IDE web
    │
    ▼
MARS ► ensamblado, código máquina y ejecución
```

El diseño detallado se encuentra en
[docs/arquitectura-compiscript.md](docs/arquitectura-compiscript.md). La
especificación completa del lenguaje intermedio está en
[docs/codigo-intermedio-tac.md](docs/codigo-intermedio-tac.md).
La fase final y su convención de llamadas se documentan en
[docs/generacion-mips.md](docs/generacion-mips.md).

## Requisitos

- CMake 3.20 o superior.
- Compilador compatible con C++17.
- Java para ejecutar la herramienta generadora de ANTLR y el simulador MARS.
- `curl` en macOS/Linux para la descarga inicial de ANTLR.
- Python 3.10 o superior con Flask para servir el IDE.

Flask es únicamente el adaptador web; el análisis sigue ejecutándose en C++.
En macOS o Linux puedes instalar la dependencia en un entorno virtual:

```bash
python3 -m venv build/venv
build/venv/bin/python -m pip install -r requirements.txt
source build/venv/bin/activate
```

En Windows PowerShell:

```powershell
py -m venv build/venv
.\build\venv\Scripts\python -m pip install -r requirements.txt
```

En los comandos posteriores de Windows, usa
`.\build\venv\Scripts\python` para ejecutar `app.py` y las pruebas del IDE.

El script de construcción descarga las distribuciones oficiales de ANTLR
4.13.2 cuando no están disponibles. El JAR y los artefactos de `build/` se
ignoran en Git; el código C++ generado sí forma parte del proyecto.

Para preparar MARS una vez:

```bash
python3 scripts/setup_mars.py
```

El script descarga el JAR oficial y verifica su SHA-256. Si ya tienes el JAR
en otra ubicación, puedes indicar su ruta mediante `MARS_JAR`; `JAVA` permite
seleccionar el ejecutable Java. Las descargas iniciales requieren conexión;
con las dependencias preparadas, la ejecución se realiza localmente.

## Compilar y probar

### macOS o Linux

```bash
./scripts/build_compiscript.sh
```

### Windows PowerShell

```powershell
.\scripts\build_compiscript.ps1
```

Ambos flujos:

1. descargan el JAR oficial de ANTLR si hace falta;
2. regeneran lexer, parser y Visitor C++;
3. preparan y verifican el simulador MARS;
4. descargan y compilan localmente el runtime C++ mediante CMake;
5. construyen `compiscript_cli` y `compiscript_tests`;
6. ejecutan las baterías C++ y de ejecución MIPS.

## Ejecutar el IDE

Después de compilar:

```bash
python3 app.py
```

Abrir `http://127.0.0.1:5050` permite:

- escribir, cargar, guardar y descargar archivos `.cps`;
- analizar el programa con `Ctrl/Cmd + Enter` o con el botón principal;
- navegar los diagnósticos y saltar a su ubicación;
- inspeccionar el TAC textual y cada cuádruplo emitido;
- inspeccionar y descargar el ensamblador MIPS32;
- ejecutar el programa en MARS y revisar su salida y errores en la consola;
- descargar un ZIP con el ensamblador y sus segmentos binarios ensamblados;
- consultar métricas de temporales y registros de activación;
- explorar el árbol sintáctico de ANTLR;
- consultar la tabla jerárquica de ámbitos y símbolos;
- revisar el sistema de tipos activo y los tipos determinados en el programa;
- inspeccionar en una tabla cada ámbito creado, su padre, propietario y símbolos;
- revisar tokens y clases;
- ejecutar la batería de pruebas C++ desde la propia interfaz.

## Uso directo del CLI C++

```bash
./compiscript_cli --file backend/examples/compiscript/bienvenido.cps
```

También recibe código por entrada estándar:

```bash
./compiscript_cli --stdin program.cps < program.cps
```

La salida es un único documento JSON con `semantic`, `tac` y `mips`. El código de salida
es `0` cuando todas las fases terminan correctamente, `1` cuando el programa
contiene diagnósticos y `2` cuando el uso del CLI o el archivo no son válidos.

Para guardar únicamente el ensamblador:

```bash
./compiscript_cli --file backend/examples/compiscript/codigo_intermedio.cps --emit-mips -o programa.asm
```

Para compilar y ejecutar un `.cps` en el simulador:

```bash
python3 scripts/run_mips.py backend/examples/compiscript/codigo_intermedio.cps -o programa.asm
```

El ejemplo integral imprime `10`. También se puede ejecutar el ensamblador
guardado y generar el paquete de código máquina:

```bash
python3 scripts/run_mips.py programa.asm --package programa.zip
```

El ZIP contiene `program.asm`, `program.text.bin`, `program.data.bin`, un
manifiesto y las instrucciones de ejecución. Los `.bin` son volcados de
segmentos MIPS, no archivos ELF ni ejecutables nativos del sistema anfitrión.
El programa se ejecuta abriendo `program.asm` en MARS o mediante el comando del
manifiesto. `--assemble-only` permite ensamblar sin ejecutar y `--json` entrega
un reporte estructurado.

## Generación de código intermedio

El TAC se ejecuta como segunda pasada sobre el árbol únicamente si el análisis
semántico fue exitoso. La implementación cubre:

- operaciones aritméticas, lógicas, comparaciones, declaraciones y
  asignaciones;
- `if/else`, `while`, `do-while`, `for`, `foreach`, `switch`, `break` y
  `continue` mediante etiquetas y saltos;
- funciones, procedimientos, parámetros, retornos, recursividad, funciones
  anidadas y enlaces de acceso para closures;
- creación de objetos, constructores, campos, métodos y `%this`;
- creación, lectura, escritura y recorrido de listas;
- asignación y reciclaje de temporales;
- offsets, tamaños, etiquetas y registros de activación.

Los cuádruplos conservan `op`, `arg1`, `arg2`, `result` y ubicación fuente. La
documentación detallada y los supuestos de traducción están en
[docs/codigo-intermedio-tac.md](docs/codigo-intermedio-tac.md).

## Reglas semánticas cubiertas

La siguiente tabla corresponde directamente a las familias exigidas por el
enunciado. Todas las comprobaciones se realizan en el Visitor C++, no en Flask
ni en JavaScript.

| Área solicitada | Comportamiento implementado |
|---|---|
| Sistema de tipos | Verifica `+`, `-`, `*`, `/` y `%` con números; `&&`, `\|\|` y `!` con booleanos; comparaciones compatibles; tipos de asignación; inicialización de `const`; y elementos de listas. Reconoce `integer`, `float`, `string`, `boolean`, `null`, `void`, clases y listas, con promoción segura `integer → float`. |
| Manejo de ámbito | Resuelve primero el entorno local y continúa por sus padres hasta el global; reporta nombres no declarados y duplicados; permite sombreado en entornos hijos; crea scopes para programa, función, clase y bloque. |
| Funciones y procedimientos | Conserva firmas, valida cantidad y tipo posicional de argumentos, comprueba retornos y caminos sin retorno, permite recursión, funciones anidadas y registra capturas de closures. |
| Control de flujo | Exige condiciones booleanas en `if`, `while`, `do-while`, `for` y `switch`; limita `break`/`continue` a ciclos y `return` a funciones; detecta instrucciones inalcanzables. |
| Clases y objetos | Comprueba miembros accedidos con `.`, firmas de constructores, uso contextual de `this`, herencia simple, clases base, ciclos y compatibilidad de subtipo. |
| Listas y estructuras | Infiere un tipo común para los elementos, maneja listas anidadas y vacías tipadas, exige índices `integer` y valida que `foreach` reciba una lista. |
| Reglas generales | Rechaza objetivos de asignación inválidos, operaciones sin sentido como multiplicar una función y declaraciones duplicadas de variables, funciones o parámetros. |

## Alcance de los archivos `.cps`

El analizador no depende del nombre ni del contenido de los ejemplos incluidos.
Puede recibir un archivo `.cps` nuevo y desconocido: ANTLR lo tokeniza y analiza
desde cero, y el Visitor construye nuevas tablas de tipos, símbolos y ámbitos
para esa entrada.

Esto no significa que acepte cualquier texto o cualquier programa TypeScript.
El archivo debe respetar la sintaxis definida en
`backend/compiscript/grammar/Compiscript.g4`, porque Compiscript es solamente un
subconjunto del lenguaje. Si la sintaxis es válida pero viola una regla
semántica, el funcionamiento correcto es rechazarlo con diagnósticos; no se
considera una falla del analizador.

Existe una contradicción en el material oficial: la rúbrica exige que
`switch` reciba una condición `boolean`, mientras el documento de ejemplos usa
un selector `integer`. La implementación prioriza la regla explícita de la
rúbrica. Esta decisión y las demás extensiones de compatibilidad se explican en
[docs/decisiones-compiscript.md](docs/decisiones-compiscript.md).

## Batería de pruebas

```bash
ctest --test-dir build/compiscript --output-on-failure
```

La suite está en
[backend/tests/compiscript_tests.cpp](backend/tests/compiscript_tests.cpp) y
contiene **115 escenarios** válidos y errores u omisiones esperadas, agrupados por
cada familia de reglas. No depende de un framework externo: enlaza
directamente con `compiscript_core`, de modo que prueba el mismo servicio usado
por el CLI y el IDE.

El botón `Ejecutar batería de pruebas` presenta el reporte completo en la
interfaz. Para cada caso permite inspeccionar la regla, el código Compiscript
ejecutado, una explicación sencilla de lo que sucede, el resultado esperado,
el resultado obtenido y los códigos de diagnóstico emitidos. En los casos
negativos se aclara que el programa es incorrecto a propósito y por qué la
prueba se considera aprobada cuando el analizador lo rechaza con el diagnóstico
correcto; no basta con que la entrada falle por cualquier motivo.

La cobertura TAC incluye precedencia, cortocircuito lógico, ciclos, `switch`,
recursividad, closures, clases, listas, temporales, offsets, frames y la omisión
segura del generador ante errores sintácticos o semánticos.

La batería de ejecución MIPS está en
[backend/tests/mips_execution_tests.py](backend/tests/mips_execution_tests.py).
Contiene **66 casos: 54 válidos y 12 errores esperados**. Compila cada programa,
ejecuta el ensamblador en MARS y compara su salida con
el resultado esperado. También puede ejecutarse directamente:

```bash
python3 backend/tests/mips_execution_tests.py
```

`--filter texto` selecciona casos por nombre o categoría y `--json` presenta el
reporte que utiliza el IDE. En CTest aparecen `compiscript_semantic_suite` y
`compiscript_mips_execution` cuando Python, Java y el JAR de MARS están
disponibles al configurar CMake.

La batería incluye una adaptación de un algoritmo del compilador
a Compiscript como prueba integral; se puede abrir como
`algoritmo_compilador.cps` en el IDE. La entrada del compilador es Compiscript,
por lo que esa prueba utiliza la gramática `.g4` y conserva la referencia a su
algoritmo C++ de origen.

Las **12 pruebas de integración del IDE** comprueban las rutas de compilación,
ejecución y descarga con el compilador y MARS reales, además de entradas
inválidas, ausencia del simulador y límites de ejecución. Después de instalar
`requirements.txt` en el entorno Python activo:

```bash
python3 backend/tests/ide_integration_tests.py
```

Estas pruebas del adaptador se ejecutan por separado de CTest. El conjunto
automatizado suma **193 casos**, además de la comprobación de la interfaz en
el navegador.

## Entregables y evaluación

| Componente evaluado | Evidencia en el proyecto |
|---|---|
| Llamadas y retornos — 20 puntos | Protocolo de parámetros y retornos, frames, enlaces léxicos, recursión y preservación del estado. |
| Asignación de registros — 20 puntos | Asignación de registros físicos, almacenamiento en pila y métricas. |
| Código MIPS — 60 puntos | Generación desde TAC, ensamblado y ejecución en MARS, con casos exitosos y fallidos. |

## Decisiones sobre el material oficial

La gramática entregada, la especificación y el enunciado contienen tres puntos
que no coinciden entre sí. Se resolvieron sin quitar construcciones oficiales:

- se agregó `float`, exigido por la rúbrica aunque no aparece en la gramática;
- `+` acepta dos `string`, tal como muestran los ejemplos oficiales;
- los cuerpos de control aceptan bloque o una sentencia, porque el ejemplo de
  recursión usa `if (...) return ...;` sin llaves.

La rúbrica indica expresamente que la condición de `switch` debe ser booleana;
el analizador aplica esa regla, aunque otro ejemplo del documento usa un
selector entero. Las decisiones completas están en
[docs/decisiones-compiscript.md](docs/decisiones-compiscript.md).

## Estructura relevante

```text
backend/compiscript/
├── grammar/Compiscript.g4       # Fuente de verdad sintáctica
├── generated/                   # Lexer, parser y Visitor C++ de ANTLR
├── include/compiscript/         # API y modelos del frontend
└── src/                         # Visitors semántico/TAC, backend MIPS, JSON y CLI
backend/examples/compiscript/    # Programas .cps del IDE
backend/tests/compiscript_tests.cpp
backend/tests/mips_execution_tests.py
backend/tests/ide_integration_tests.py
frontend/                        # IDE
scripts/                         # Generación y construcción reproducible
CMakeLists.txt
requirements.txt                 # Dependencia del adaptador web
app.py                           # Solo adaptador web
mips_runtime.py                  # Puente hacia MARS y paquetes de código máquina
```

El libro *Compilers: Principles, Techniques, and Tools* se usa como consulta
local, especialmente para tablas de símbolos encadenadas, síntesis de tipos y
acceso a datos no locales. No se redistribuye en el repositorio.

El trabajo de Compiladores 1 permanece intacto en la rama `compiladores-1` y
en la etiqueta `compiladores-1-final`. Sus módulos YALex/YAPar siguen presentes
en esta rama como antecedente histórico, pero no forman parte del pipeline de
Compiscript ni del IDE actual.
