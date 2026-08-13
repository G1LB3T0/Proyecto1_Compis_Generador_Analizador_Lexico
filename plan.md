# Plan de avance: analizador semántico

## Estado del primer avance (implementado)

El primer incremento de esta hoja de ruta ya está implementado para **SLR y LALR**:

- CST construido por el parser y AST mínimo compartido.
- Tipos primitivos, tipos de clase y firmas de métodos.
- Ámbitos global, de clase, método y bloque.
- Símbolos de clases, atributos, métodos, parámetros y variables.
- Tabla de clases con atributos y métodos.
- Diagnósticos `SEM002`, `SEM003` y `SEM010`.
- Ejemplos, JSON, pruebas unitarias y comparación SLR/LALR.

Siguen pendientes, de forma intencional, las expresiones, resolución de usos, comprobación completa de tipos, normalización LL(1) y visualización semántica especializada. El resto del documento conserva la ruta completa para esos avances posteriores.

## 1. Objetivo

Agregar al proyecto una fase de análisis semántico capaz de construir y mostrar, de forma determinista:

1. Los **tipos** del programa y el tipo resultante de las expresiones.
2. Los **ámbitos** y los símbolos declarados en cada uno.
3. Las **clases**, junto con sus atributos y métodos.
4. Los **errores semánticos**, con línea, columna, código y una explicación útil.

El analizador semántico se ejecutará únicamente después de que el análisis léxico y sintáctico haya producido una estructura válida. No generará código todavía.

La salida esperada del front-end será:

```text
caracteres -> tokens -> árbol sintáctico/AST -> modelo semántico validado
                                              |- tipos
                                              |- ámbitos y símbolos
                                              |- clases
                                              `- diagnósticos
```

## 2. Fundamento teórico que guiará la implementación

### 2.1 Fases del compilador

La presentación **01 - Compiladores v 1.2**, diapositiva 7, coloca el análisis semántico después del análisis sintáctico y antes de la representación intermedia. También muestra que la tabla de símbolos participa en esta fase. Por eso no se intentará deducir tipos directamente desde el texto ni desde la tabla LR: se trabajará sobre un árbol construido por el parser.

### 2.2 Traducción dirigida por la sintaxis

La presentación **02 - Traducción orientada por la sintaxis v 1.0** establece que las producciones se relacionan con reglas semánticas mediante atributos:

- Atributos sintetizados: suben información desde los hijos, por ejemplo `expr.type`.
- Atributos heredados: llevan contexto hacia los hijos, por ejemplo `node.scope`, `expectedReturnType` o `currentClass`.
- El grafo de dependencias no debe tener ciclos.
- Los efectos adicionales sobre tablas deben estar controlados.

En este proyecto, la traducción dirigida por la sintaxis se utilizará para **construir el AST**. La creación de ámbitos, inserción de símbolos y comprobación de tipos se hará después, en pasadas separadas. Esta decisión evita que el resultado dependa del orden particular de reducciones de SLR, LALR o LL(1), y reduce los efectos laterales durante el parsing.

### 2.3 Sistema de tipos

Cardelli define un sistema de tipos como el componente que sigue los tipos de variables y expresiones, y distingue programas bien tipados de programas mal tipados. También formaliza las comprobaciones con juicios y entornos. En nuestra implementación:

- El entorno `Γ` será la cadena de ámbitos visible desde un nodo.
- `Γ(x) = T` significará que la búsqueda de `x` encontró un símbolo de tipo `T`.
- `Γ ⊢ e : T` significará que la expresión `e` tiene tipo `T`.
- Una incompatibilidad producirá un diagnóstico semántico y el tipo especial `Error`, para poder continuar sin generar decenas de errores derivados del primero.

Ejemplos de reglas que se implementarán:

```text
Γ(x) = T
----------
Γ ⊢ x : T

Γ ⊢ e : Te       Te es asignable a Tx
--------------------------------------
Γ ⊢ x:Tx = e : válido

