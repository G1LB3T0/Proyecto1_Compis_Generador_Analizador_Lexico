# Arquitectura del compilador de Compiscript

## Separación de responsabilidades

El sistema tiene un núcleo C++ independiente de la interfaz. La función
`analyzeSource` recibe texto y devuelve un `AnalysisResult`; el CLI, las pruebas
y Flask consumen esa misma API, por lo que no existen tres implementaciones
distintas del compilador.

1. `CompiscriptLexer` convierte caracteres en tokens y entrega errores léxicos
   a `CollectingErrorListener`.
2. `CompiscriptParser` aplica la gramática y construye el parse tree.
3. `tree_serializer` transforma el árbol en una estructura JSON recursiva con
   regla, token, texto, línea y columna.
4. `SemanticAnalyzer`, derivado de `CompiscriptBaseVisitor`, sintetiza el tipo
   de cada expresión, aplica reglas contextuales y produce asociaciones
   efímeras nodo → símbolo/tipo/ámbito.
5. Si el resultado semántico es válido, `TacGenerator` realiza una segunda
   visita y emite cuádruplos, etiquetas, temporales y llamadas.
6. `TacGenerator` complementa la tabla con tamaños, offsets, almacenamiento y
   etiquetas, y calcula los registros de activación.
7. Si el TAC es válido, `MipsGenerator` traduce los cuádruplos y la información
   de símbolos a ensamblador MIPS32. Esta fase determina la distribución física
   de memoria y las secuencias de llamada y retorno.
8. Los modelos semántico, TAC y MIPS se serializan con relaciones por ID
   estables. El mismo ensamblador se usa para descargar, ejecutar y construir
   los artefactos de código máquina.

El análisis semántico no se ejecuta cuando existen errores léxicos o
sintácticos. Esto evita diagnósticos en cascada sobre un árbol recuperado e
incompleto, pero el árbol y los tokens se conservan para ayudar a corregir la
entrada.

De forma equivalente, el TAC no se ejecuta cuando existen errores de cualquiera
de las fases anteriores. `TacResult.skipped` y `reason` explican esta decisión al
CLI y al IDE.

El backend MIPS conserva la misma regla: no se ejecuta si una fase anterior
falla. `MipsResult.skipped`, `reason` y sus diagnósticos distinguen un error de
generación de una omisión provocada por una entrada inválida. La ejecución del
ensamblador pertenece a otra operación: compilar correctamente no equivale a
haber ejecutado correctamente el programa.

## Separación entre semántica y TAC

El código intermedio no se genera dentro de `semantic_analyzer.cpp`. Ambas fases
usan Visitors porque recorren la misma gramática, pero tienen contratos
distintos:

- `SemanticAnalyzer` produce `SemanticResult` y `SemanticBindings`;
- `TacGenerator` consume ambos, actualiza el layout de símbolos y produce
  `TacResult`;
- `MipsGenerator` consume `SemanticResult` y `TacResult` y produce `MipsResult`;
- `service.cpp` controla el orden y evita ejecutar una fase cuando su
  precondición falla.

`SemanticBindings` usa punteros al parse tree únicamente durante
`analyzeSource`. No se serializa ni escapa del tiempo de vida del parser. Esto
permite distinguir correctamente símbolos sombreados sin guardar punteros
inválidos en el resultado público.

## Tabla de símbolos

Cada `Scope` contiene:

- ID estable;
- clase de entorno (`global`, `class`, `function` o `block`);
- referencia al padre;
- símbolo propietario para clases y funciones;
- mapa local nombre → símbolo;
- lista de ámbitos hijos.

La resolución comienza en el ámbito actual y recorre los padres hasta el
global. Por eso una declaración interna oculta una externa, mientras que una
redeclaración dentro del mismo mapa se reporta como error. El modelo coincide
con la tabla por ámbito y la regla del entorno más cercano descritas en la
sección 2.7 del libro del dragón.

Los scopes no se destruyen al salir de un bloque: el Visitor solo restaura el ID
del padre. Así, el resultado final conserva una instantánea navegable de todos
los entornos requeridos por la evaluación.

## Tipos

Los tipos se representan mediante expresiones nominales:

- primitivos: `integer`, `float`, `string`, `boolean`, `null` y `void`;
- listas: aplicación repetida del constructor `[]`, como `integer[][]`;
- clases: nombre de la declaración;
- auxiliares internos: `function`, `class`, `unknown` y `error`.

El Visitor implementa síntesis de tipos: obtiene el tipo de una expresión a
partir de sus subexpresiones. `integer` puede ampliarse implícitamente a
`float`; una instancia derivada es asignable a una referencia de su clase base;
y `null` es compatible con referencias. Los tipos `unknown` y `error` impiden
duplicar mensajes cuando un error anterior ya quitó información al análisis.

## Declaraciones en dos momentos

Las clases y funciones se registran antes de analizar sus cuerpos. Esta pasada
de recolección permite:

- llamadas recursivas;
- llamadas a funciones declaradas más adelante en el mismo ámbito;
- referencias a clases declaradas posteriormente;
- resolución de miembros heredados sin depender del orden textual.

Las variables normales se registran al visitar su declaración, por lo que no
pueden usarse antes de declararse. Los miembros de clase se recolectan junto con
la clase para permitir acceso entre métodos.

## Funciones y closures

El símbolo de función almacena tipos y nombres de parámetros, retorno y scope
propio. Cada parámetro se declara dentro del scope de función y el bloque del
cuerpo crea, además, su entorno de bloque.

