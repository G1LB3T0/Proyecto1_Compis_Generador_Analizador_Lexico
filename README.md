# Compiscript — Generador de código intermedio TAC

Proyecto 2 de Compiladores 2 desarrollado por:

- Joel Jaquez — #23369
- Luis Gonzalez — #23353

Proyecto de **Compiladores 2** que implementa el frontend y la generación de
código intermedio de Compiscript con ANTLR 4.13.2 y C++17. El sistema reconoce
archivos `.cps`, construye el árbol sintáctico, ejecuta el análisis semántico y,
si el programa es válido, genera código de tres direcciones (TAC), temporales,
layout de símbolos y registros de activación en un IDE web.

El analizador es C++. Python solo se conserva como el adaptador Flask que sirve
el IDE y comunica el navegador con el ejecutable `compiscript_cli`; ninguna
regla léxica, sintáctica, semántica o de generación TAC está implementada en
Python.

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
    └── offsets y registros de activación
    │
    ▼
JSON: diagnósticos + símbolos + TAC + frames
    │
    ▼
Flask (adaptador) ► IDE web
```

El diseño detallado se encuentra en
[docs/arquitectura-compiscript.md](docs/arquitectura-compiscript.md). La
especificación completa del lenguaje intermedio está en
[docs/codigo-intermedio-tac.md](docs/codigo-intermedio-tac.md).

## Requisitos

- CMake 3.20 o superior.
- Compilador compatible con C++17.
- Java para ejecutar la herramienta generadora de ANTLR.
- `curl` en macOS/Linux para la descarga inicial de ANTLR.
- Python 3 con Flask para servir el IDE.

Flask es únicamente el adaptador web; el análisis sigue ejecutándose en C++.
Para instalar la dependencia del IDE:

```bash
python3 -m pip install -r requirements.txt
```

El script de construcción descarga las distribuciones oficiales de ANTLR
4.13.2 cuando no están disponibles. El JAR y los artefactos de `build/` se
ignoran en Git; el código C++ generado sí forma parte del proyecto.

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
3. descargan y compilan localmente el runtime C++ mediante CMake;
4. construyen `compiscript_cli` y `compiscript_tests`;
5. ejecutan la batería automatizada.

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

La salida es un único documento JSON con `semantic` y `tac`. El código de salida
es `0` cuando todas las fases terminan correctamente, `1` cuando el programa
contiene diagnósticos y `2` cuando el uso del CLI o el archivo no son válidos.

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
contiene 107 escenarios: 51 programas válidos y 56 errores u omisiones
esperadas, agrupados por cada familia de reglas. No depende de un framework externo: enlaza
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

## Entregables y evaluación

| Componente evaluado | Evidencia en el proyecto |
|---|---|
| Diseño de CI — 25 puntos | Cuádruplos documentados, catálogo de instrucciones, ejemplos y supuestos de traducción. |
| Generación de TAC — 65 puntos | Segundo Visitor C++, temporales, control de flujo, funciones, clases, listas y batería automatizada. |
| Tabla de símbolos — 10 puntos | Tamaños, offsets, etiquetas, profundidad léxica y registros de activación. |

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
└── src/                         # Visitors semántico/TAC, JSON, servicio y CLI
backend/examples/compiscript/    # Programas .cps del IDE
backend/tests/compiscript_tests.cpp
frontend/                        # IDE
scripts/                         # Generación y construcción reproducible
CMakeLists.txt
requirements.txt                 # Dependencia del adaptador web
app.py                           # Solo adaptador web
```

## Material e historial

- [Enunciado del Proyecto 2](Instrucciones/Proyecto_2__Compiladores.md)
- [Enunciado semántico previo](Instrucciones/Generador_de_Analizadores_Semánticos.md)
- [Especificación](Instrucciones/Especificaciones%20(1).md)
- [Gramática originalmente entregada](Instrucciones/Compiscript%20(1).g4)
- [ANTLR](https://www.antlr.org/)

El libro *Compilers: Principles, Techniques, and Tools* se usa como consulta
local, especialmente para tablas de símbolos encadenadas, síntesis de tipos y
acceso a datos no locales. No se redistribuye en el repositorio.

El trabajo de Compiladores 1 permanece intacto en la rama `compiladores-1` y
en la etiqueta `compiladores-1-final`. Sus módulos YALex/YAPar siguen presentes
en esta rama como antecedente histórico, pero no forman parte del pipeline de
Compiscript ni del IDE actual.
