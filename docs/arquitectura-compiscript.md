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
7. Los modelos semántico y TAC se serializan con relaciones por ID estables.

El análisis semántico no se ejecuta cuando existen errores léxicos o
sintácticos. Esto evita diagnósticos en cascada sobre un árbol recuperado e
incompleto, pero el árbol y los tokens se conservan para ayudar a corregir la
entrada.

De forma equivalente, el TAC no se ejecuta cuando existen errores de cualquiera
de las fases anteriores. `TacResult.skipped` y `reason` explican esta decisión al
CLI y al IDE.

## Separación entre semántica y TAC

El código intermedio no se genera dentro de `semantic_analyzer.cpp`. Ambas fases
usan Visitors porque recorren la misma gramática, pero tienen contratos
distintos:

- `SemanticAnalyzer` produce `SemanticResult` y `SemanticBindings`;
- `TacGenerator` consume ambos, actualiza el layout de símbolos y produce
  `TacResult`;
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

Esto representa estáticamente el entorno de definición que una fase posterior
convierte en un enlace de acceso o closure en tiempo de ejecución. El TAC emite
`capture` en la función anidada, `static_link` en la llamada y usa direcciones
`env[...]` para los símbolos capturados, como se discute en la sección 7.3 del
libro del dragón.

## Código intermedio y temporales

Las instrucciones se guardan como cuádruplos estructurados, no como cadenas
sueltas. Una serialización textual facilita la evaluación humana, mientras que
`op`, `arg1`, `arg2` y `result` permiten pruebas precisas y una futura fase de
código objeto.

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
- `tokens`;
- `tree`;
- `syntax_diagnostics` y `semantic.diagnostics`;
- `semantic.scopes`, `semantic.symbols` y `semantic.classes`;
- `tac.instructions`, `tac.text` y `tac.activation_records`;
- métricas de creación, reutilización y máximo simultáneo de temporales;
- `summary` con conteos para el IDE.

Todos los diagnósticos poseen código estable, categoría, severidad, mensaje,
línea y columna. El IDE usa esta ubicación para mover el cursor al error.