Cuando una función anidada resuelve una variable o parámetro declarado fuera
de su scope de función, se guardan dos relaciones:

- `captures` en la función;
- `captured_by` en el símbolo capturado.

Esto representa estáticamente el entorno de definición que el backend MIPS
convierte en un enlace de acceso en tiempo de ejecución. El TAC emite
`capture` en la función anidada, `static_link` en la llamada y usa direcciones
`env[...]` para los símbolos capturados, como se discute en la sección 7.3 del
libro del dragón.

## Código intermedio y temporales

Las instrucciones se guardan como cuádruplos estructurados, no como cadenas
sueltas. Una serialización textual facilita la evaluación humana, mientras que
`op`, `arg1`, `arg2` y `result` permiten pruebas precisas y la generación de
instrucciones MIPS.

El backend consume estos campos estructurados directamente. La presentación
textual TAC se conserva para inspección, pero no se vuelve a analizar como si
fuera otro lenguaje fuente.

Cada rutina tiene su propio pool de temporales. Los operandos se liberan después
de emitir la instrucción que los consume; el nombre vuelve a una lista libre y
puede utilizarse en otra expresión. El máximo de temporales simultáneamente
vivos determina los bytes de temporales del frame.

El diseño completo del lenguaje, catálogo de instrucciones, ejemplos y
supuestos está en [codigo-intermedio-tac.md](codigo-intermedio-tac.md).

## Layout y registros de activación

La tabla de símbolos se complementa después del análisis semántico:

- globales: offset positivo en el segmento global abstracto;
- parámetros: offsets desde `FP + 32`;
- locales: offsets negativos respecto de `FP`;
- campos: offsets dentro del objeto, después de los campos heredados;
- funciones y clases: etiquetas simbólicas.

Cada `ActivationRecord` reserva enlaces de control y acceso, dirección y valor
de retorno, parámetros, locales y temporales. El modelo conserva además la
profundidad léxica y las capturas requeridas por closures.

Ese registro es un diseño abstracto de la fase TAC. `MipsFrame` describe el
frame físico que utiliza el ensamblador: tamaño efectivo, función y padre
léxico, espacio de parámetros, locales, temporales y registros preservados.
Ambos modelos se muestran por separado para que el tamaño abstracto no se
confunda con una dirección de memoria ejecutable.

## Backend MIPS y entorno de ejecución

`mips_generator.cpp` organiza el TAC por rutinas antes de emitir instrucciones.
Así una declaración de función, incluida una función anidada, no introduce una
caída accidental del flujo principal dentro del cuerpo de la rutina.

Los valores que sobreviven a operaciones y llamadas cuentan con almacenamiento
en memoria. El asignador selecciona registros físicos y conserva el valor en
su ubicación de respaldo al reutilizarlos. Los límites de bloque y las llamadas
mantienen coherente ese almacenamiento, incluso cuando distintos caminos
convergen en una misma etiqueta.

Los enlaces de control identifican al llamante; los enlaces de acceso
identifican al entorno de definición. Para una llamada recursiva o entre
funciones hermanas, el enlace de acceso se obtiene del padre léxico correcto.
Una función anidada dentro de un método continúa siendo una función y accede a
`this` a través de su entorno; únicamente los métodos reciben un receptor como
argumento implícito.

El código generado incluye las rutinas auxiliares que necesita el programa.
La impresión y la reserva de memoria usan los servicios del simulador MARS.
La representación de valores, la distribución de frames, la asignación de
registros y los errores de ejecución están descritos en
[generacion-mips.md](generacion-mips.md).

## Compilación, ensamblado y ejecución

El núcleo C++ genera MIPS; MARS ensambla ese texto a instrucciones máquina y
simula su ejecución. Python coordina procesos y transporta resultados hacia el
IDE, sin interpretar expresiones Compiscript ni sustituir la ejecución MIPS.

Las pruebas de ejecución recorren todo el flujo `.cps → TAC → MIPS → MARS` y
comparan la salida con un resultado esperado independiente. Se mantienen
además las pruebas de semántica y TAC, de modo que una regresión puede ubicarse
en la fase que la produjo.

## Clases

Cada clase tiene un scope independiente con campos y métodos. La búsqueda de
miembros avanza por la cadena de herencia simple y evita ciclos. El análisis
valida:

- clase base existente y ausencia de ciclos;
- acceso a campos y métodos;
- uso de `this` dentro de una clase;
- firma del método `constructor` al ejecutar `new`;
- compatibilidad de subtipo en asignaciones, argumentos y retornos.

## Control de flujo

Los contextos mantienen profundidad de bucle y pila de funciones. De esa forma
`break`, `continue` y `return` se validan sin depender del texto de la entrada.
Una inspección estructural determina si una sentencia termina el flujo; toda
sentencia posterior en la misma secuencia recibe `SEM_FLOW_DEAD_CODE`.

## Contrato JSON

La salida contiene:

- `syntax_ok`, `semantic_ok` y `ok`;
- `tac_ok`;
- `mips_ok`;
- `tokens`;
- `tree`;
- `syntax_diagnostics` y `semantic.diagnostics`;
- `semantic.scopes`, `semantic.symbols` y `semantic.classes`;
- `tac.instructions`, `tac.text` y `tac.activation_records`;
- `mips.text`, `mips.diagnostics`, `mips.frames` y métricas de generación y
  asignación de registros;
- métricas de creación, reutilización y máximo simultáneo de temporales;
- `summary` con conteos para el IDE.

Todos los diagnósticos poseen código estable, categoría, severidad, mensaje,
línea y columna. El IDE usa esta ubicación para mover el cursor al error.