Γ ⊢ e : C        atributo(C, m) = T
-------------------------------------
Γ ⊢ e.m : T
```

El propósito práctico coincide con Cardelli: rechazar antes de la ejecución construcciones que puedan aplicar operaciones a valores inadecuados.

### 2.4 Dragon Book

El orden de trabajo sigue los temas del Dragon Book, segunda edición:

- 2.7: tablas de símbolos.
- Capítulo 5: definiciones y esquemas dirigidos por la sintaxis.
- 6.1: variantes de árboles sintácticos.
- 6.3: tipos y declaraciones.
- 6.5: comprobación de tipos.
- Apéndice A.4: tablas de símbolos y tipos dentro de un front-end completo.

Referencia oficial del contenido: <https://www.pearson.com/en-us/subject-catalog/p/compilers-principles-techniques-and-tools/P200000003472/9780133002140>

## 3. Estado actual y brecha que hay que cerrar

El proyecto ya tiene:

- Tokens con tipo, lexema, línea y columna.
- Gramática y producciones.
- Parsers SLR(1), LALR(1) y LL(1).
- Traza, reducciones o derivaciones y recuperación básica de errores.
- Salida JSON y visualización web del pipeline.

El proyecto todavía no tiene:

- Árbol sintáctico concreto ni AST.
- Valores semánticos asociados con los símbolos de la pila.
- Posiciones completas de origen para nodos compuestos.
- Tabla de símbolos jerárquica.
- Representación canónica de tipos.
- Modelo de clases.
- Reglas de resolución de nombres o comprobación de tipos.

La brecha crítica es el AST. Una lista de reducciones indica qué producciones se usaron, pero no conserva de forma suficiente la relación entre identificadores, declaraciones, expresiones y bloques. Por eso el AST será el primer entregable técnico.

## 4. Contrato del lenguaje para el primer avance

Antes de programar se congelará un subconjunto pequeño y comprobable del lenguaje. Los valores predeterminados del plan serán:

### Construcciones incluidas

- Tipos primitivos: `int`, `float`, `double`, `char`, `bool`, `string` y `void`.
- Declaraciones de variables y parámetros.
- Bloques anidados.
- Funciones o métodos con parámetros y retorno.
- Clases con atributos y métodos.
- Uso de identificadores.
- Asignaciones.
- Expresiones aritméticas, relacionales y lógicas.
- Acceso a miembros y llamadas a métodos.
- Sentencia `return`.

### Reglas iniciales

- No se puede declarar dos veces el mismo nombre en el mismo ámbito.
- Sí se permite ocultamiento en un ámbito hijo; la búsqueda toma la declaración visible más cercana.
- Toda referencia debe resolver a un símbolo declarado y visible.
- Una variable no puede tener tipo `void`.
- Una clase debe existir antes de utilizarse como tipo; se permitirán referencias adelantadas porque las clases se predeclaran en una primera pasada.
- Un miembro debe existir en la clase del receptor.
- `return` debe ser compatible con el tipo declarado del método o función.
- Conversión numérica implícita únicamente en ampliación: `char -> int -> float -> double`.
- No habrá conversión implícita de `bool` a número.
- Una asignación entre clases exige el mismo tipo de clase en este primer avance.

### Fuera del primer avance

- Herencia.
- Sobrecarga de métodos u operadores.
- Constructores y destructores.
- Modificadores `public`, `private` y `protected`.
- Genéricos, polimorfismo paramétrico e inferencia avanzada.
- Arreglos, punteros y referencias.
- Generación de código intermedio.

Estas características solo se agregarán si la rúbrica las exige. Mantenerlas fuera evita inventar reglas incompatibles entre sí antes de tener funcionando el núcleo.

## 5. Arquitectura propuesta

### 5.1 Árbol sintáctico y AST

Se incorporarán dos niveles conceptuales:

1. **ParseTreeNode**: representación genérica de una producción, sus hijos y tokens. Puede construirla cualquier método de parsing.
2. **ASTNode**: representación reducida orientada al significado del programa; elimina puntuación y no terminales auxiliares.

Cada nodo tendrá un `SourceSpan` con inicio y final. Así todos los diagnósticos podrán señalar una ubicación reproducible.

Nodos mínimos del AST:

```text
Program
ClassDecl
FieldDecl
MethodDecl / FunctionDecl
ParameterDecl
VarDecl
BlockStmt
ReturnStmt
AssignmentExpr
BinaryExpr / UnaryExpr
CallExpr
MemberExpr
IdentifierExpr
LiteralExpr
```

### 5.2 Tipos canónicos

No se compararán tipos usando cadenas dispersas. Existirá una sola tabla de tipos y cada tipo tendrá un `TypeId` estable.

```text
Type
|- id
|- kind: Primitive | Class | Function | Void | Error | Unknown
|- name
|- classId, si corresponde
|- parameterTypes, si es función o método
`- returnType, si es función o método
```

`Unknown` se usará mientras una referencia todavía no se resuelve. `Error` se usará después de detectar un fallo para impedir diagnósticos en cascada.

### 5.3 Símbolos y ámbitos

Todos los nombres declarables compartirán el mismo modelo base:

