# Arquitectura del frontend de Compiscript

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
   de cada expresión y aplica reglas contextuales.
5. El modelo semántico se serializa con relaciones por ID estables.

El análisis semántico no se ejecuta cuando existen errores léxicos o
sintácticos. Esto evita diagnósticos en cascada sobre un árbol recuperado e
incompleto, pero el árbol y los tokens se conservan para ayudar a corregir la
entrada.

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
necesitaría convertir en un enlace de acceso o closure en tiempo de ejecución,
como se discute en la sección 7.3 del libro del dragón.

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
- `tokens`;
- `tree`;
- `syntax_diagnostics` y `semantic.diagnostics`;
- `semantic.scopes`, `semantic.symbols` y `semantic.classes`;
- `summary` con conteos para el IDE.

Todos los diagnósticos poseen código estable, categoría, severidad, mensaje,
línea y columna. El IDE usa esta ubicación para mover el cursor al error.
