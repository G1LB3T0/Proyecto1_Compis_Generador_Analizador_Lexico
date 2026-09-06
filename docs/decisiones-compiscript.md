# Decisiones de compatibilidad de Compiscript

Este documento distingue tres fuentes: el enunciado define qué se evalúa, la
especificación describe el lenguaje esperado y `Compiscript (1).g4` es la base
sintáctica entregada. El libro del dragón es referencia conceptual, no una
fuente de requisitos.

## Extensiones de la gramática base

### `float`

El sistema de tipos del enunciado exige operaciones entre `integer` y `float`,
pero la gramática solo contiene enteros. La gramática activa agrega:

- tipo base `float`;
- literal decimal formado por dígitos, punto y dígitos;
- ampliación implícita `integer → float`.

### Cuerpos sin llaves

La gramática base requiere bloques en `if`, `while`, `do-while`, `for` y
`foreach`, mientras que la especificación muestra `if (n <= 1) return 1;`.
`controlBody` acepta un bloque o una sentencia. Con esto los programas escritos
según cualquiera de las dos fuentes siguen siendo válidos.

### `void`

Una función sin anotación de retorno se trata como `void`. También se acepta
`void` de forma explícita en una firma. El Visitor rechaza variables,
parámetros, constantes y listas de `void`.

## Reglas semánticas reconciliadas

### Operador `+`

Además de suma numérica, se permite `string + string`, porque los ejemplos de
funciones, métodos y `catch` usan concatenación. No se permite concatenar de
forma implícita cualquier objeto con un string.

### `switch`

El enunciado dice que la condición de `switch` debe ser `boolean`, pero la
especificación usa `switch (x)` con casos enteros. Se priorizó la regla explícita
de evaluación: selector booleano y casos compatibles con él. Si el profesor
confirma que el ejemplo entero tiene prioridad, el cambio se limita a retirar
una llamada a `requireType`; la comprobación de compatibilidad de los casos ya
está separada.

### `break` en `switch`

El enunciado limita `break` y `continue` a bucles. Por eso `break` dentro de un
`switch` sin bucle también se reporta. Esta decisión sigue literalmente la
rúbrica, aunque otros lenguajes suelen permitir `break` en ambos contextos.

## Elementos heredados de Compiladores 1

Se reutilizaron:

- Flask y la separación navegador/CLI;
- el patrón de salida JSON;
- la experiencia de visualización de árboles;
- la organización de ejemplos y pruebas.

No se reutilizan en el pipeline activo:

- YALex;
- YAPar;
- FIRST/FOLLOW;
- tablas LL(1), SLR o LALR;
- el modelo semántico experimental ligado a gramáticas `.yalp`.

Estos archivos permanecen como historial y respaldo académico, pero el IDE de
la rama `compiladores-2` solo ofrece Compiscript con ANTLR.