```text
Symbol
|- SymbolId
|- name
|- kind: Class | Field | Method | Function | Parameter | Variable
|- TypeId
|- ScopeId donde fue declarado
`- SourceSpan
```

Los ámbitos formarán un árbol:

```text
Global
`- Clase
   `- Método
      `- Bloque
         `- Bloque anidado
```

Cada `Scope` tendrá `ScopeId`, clase de ámbito, padre, propietario y un mapa `nombre -> SymbolId`. Habrá únicamente dos operaciones públicas de resolución:

- `declare`: inserta y detecta duplicados locales.
- `lookup`: busca desde el ámbito actual hacia sus padres.

Esto centraliza las reglas de visibilidad y evita que cada visitante implemente su propia versión.

### 5.4 Modelo de clases

Cada clase tendrá:

```text
ClassInfo
|- ClassId
|- símbolo de la clase
|- TypeId de la clase
|- ScopeId de miembros
|- atributos: nombre -> SymbolId
`- métodos: nombre -> SymbolId
```

En la primera versión no se permitirán miembros duplicados y no habrá sobrecarga; por tanto, un nombre de método identifica una sola firma dentro de su clase.

### 5.5 Diagnósticos

Todos los errores tendrán el mismo formato:

```text
SemanticDiagnostic
|- code, por ejemplo SEM001
|- severity: error | warning
|- message
|- SourceSpan principal
`- ubicación relacionada opcional
```

Códigos iniciales:

- `SEM001`: símbolo no declarado.
- `SEM002`: declaración duplicada en el mismo ámbito.
- `SEM003`: tipo desconocido.
- `SEM004`: tipos incompatibles en asignación.
- `SEM005`: operador no válido para los operandos.
- `SEM006`: miembro inexistente.
- `SEM007`: cantidad o tipos de argumentos incorrectos.
- `SEM008`: retorno incompatible.
- `SEM009`: uso inválido de `void`.
- `SEM010`: clase duplicada.

Los diagnósticos se ordenarán por línea, columna y código para que las pruebas y la interfaz siempre obtengan la misma salida.

## 6. Pasadas del analizador semántico

No se hará todo en un único recorrido. Se utilizarán pasadas explícitas para resolver dependencias sin depender del orden textual.

### Pasada 0: construcción y normalización del AST

- Construir nodos terminales al hacer `shift` o `match`.
- Construir nodos de producción al hacer `reduce` o aplicar una producción LL(1).
- Convertir el árbol de parsing a un AST canónico.
- Conservar lexemas y posiciones.
- Validar que SLR y LALR produzcan el mismo AST para la misma entrada.

### Pasada 1: predeclaración

- Crear el ámbito global.
- Registrar tipos primitivos.
- Registrar primero todas las clases como nombres de tipo incompletos.
- Registrar firmas de funciones y métodos.

Esta pasada permite usar una clase declarada más adelante sin confundirla con un tipo inexistente.

### Pasada 2: construcción de ámbitos y símbolos

- Crear ámbitos de clase, método/función y bloque.
- Insertar atributos, parámetros y variables.
- Asociar cada declaración y referencia del AST con un `ScopeId`.
- Detectar declaraciones duplicadas.
- Construir las tablas de clases.

### Pasada 3: resolución de nombres y tipos declarados

- Resolver cada `IdentifierExpr` con `lookup`.
- Resolver nombres de tipos primitivos y clases.
- Vincular accesos a miembros con `ClassInfo`.
- Marcar nombres no resueltos con `ErrorType`.

### Pasada 4: comprobación de tipos

- Sintetizar el tipo de literales, identificadores, operaciones, asignaciones, miembros y llamadas.
- Comprobar operadores mediante una matriz central de compatibilidad.
- Comprobar argumentos y retornos.
- Aplicar únicamente las conversiones implícitas definidas en el contrato.
- Anotar cada expresión con su `TypeId` resultante.

### Pasada 5: reporte

- Serializar tipos, ámbitos, símbolos, clases y diagnósticos.
- Mostrar el modelo incluso cuando existan errores recuperables.
- Marcar `semantic.ok = false` si existe al menos un diagnóstico de severidad `error`.

## 7. Integración con los parsers actuales

### SLR y LALR

Se agregará una pila de nodos paralela a las pilas de estados y símbolos:

- `shift`: crea un nodo terminal con el token completo.
- `reduce A -> β`: extrae los nodos de `β`, conserva su orden y crea el nodo de `A`.
- `accept`: devuelve la raíz del árbol junto con la traza actual.

SLR y LALR usan la gramática original aumentada, por lo que deben producir el mismo árbol canónico.

### LL(1)

Actualmente LL(1) transforma la gramática eliminando recursión izquierda y factorizando. Los no terminales auxiliares pueden cambiar la forma del árbol. Para evitar resultados semánticos inconsistentes:

1. Se conservará la relación entre producciones originales y producciones transformadas.
2. El árbol LL(1) pasará por una normalización que elimine los no terminales generados.
3. El análisis semántico con LL(1) no se declarará terminado hasta que pruebas doradas demuestren que su AST y su JSON semántico son equivalentes a SLR/LALR.

No se copiarán reglas semánticas tres veces. Los tres métodos entregarán el mismo AST al mismo `SemanticAnalyzer`.

## 8. Archivos previstos

### Archivos nuevos

```text
backend/src/source_span.h
backend/src/parse_tree.h
backend/src/parse_tree.cpp
backend/src/ast.h
backend/src/ast.cpp
backend/src/semantic_types.h
backend/src/semantic_types.cpp
backend/src/symbol_table.h
backend/src/symbol_table.cpp
backend/src/semantic_model.h
backend/src/semantic_analyzer.h
backend/src/semantic_analyzer.cpp
backend/src/semantic_json.h
backend/src/semantic_json.cpp
backend/examples/semantica.yal
backend/examples/semantica.yalp
backend/examples/semantic_valid.txt
backend/examples/semantic_errors.txt
backend/tests/semantic_tests.cpp
```

### Archivos que se modificarán

```text
backend/src/token_stream.h       - posiciones completas si fueran necesarias
backend/src/grammar.h            - identidad/origen de producciones
backend/src/lr_parser.h/.cpp     - construcción del árbol LR
backend/src/ll1_parser.h/.cpp    - construcción del árbol LL(1)
backend/src/grammar_transform.*  - mapa de producciones transformadas
backend/src/yapar_cli.cpp        - ejecutar y serializar la fase semántica
app.py                           - exponer la nueva salida sin romper la API
frontend/static/js/app.js        - paneles de tipos, ámbitos, clases y errores
frontend/static/css/style.css    - estilos de los nuevos paneles
```

## 9. Contrato JSON propuesto

Se conservarán todas las claves actuales y se agregará una clave `semantic`:

```json
{
  "semantic": {
    "ok": true,
    "skipped": false,
    "types": [],
    "scopes": [],
    "symbols": [],
    "classes": [],
    "diagnostics": []
  }
}
```

Si el análisis sintáctico falla, la fase semántica no se ejecutará:

```json
{
  "semantic": {
    "ok": false,
    "skipped": true,
    "reason": "El programa contiene errores sintácticos"
  }
}
```

La adición será compatible hacia atrás: la interfaz existente podrá seguir leyendo `dfa`, `grammar`, `ff`, `lr0`, `table`, `tokens` y `result`.

## 10. Orden de implementación por avances

### Avance A: contrato y árbol

- Crear la gramática del subconjunto semántico.
- Agregar `SourceSpan`, árbol de parsing y AST.
- Integrar la construcción del árbol en SLR y LALR.
- Probar igualdad estructural entre ambos métodos.

**Criterio de terminado:** una entrada válida produce un AST con identificadores, literales, declaraciones, clases y bloques, conservando línea y columna.

### Avance B: ámbitos, símbolos y clases

- Crear tabla de tipos primitivos.
- Implementar `Scope`, `Symbol` y `ClassInfo`.
- Ejecutar las pasadas de predeclaración y construcción de ámbitos.
- Detectar símbolos, campos y clases duplicadas.
- Serializar las tres tablas.

**Criterio de terminado:** la salida muestra un árbol de ámbitos coherente y cada símbolo tiene un único propietario e identificador.

### Avance C: comprobación de tipos

- Implementar resolución de nombres.
- Implementar matriz de operadores y asignabilidad.
- Comprobar miembros, llamadas y retornos.
- Añadir `ErrorType` y diagnósticos con recuperación.

**Criterio de terminado:** programas válidos no generan errores y cada caso inválido genera el código esperado sin cascadas innecesarias.

### Avance D: LL(1), API e interfaz

- Normalizar el árbol de la gramática transformada para LL(1).
- Probar equivalencia con SLR/LALR.
- Agregar `semantic` al JSON.
- Mostrar tipos, árbol de ámbitos, tabla de clases y diagnósticos en la interfaz.

**Criterio de terminado:** la misma entrada produce el mismo modelo semántico, sin importar el método de parsing seleccionado.

### Avance E: endurecimiento y documentación

- Ejecutar regresión del pipeline léxico/sintáctico existente.
- Añadir pruebas de errores y ubicaciones.
- Documentar reglas soportadas y limitaciones.
- Revisar memoria, ciclos de referencias y salida JSON válida.

**Criterio de terminado:** todas las pruebas son repetibles y las funcionalidades existentes siguen funcionando.

## 11. Estrategia de pruebas

### Pruebas válidas

- Variables primitivas y expresiones compatibles.
- Bloques anidados y búsqueda en el padre.
- Ocultamiento permitido en un bloque hijo.
- Clase con atributos y métodos.
- Referencia adelantada a una clase.
- Acceso a un atributo válido.
- Llamada con argumentos correctos.
- Retorno compatible.
- Promoción numérica válida.

### Pruebas inválidas

- Uso de variable no declarada.
- Duplicado en el mismo ámbito.
- Tipo de clase inexistente.
- Variable de tipo `void`.
- Asignación incompatible.
- Operación aritmética con `bool` o clase.
- Miembro inexistente.
- Llamada con cantidad o tipos incorrectos.
- Retorno incompatible o retorno con valor en método `void`.
- Clase, atributo o método duplicado.

### Pruebas de consistencia

- SLR y LALR producen el mismo AST normalizado.
- SLR, LALR y LL(1) producen el mismo JSON semántico, ignorando identificadores internos que no sean parte del contrato.
- Dos ejecuciones idénticas producen diagnósticos en el mismo orden.
- Cada `SymbolId`, `ScopeId`, `TypeId` y `ClassId` referencia una entidad existente.
- Cada ámbito, excepto el global, tiene exactamente un padre.
- Cada símbolo pertenece a exactamente un ámbito.
- Cada tipo de clase referencia exactamente una clase.

### Regresión

- Los ejemplos léxicos actuales deben seguir generando el mismo DFA y tokens.
- Las tablas FIRST/FOLLOW, LR(0), SLR, LALR y LL(1) no deben cambiar salvo cuando se use la nueva gramática de ejemplo.
- La API existente no debe perder campos ni cambiar su significado.

## 12. Reglas para evitar errores e inconsistencias durante el desarrollo

1. **Una fuente de verdad:** tipos, ámbitos y símbolos se crean en repositorios centrales; no se duplican en visitantes ni en la interfaz.
2. **Identificadores estables:** las referencias internas usan IDs, no copias de nombres o punteros serializados.
3. **Pasadas separadas:** declarar, resolver y comprobar tipos son operaciones distintas.
4. **Sin efectos semánticos irreversibles en el parser:** el parser construye el árbol; el analizador modifica el modelo semántico.
5. **Tipos canónicos:** la igualdad y asignabilidad pasan por `TypeSystem`, nunca por comparaciones ad hoc de strings.
6. **Recuperación controlada:** `ErrorType` evita repetir el mismo fallo en nodos padres.
7. **Ubicación obligatoria:** todo símbolo, nodo y diagnóstico conserva `SourceSpan`.
8. **Resultados deterministas:** ordenar las colecciones antes de serializarlas.
9. **Compatibilidad de métodos:** ninguna regla semántica conoce si el árbol vino de SLR, LALR o LL(1).
10. **No ejecutar semántica tras error sintáctico:** evita interpretar árboles incompletos como programas válidos.
11. **Pruebas antes de interfaz:** primero se valida el modelo C++ y su JSON; después se visualiza.
12. **No ampliar el lenguaje silenciosamente:** toda nueva conversión, miembro especial o regla de visibilidad se documenta y prueba antes de activarse.

## 13. Definición de terminado del avance semántico

El avance completo estará terminado cuando:

- Un programa válido produce AST, tipos, ámbitos, símbolos y clases sin diagnósticos de error.
- Un programa inválido reporta todos los errores semánticos previstos con línea y columna correctas.
- No hay errores en cascada evitables.
- Las referencias de identificadores y miembros apuntan a símbolos existentes.
- La tabla de clases concuerda con los ámbitos de clase.
- Las reglas de tipos están centralizadas y cubiertas por pruebas.
- SLR y LALR son equivalentes; LL(1) también lo es después de normalizar su gramática transformada.
- El pipeline léxico y sintáctico anterior continúa funcionando.
- La interfaz muestra la salida semántica sin alterar el contrato anterior.

## 14. Primera acción cuando se autorice la implementación

No se comenzará por la interfaz. La primera implementación será un programa mínimo de prueba con una clase, un método, parámetros, un bloque y expresiones. Sobre esa entrada se construirá primero el AST y se validará su estructura con SLR y LALR. Solo después se implementarán las tablas y las reglas de tipos.
