# Proyecto 3: intérprete SQL con almacenamiento en archivos

Esta es la guía para defender el proyecto. Explica **qué se reutilizó del
Proyecto 2**, **qué cambió**, **qué es nuevo**, **cómo se guardan los datos** y
**el recorrido completo** de una sentencia, desde el texto hasta el archivo
en disco.

Rama: `Compiladores2-Proyecto3`, creada a partir de `Compiladores2-Proyecto2`.

---

## 1. El proyecto en 30 segundos

El sistema recibe texto SQL y lo procesa con las mismas fases de un compilador:

```text
SQL ─► Lexer ─► Parser ─► AST ─► Análisis semántico ─► Intérprete ─► Archivos de texto ─► Resultado
      (ANTLR)  (ANTLR)   (nuevo)    (nuevo)             (nuevo)      (.txt/.csv/.idx)
```

- **ANTLR** genera el lexer y el parser desde una gramática nueva, `SQL.g4`.
- El **AST** guarda solo el significado de cada sentencia, sin paréntesis ni
  comas.
- La **semántica** valida tablas, columnas, tipos, restricciones e índices
  contra el catálogo **antes** de ejecutar.
- El **intérprete** recorre el AST y lee y escribe archivos `.csv` e `.idx`.
  **No se usa ningún DBMS** (ni SQLite, ni MySQL, ni PostgreSQL).
- El **IDE web** permite ver tokens, árbol sintáctico, AST, errores,
  resultados, plan de ejecución y los archivos reales de la base de datos.

---

## 2. Qué se reutilizó, qué se adaptó y qué es nuevo

| Pieza | Proyecto 2 (Compiscript) | Proyecto 3 (SQL) | Estado |
|---|---|---|---|
| Herramienta de lexer/parser | ANTLR 4.13.2, target C++ | Igual | **Reutilizado** |
| Gramática | `Compiscript.g4` | `SQL.g4` | **Nuevo** (cambia por completo) |
| Código generado por ANTLR | `CompiscriptLexer/Parser/Visitor` | `SQLLexer/Parser/Visitor` | **Regenerado** |
| Listener de errores léxicos/sintácticos | `diagnostics.cpp` | `sql/src/diagnostics.cpp` | **Reutilizado** (+ traducción al español) |
| Serializar el parse tree a JSON | `tree_serializer.cpp` | `sql/src/tree_serializer.cpp` | **Reutilizado sin cambios de lógica** |
| Escapar JSON (`quoteJson`) | `json.cpp` | `sql/src/json.cpp` | **Reutilizado**; el resto del archivo es nuevo |
| Categorías de tokens | `token_classifier.cpp` | `sql/src/token_classifier.cpp` | **Adaptado** (palabras de SQL) |
| Orquestador de fases | `service.cpp`: lexer→parser→semántica→TAC | `sql/src/service.cpp`: lexer→parser→AST→semántica→intérprete | **Adaptado** |
| CLI | `compiscript_cli --file/--stdin` | `sql_cli --db --analyze --catalog --reset` | **Adaptado** |
| AST | No existía (se trabajaba sobre el parse tree) | `ast.h`, `ast_builder.cpp`, `ast_serializer.cpp` | **Nuevo** |
| Semántica | Tipos, ámbitos y funciones de Compiscript | Catálogo, tablas, columnas, tipos SQL, restricciones | **Nuevo** (misma idea: Visitor + diagnósticos con código) |
| Generación de TAC | `tac_generator.cpp` | — (no aplica) | **No se usa** |
| Intérprete | — | `executor.cpp` | **Nuevo** |
| Almacenamiento | — | `catalog.cpp`, `storage.cpp`, `value.cpp` | **Nuevo** |
| Adaptador web | `app.py` llama a `compiscript_cli` | `app.py` llama a `sql_cli` | **Adaptado** (mismo patrón) |
| IDE | Editor, árbol, diagnósticos, tokens, TAC, ámbitos | Editor, árbol, **AST**, errores, tokens, **resultados**, **base de datos** | **Adaptado** |
| Visor gráfico de árboles | Una sola instancia para el parse tree | Generalizado: dibuja el parse tree y el AST | **Adaptado** |
| Batería de pruebas | `compiscript_tests.cpp`, reporte JSON | `sql_tests.cpp`, **mismo formato de reporte** | **Nuevo** (con el formato reutilizado) |
| CMake | Solo Compiscript | SQL por defecto; Compiscript con `-DBUILD_COMPISCRIPT=ON` | **Adaptado** |

**Frase para la defensa:** "Reutilizamos toda la infraestructura del
compilador: ANTLR, el manejo de errores, la serialización del árbol, el
servicio que encadena fases, el CLI, Flask y el IDE. Lo que cambia es el
lenguaje (la gramática) y lo que se hace después del parser: antes se
generaba código intermedio; ahora se construye un AST, se valida contra un
catálogo y se interpreta sobre archivos."

---

## 3. Árbol de archivos

Leyenda: 🆕 nuevo · ✏️ modificado · ♻️ reutilizado del Proyecto 2 (copiado y
adaptado de namespace) · ⚙️ generado por ANTLR · 📦 Proyecto 2 sin cambios

```text
Proyecto/
├── CMakeLists.txt                     ✏️ targets sql_core, sql_cli, sql_tests; Compiscript opcional
├── app.py                             ✏️ Flask: /api/run, /api/database, /api/database/reset, /api/tests
├── requirements.txt                   📦 Flask
├── README.md                          ✏️ ahora describe el Proyecto 3
├── .gitignore                         ✏️ ignora database/, sql_cli, sql_tests
│
├── backend/
│   ├── sql/                           🆕 MÓDULO DEL PROYECTO 3
│   │   ├── grammar/
│   │   │   └── SQL.g4                 🆕 gramática (lexer + parser) de SQL
│   │   ├── generated/                 ⚙️ SQLLexer, SQLParser, SQLVisitor, SQLBaseVisitor (.cpp/.h)
│   │   ├── include/sql/
│   │   │   ├── value.h                🆕 tipos SQL y valores
│   │   │   ├── model.h                🆕 diagnósticos, tokens y resultados (idea del model.h anterior)
│   │   │   ├── ast.h                  🆕 nodos del AST
│   │   │   ├── ast_builder.h          🆕 Visitor: parse tree → AST
│   │   │   ├── ast_serializer.h       🆕 AST → JSON para dibujarlo
│   │   │   ├── catalog.h              🆕 esquemas, restricciones e índices (metadata.txt)
│   │   │   ├── storage.h              🆕 lectura y escritura de .csv e .idx
│   │   │   ├── semantic_analyzer.h    🆕 reglas semánticas SQL
│   │   │   ├── executor.h             🆕 intérprete
│   │   │   ├── service.h              ♻️ orquestador (adaptado)
│   │   │   ├── diagnostics.h          ♻️ listener de errores de ANTLR
│   │   │   ├── json.h                 ♻️ quoteJson + salida SQL
│   │   │   ├── token_classifier.h     ♻️ categorías de tokens
│   │   │   └── tree_serializer.h      ♻️ parse tree → JSON
│   │   └── src/                       (una implementación .cpp por cada .h, más main.cpp)
│   │
│   ├── compiscript/                   📦 Proyecto 2 completo, sin cambios
│   ├── examples/
│   │   ├── sql/                       🆕 01…09 scripts de demostración, en orden de entregas
│   │   └── compiscript/               📦 ejemplos del Proyecto 2
│   └── tests/
│       ├── sql_tests.cpp              🆕 117 casos, incluidos fallos de disco y recuperación
│       └── compiscript_tests.cpp      📦 107 casos del Proyecto 2
│
├── frontend/
│   ├── templates/index.html           ✏️ pestañas: Resultados, Errores, Tokens, Árbol, AST, Base de datos
│   └── static/
│       ├── js/app.js                  ✏️ resultados, catálogo y visor de árboles generalizado
│       └── css/style.css              ✏️ estilos de resultados, plan y archivos
│
├── scripts/
│   ├── build_sql.ps1 / build_sql.sh   🆕 generan ANTLR, compilan y ejecutan las pruebas
│   ├── generate_antlr.sh              ✏️ parámetro sql (predeterminado) | compiscript
│   └── build_compiscript.*            ✏️ solo agregan -DBUILD_COMPISCRIPT=ON
│
├── docs/proyecto3-interprete-sql.md   🆕 este documento
│
└── database/                          (se crea al ejecutar; ignorada por git)
    ├── metadata.txt
    ├── <tabla>.csv
    └── indexes/<indice>.idx
```

---

## 4. Flujo completo con un ejemplo

Se sigue la consulta del enunciado:

```sql
SELECT nombre FROM estudiantes WHERE edad >= 18;
```

### 4.1 `app.py` → `sql_cli`

El botón **Ejecutar SQL** hace `POST /api/run`. Flask **no interpreta
nada**: ejecuta `sql_cli.exe --db database --stdin script.sql`, le pasa el
texto y devuelve al navegador el JSON que produce el CLI. Es el mismo patrón
del Proyecto 2.

### 4.2 Análisis léxico (`SQLLexer`, generado por ANTLR)

El texto se convierte en tokens:

| Lexema | Token ANTLR | Categoría (token_classifier) |
|---|---|---|
| `SELECT` | `K_SELECT` | Palabra reservada |
| `nombre` | `IDENTIFIER` | Identificador |
| `FROM` | `K_FROM` | Palabra reservada |
| `estudiantes` | `IDENTIFIER` | Identificador |
| `WHERE` | `K_WHERE` | Palabra reservada |
| `edad` | `IDENTIFIER` | Identificador |
| `>=` | `GTE` | Operador de comparación |
| `18` | `INT_LITERAL` | Literal entero |
| `;` | `SEMI` | Fin de sentencia |

Un carácter que no encaja en ningún token (por ejemplo `#`) produce
`LEX001 Símbolo no reconocido`.

### 4.3 Análisis sintáctico (`SQLParser`, generado por ANTLR)

El parser aplica las reglas de `SQL.g4` y produce el **parse tree**:
`script → statement → selectStatement → selectItem / fromClause / whereClause
→ expression (comparisonExpression) → …`. Si la estructura no encaja, se
produce `SYN001`, por ejemplo "Entrada inesperada 'SELEC'; se esperaba
{…}". El árbol se serializa con el `tree_serializer` reutilizado y se ve en la
pestaña **Árbol sintáctico**.

### 4.4 Construcción del AST (`ast_builder.cpp`)

Un Visitor recorre el parse tree y crea nodos con significado:

```text
SELECT
├── Proyección ── nombre
├── FROM ──────── estudiantes
└── WHERE ─────── >=
                  ├── edad
                  └── 18
```

Se descartan `;`, paréntesis y comas. Los nombres se pasan a minúsculas (SQL
no distingue mayúsculas) y los literales se convierten a valores (`'O''Brien'`
pasa a `O'Brien`).

### 4.5 Análisis semántico (`semantic_analyzer.cpp`)

Antes de ejecutar se comprueba contra el **catálogo** (`metadata.txt`) que:

1. `estudiantes` existe (si no, `SEM001`);
2. `nombre` y `edad` existen en esa tabla (si no, `SEM002`);
3. `edad >= 18` compara `INT` con `INT` (si no, `SEM005`);
4. la condición de `WHERE` es `BOOLEAN` (si no, `SEM019`).

Además **decora** el AST: a `edad` le anota su tipo (`INT`) y su posición en
la fila (`slot = 2`). El intérprete usa esa posición directamente y ya no
busca la columna por nombre.

Si cualquier sentencia del script tiene errores, **no se ejecuta ninguna**
y ningún archivo se modifica.

### 4.6 Interpretación (`executor.cpp`)

1. El **planificador** ve que `edad` tiene el índice `idx_estudiantes_edad` y
   que `18` es constante. En lugar de recorrer la tabla, pide al índice las
   claves `>= 18`, que son los rowids `{4, 5, 1, 7, 3, 6}`.
2. Busca sus posiciones en `estudiantes.rows` y lee solo esos registros de
   `estudiantes.csv` (o usa la caché si la tabla ya está en memoria).
3. Evalúa el `WHERE` completo sobre cada uno y proyecta `nombre`.
4. Registra en el **plan de ejecución**: *"Index Scan en estudiantes usando
   idx_estudiantes_edad (edad >= 18): se leyeron 6 registros de 8 sin
   recorrer la tabla."*

Sin índice, el plan dice *"Full Scan en estudiantes: se leyeron los 8
registros."*

### 4.7 Archivos y resultado

Un `SELECT` solo lee. Un `INSERT`, `UPDATE`, `DELETE`, `CREATE` o `DROP`
reescribe los archivos afectados (ver sección 6). El CLI responde con un JSON
que contiene tokens, árbol, AST, diagnósticos, resultados por sentencia, plan,
archivos escritos y el estado de la base. El IDE lo reparte en sus pestañas.

---

## 5. La gramática: qué cambió

`Compiscript.g4` describía un lenguaje de programación (funciones, clases,
ciclos). `SQL.g4` describe sentencias de bases de datos. Es la parte que más
cambió, como se esperaba.

| Aspecto | Decisión en `SQL.g4` | Por qué |
|---|---|---|
| Unidad | `script : (statement ';')* EOF` | Un archivo puede tener muchas sentencias |
| Sentencias | `createTable`, `dropTable`, `createIndex`, `dropIndex`, `insert`, `update`, `delete`, `select` | Todo el "SQL requerido" del enunciado |
| Mayúsculas | `options { caseInsensitive = true; }` | En SQL `select` y `SELECT` son iguales |
| Nombres de tokens | Prefijo `K_` (`K_NULL`, `K_DELETE`, `K_IN`…) | `NULL`, `TRUE`, `FALSE`, `DELETE` e `IN` son **macros de C/C++/Windows** y romperían el código generado |
| Precedencia | Alternativas etiquetadas de `expression` ordenadas: unario, `* / %`, `+ -`, comparación, `IS NULL`, `IN`, `NOT`, `AND`, `OR` | ANTLR da más precedencia a la primera alternativa; así `a OR b AND c` = `a OR (b AND c)` |
| Subconsultas | `IN (selectStatement)`, `EXISTS (selectStatement)`, `(selectStatement)` como valor | IN, NOT IN, EXISTS, NOT EXISTS y escalares |
| Agregaciones | `COUNT(*)`, `COUNT/SUM/AVG/MIN/MAX([DISTINCT] expr)` | Requisito del enunciado |
| Tipos | `INT`/`INTEGER`, `FLOAT`, `VARCHAR(n)`, `BOOLEAN`, `DATE` | Los tipos mínimos del enunciado |
| Literales | Enteros, decimales, `'texto'` (con `''` como escape), `TRUE`, `FALSE`, `NULL`, `DATE 'AAAA-MM-DD'` | |
| Restricciones | `PRIMARY KEY`, `NOT NULL`, `UNIQUE`, `REFERENCES`, `FOREIGN KEY (...)` | El enunciado pide definir las "restricciones necesarias" |
| Comentarios | `-- …` y `/* … */` se descartan (`-> skip`) | |
| Extras | `DISTINCT`, `AS`, `LIMIT`, `IF [NOT] EXISTS`, `IS [NOT] NULL` | Comodidad; no complican el diseño |

---

## 6. Almacenamiento y persistencia

### 6.1 Estructura de la carpeta

```text
database/
├── metadata.txt                 catálogo
├── carreras.csv                 una tabla = un CSV
├── estudiantes.csv
├── estudiantes.rows             rowid → posición y longitud en bytes del CSV
├── cursos.csv
├── inscripciones.csv
└── indexes/
    ├── pk_estudiantes.idx       índice automático de la PRIMARY KEY
    ├── uq_carreras_nombre.idx   índice automático de UNIQUE
    └── idx_estudiantes_edad.idx índice creado con CREATE INDEX
```

### 6.2 `metadata.txt`: el catálogo

Guarda **esquemas, tipos, restricciones e índices**. Se lee al arrancar y se
reescribe tras cada sentencia DDL o `INSERT`:

```text
TABLE estudiantes
  NEXT_ROWID 10
  COLUMN id INT PRIMARY_KEY NOT_NULL
  COLUMN nombre VARCHAR(50) NOT_NULL
  COLUMN edad INT
  COLUMN promedio FLOAT
  COLUMN activo BOOLEAN
  COLUMN ingreso DATE
  COLUMN carrera_id INT REFERENCES carreras.id
END

INDEX idx_estudiantes_edad ON estudiantes.edad
INDEX pk_estudiantes ON estudiantes.id UNIQUE AUTO
```

- La semántica usa los tipos y restricciones para validar **antes** de ejecutar.
- `NEXT_ROWID` es el siguiente identificador interno libre.
- `AUTO` marca los índices creados por `PRIMARY KEY` o `UNIQUE`; estos no se
  pueden eliminar con `DROP INDEX`.

### 6.3 `<tabla>.csv`: los registros

```text
__rowid,id,nombre,edad,promedio,activo,ingreso,carrera_id
1,1,"Ana López",20,91.5,TRUE,2023-01-15,1
2,2,"Luis Pérez",17,78.0,TRUE,2024-01-10,2
4,4,"Pedro Méndez",18,,TRUE,2024-01-10,
```

- **`__rowid`** identifica cada registro y nunca cambia, aunque se borren
  otros. Los índices apuntan a este número (igual que el `rowid` de SQLite).
  Si guardaran la posición de la fila, cada `DELETE` los desordenaría.
- **El texto siempre va entre comillas** y las comillas internas se duplican
  (`""`). Así un campo vacío **sin** comillas significa `NULL` y `""` significa
  texto vacío.
- Los números, `TRUE`/`FALSE` y las fechas `AAAA-MM-DD` se escriben tal cual.
- El lector respeta comas y saltos de línea dentro de las comillas.

### 6.4 `indexes/<índice>.idx`: los índices

```text
# INDEX idx_estudiantes_edad ON estudiantes(edad)
# valor,rowids (ordenado por valor)
16,8
17,2
18,4
20,1
```

Cada línea dice "este valor está en estos rowids". En memoria es un
`std::map<Value, std::set<rowid>>`, un árbol ordenado en memoria (no es un
árbol B), por eso sirve para `=`, `<>`, `IN` y para rangos
`<`, `<=`, `>`, `>=`. Los `NULL` no se indexan, porque `col = NULL` nunca es
verdadero.

Cada tabla también tiene un archivo de texto `<tabla>.rows`:

```text
ROWS 1 <tamaño_del_csv_en_bytes>
<rowid> <offset_en_bytes> <longitud_en_bytes>
```

El directorio permite hacer `seekg` directamente a cada candidato del índice.
Las posiciones incluyen comillas y saltos de línea dentro del texto. Se
actualizan en la misma confirmación que el CSV. Las bases anteriores sin este
archivo se cargan una vez y lo generan en la siguiente modificación.

### 6.5 Cómo persiste, paso a paso

1. Cada ejecución del CLI **arranca con la memoria vacía**: abre
   `metadata.txt` y carga las tablas e índices **la primera vez que los
   necesita** (carga perezosa).
2. Cada sentencia que modifica datos trabaja sobre **una copia** de la tabla
   y de sus índices.
3. Si termina bien, se preparan **todos** los archivos temporales (tabla,
   posiciones, índices y, si cambió, `metadata.txt`) antes de tocar originales.
4. Si falla a la mitad (por ejemplo, la 3.ª fila de un `INSERT` repite la
   PRIMARY KEY), la copia se descarta: **no quedan cambios parciales**.
5. `.transaction` guarda el manifiesto y los respaldos. Una instalación
   fallida se revierte; si el proceso se interrumpe, se recupera al abrir la
   base. El marcador `committed` conserva las operaciones ya confirmadas.
6. Las sentencias se confirman **una por una** (*autocommit*, como en la
   consola de MySQL). Si la sentencia 5 falla, las 4 anteriores ya quedaron
   guardadas y las siguientes se omiten.

Un bloqueo del sistema operativo (`.lock`) protege toda la ejecución del CLI,
incluida la recuperación: una segunda ejecución sobre la misma base recibe
`EXE008` mientras la primera la esté usando. El bloqueo se libera al cerrar
el proceso. El diario protege frente a interrupciones del proceso; no se
ofrece durabilidad ante cortes eléctricos mediante `fsync`.

---

## 7. El intérprete (`executor.cpp`)

### 7.1 Orden de evaluación de un SELECT

```text
FROM (Index Scan o Full Scan)
  → INNER JOIN (Index Nested Loop o Nested Loop)
    → WHERE
      → GROUP BY + agregaciones → HAVING
        → proyección (SELECT)
          → DISTINCT → ORDER BY → LIMIT
```

- **JOIN:** por cada fila acumulada se buscan las filas de la otra tabla que
  cumplen `ON`. Si `ON` tiene `columna_indexada = valor_conocido`, se busca en
  el índice (**Index Nested Loop**); si no, se prueban todas las combinaciones
  (**Nested Loop**).
- **GROUP BY:** las filas se agrupan por el valor de las expresiones. Cada
  agregación recorre las filas de su grupo. Sin `GROUP BY`, todas las filas
  forman un solo grupo, por eso `COUNT(*)` de una tabla vacía devuelve 0.
- **NULL:** se usa lógica de tres valores (`TRUE`, `FALSE`, `NULL`). `WHERE`
  solo conserva las filas `TRUE`. Las agregaciones ignoran los `NULL` (salvo
  `COUNT(*)`).
- **División:** `INT / INT` es división entera, como en PostgreSQL. Si algún
  operando es `FLOAT`, el resultado es decimal.

### 7.2 Subconsultas

- **No correlacionada** (no usa columnas de afuera): se ejecuta **una vez** y
  se guarda en caché. El plan dice "se ejecutó 1 vez y su resultado se
  reutilizó".
- **Correlacionada** (por ejemplo, `EXISTS (… WHERE i.estudiante_id = e.id)`):
  se reevalúa por cada fila externa. La semántica la detecta al resolver
  `e.id` en un nivel exterior (`depth > 0`) y la marca como correlacionada.
- **Escalar:** si devuelve más de una columna es error semántico (`SEM015`);
  si devuelve más de un registro es error de ejecución (`EXE005`); si no
  devuelve registros, su valor es `NULL`.

### 7.3 Uso y mantenimiento de índices

**Cuándo se usa un índice:** el planificador (`chooseIndex`) separa la
condición por sus `AND` y busca, en este orden:

1. `col = valor`;
2. `col IN (lista)` o `col IN (SELECT …)` no correlacionada;
3. `col <, <=, >, >= valor`.

También admite `<>`. Para `AND` intersecta los accesos disponibles; para
`OR` une los candidatos si **ambas ramas** pueden resolverse mediante índices.
Cuando una rama del `OR` carece de índice se recorre la tabla para conservar
todos los resultados correctos.

Aquí `col` tiene índice y `valor` ya se conoce antes de leer la tabla: es un
literal, una columna de una tabla anterior en el JOIN, una columna de la
consulta externa o una subconsulta escalar no correlacionada. Después de usar
el índice, el `WHERE` se evalúa completo, así el resultado es idéntico al de
un Full Scan.

**Cómo se mantiene al día:**

| Sentencia | Qué le pasa al índice |
|---|---|
| `INSERT` | Se agrega `valor → rowid` |
| `UPDATE` | Se quita la clave vieja y se agrega la nueva. Primero se retiran **todas** las viejas, así `UPDATE t SET id = id + 1` no reporta un falso duplicado |
| `DELETE` | Se quita `valor → rowid` |
| `DROP TABLE` | Se borran sus `.idx` |
| `CREATE INDEX` | Se recorre la tabla **una vez** para construirlo |

**Índices automáticos:** cada `PRIMARY KEY` y cada `UNIQUE` crean un índice.
Validar un duplicado o una clave foránea es entonces una búsqueda en el árbol
y no un recorrido de toda la tabla.

### 7.4 Restricciones

| Restricción | Cuándo se valida | Código |
|---|---|---|
| Tipo de columna, `VARCHAR(n)`, fecha válida | Semántica (literales) y ejecución (valores calculados) | `SEM005/008/009`, `EXE007` |
| `NOT NULL` / `PRIMARY KEY` no nula | Semántica y ejecución | `SEM007`, `EXE007` |
| `PRIMARY KEY` única | Ejecución, con su índice | `EXE001` |
| `UNIQUE` | Ejecución, con su índice | `EXE002` |
| `FOREIGN KEY` al insertar o actualizar | Ejecución: el valor debe existir en la tabla padre | `EXE003` |
| `FOREIGN KEY` al borrar o actualizar el padre | Ejecución, política **RESTRICT** | `EXE004` |
| `DROP TABLE` de una tabla referenciada | Semántica | `SEM017` |

---

## 8. Catálogo de errores

**Léxicos y sintácticos** (ANTLR, mensajes traducidos):
`LEX001` símbolo no reconocido · `SYN001` estructura inválida.

**Semánticos** (antes de ejecutar; con sentencia, línea y columna):

| Código | Error | Ejemplo |
|---|---|---|
| SEM001 | Tabla inexistente | `SELECT * FROM alumnos` |
| SEM002 | Columna inexistente | `SELECT apellido FROM estudiantes` |
| SEM003 | Referencia ambigua | `nombre` existe en `e` y en `c` |
| SEM004 | Alias o tabla inválida | `SELECT x.nombre FROM estudiantes e` |
| SEM005 | Tipos incompatibles | `edad = 'veinte'`, `nombre + 1` |
| SEM006 | Cantidad de valores en INSERT | 2 valores para 7 columnas |
| SEM007 | NULL u omisión en columna NOT NULL / PK | `INSERT INTO cursos (id) VALUES (9)` |
| SEM008 | Texto excede VARCHAR(n) | |
| SEM009 | Fecha inválida | `'2024-02-30'` |
| SEM010 | Tabla ya existe | |
| SEM011 | Columna duplicada | `CREATE TABLE t (a INT, a INT)` |
| SEM012 | Restricción inválida | dos PRIMARY KEY, FK a una columna no clave, borrar un índice automático |
| SEM013 | Índice inexistente | `DROP INDEX idx_x` |
| SEM014 | Índice duplicado | mismo nombre o columna ya indexada |
| SEM015 | Subconsulta con más de una columna | `edad = (SELECT edad, id …)` |
| SEM016 | Uso inválido de agregación | `COUNT` en WHERE, columna no agrupada |
| SEM017 | Tabla referenciada por una FK | `DROP TABLE carreras` |
| SEM018 | Literal fuera de rango | |
| SEM019 | Condición no booleana | `WHERE edad` |
| SEM020 | ORDER BY fuera de rango | `ORDER BY 5` con 2 columnas |

**De ejecución** (solo se conocen con los datos):
`EXE001` PK duplicada · `EXE002` UNIQUE duplicado · `EXE003` FK sin padre ·
`EXE004` registro referenciado · `EXE005` subconsulta escalar con más de un
registro · `EXE006` división entre cero · `EXE007` valor incompatible ·
`EXE008` error de lectura o escritura.

---

## 9. Explicación archivo por archivo

### Gramática y código generado

**`backend/sql/grammar/SQL.g4`** 🆕 — Única fuente de verdad del lenguaje.
La parte de parser (reglas en minúscula) define la estructura de cada
sentencia. La parte de lexer (reglas en MAYÚSCULA) define palabras
reservadas, operadores, literales, identificadores, comentarios y espacios.
Ver la sección 5.

**`backend/sql/generated/*`** ⚙️ — Lo produce ANTLR con
`-Dlanguage=Cpp -visitor -no-listener`. No se edita a mano. Contiene
`SQLLexer` (tokens), `SQLParser` (una clase `XxxContext` por regla) y
`SQLBaseVisitor` (un método `visitXxx` por regla o alternativa etiquetada).

### Infraestructura reutilizada

**`diagnostics.h/.cpp`** ♻️ — `CollectingErrorListener` reemplaza el listener
de ANTLR que imprime en consola. Guarda cada error como `Diagnostic` con
código, categoría, línea y columna. Es el mismo del Proyecto 2; solo se agregó
`translate()`, que pasa al español los mensajes estándar de ANTLR
("mismatched input" → "Entrada inesperada").

**`tree_serializer.h/.cpp`** ♻️ — Recorre el parse tree y produce JSON
`{kind, name, text, line, column, children}`. Es idéntico al del Proyecto 2.

**`token_classifier.h/.cpp`** ♻️ — Traduce `K_SELECT`, `GTE`,
`INT_LITERAL`… a categorías legibles ("Palabra reservada", "Operador de
comparación"). Mantiene la misma interfaz; cambian las listas de palabras.

**`json.h/.cpp`** ♻️/🆕 — `quoteJson` (escapar texto en JSON) se copió del
Proyecto 2. `runToJson` (el resultado completo) y `databaseToJson` (catálogo
y archivos) son nuevos.

### Modelo de datos

**`value.h/.cpp`** 🆕 — Define `DataType` (INT, FLOAT, VARCHAR(n), BOOLEAN,
DATE) y `Value`, el valor de una celda o de una expresión, que puede ser
NULL. Funciones clave:

- `compareValues`: orden total usado por `ORDER BY`, comparaciones e índices.
- `isValidDate`: valida `AAAA-MM-DD`, incluidos los años bisiestos.
- `coerceToType`: convierte un valor al tipo de una columna antes de
  guardarlo. Promueve `INT` a `FLOAT`, convierte texto a `DATE`, valida el
  largo de `VARCHAR` y rechaza lo demás.

**`model.h`** 🆕 (misma idea que el del Proyecto 2) — Estructuras que viajan
al IDE: `Diagnostic`, `TokenInfo`, `StatementResult` (estado, mensaje,
columnas, filas, plan y archivos de cada sentencia) y `RunResult` (todo el
pipeline).

### AST

**`ast.h`** 🆕 — Nodos del AST:

- `Expr` es un solo tipo de nodo con un campo `kind` (Literal, Column, Unary,
  Binary, IsNull, InList, InSubquery, Exists, ScalarSubquery, Aggregate).
  Sus hijos están en `children`.
- Hay una estructura por sentencia (`CreateTableStatement`,
  `InsertStatement`, `SelectStatement`, …) y `Statement` guarda cualquiera
  de ellas en un `std::variant`.
- Los campos marcados como *decoración* los llena la semántica: `type`,
  `slot`, `depth`, `correlated`, `projection`, `schema`, entre otros.

**`ast_builder.h/.cpp`** 🆕 — Hereda de `SQLBaseVisitor` (el Visitor generado
por ANTLR), igual que la semántica del Proyecto 2. Cada `visitXxx` devuelve
dentro de `std::any` un puntero recién creado, y quien llama lo adopta en un
`unique_ptr`. Se usa puntero porque `std::any` exige tipos copiables y los
nodos del AST no lo son. Además:

- normaliza nombres a minúsculas;
- quita las comillas de los textos;
- convierte `!=` en `<>`;
- pliega `-5` a un literal negativo;
- descarta los paréntesis, que solo cambian la forma del árbol;
- guarda línea, columna y texto original de cada nodo para los mensajes de
  error.

**`ast_serializer.h/.cpp`** 🆕 — Convierte el AST, ya decorado, a JSON con el
**mismo formato** que el parse tree. Así el IDE dibuja los dos árboles con el
mismo componente. Los nodos internos son operaciones o cláusulas (`SELECT`,
`WHERE`, `>=`, `COUNT`). Las hojas son valores: tablas, columnas con su tipo
y literales.

### Semántica

**`semantic_analyzer.h/.cpp`** 🆕 —

- **Recibe una copia del catálogo.** Si la sentencia 1 hace `CREATE TABLE`,
  la sentencia 2 ya ve la tabla, pero nada se escribe en disco.
- **Ámbitos.** Cada `SELECT` abre un `Scope` con su `RowLayout` (columnas de
  `FROM` y de cada `JOIN`). Una subconsulta abre otro encima, así
  `resolveColumn` busca primero adentro y luego afuera. Si la columna se
  encuentra afuera (`depth > 0`), la subconsulta se marca correlacionada.
  Es la misma idea de las tablas de símbolos encadenadas del Proyecto 2.
- **`check()`** recorre cada expresión, infiere su tipo y aplica las reglas
  de la sección 8.
- **`checkGrouped()`** verifica que en una consulta agregada cada columna
  esté en `GROUP BY` o dentro de una agregación.
- **Restricciones del CREATE TABLE.** Las integra en el esquema final, que
  guarda en el AST junto con los índices automáticos que se crearán.

### Almacenamiento

**`catalog.h/.cpp`** 🆕 — `Catalog` contiene las tablas (`TableSchema` con
sus `ColumnSchema`) y los índices (`IndexSchema`). `load()` lee el catálogo y
`serialize()` produce el texto que la transacción guarda en `metadata.txt`.
También define
`RowLayout`, el orden de las columnas en una fila combinada de JOIN, que
comparten la semántica y el intérprete.

**`storage.h/.cpp`** 🆕 — El "motor de almacenamiento":

- `TableData` es `map<rowid, fila>` e `IndexData` es
  `map<valor, set<rowid>>`.
- `table()` e `index()` cargan los archivos de forma perezosa y los guardan
  en caché.
- `readRows()` consulta `.rows` y lee los candidatos mediante posiciones de byte.
- `rowCount()` obtiene el tamaño de una tabla sin cargar todo el CSV.
- `saveTable()` y `saveIndex()` preparan los archivos de una misma transacción.
- `file_transaction.h/.cpp` administra temporales, respaldos, confirmación y
  recuperación de escrituras y eliminaciones.
- `buildIndex()` crea un índice recorriendo la tabla una vez.
- `parseCsv()` y `encodeCsvValue()` implementan el formato CSV con comillas.
- `reset()` vacía la base y `files()` entrega el contenido de los archivos
  al IDE.

### Interpretación

**`executor.h/.cpp`** 🆕 —

- **`execute()`** atiende una sentencia. Si ocurre un `ExecutionError`, recarga
  el estado desde disco, lo que descarta la copia de trabajo.
- **DDL:** `createTable`, `dropTable`, `createIndex` y `dropIndex` actualizan
  el catálogo y los archivos.
- **DML:** `insert`, `update` y `remove` trabajan sobre copias. Validan con
  `coerceRow` (tipos y NOT NULL), `checkUnique` (PK y UNIQUE vía índice),
  `checkForeignKeys` (el padre existe) y `checkReferencedBy` (RESTRICT).
  Después mantienen los índices y llaman a `persistTable`.
- **`chooseIndex()`** es el planificador de la sección 7.3.
- **`runSelect()`** implementa el pipeline de la sección 7.1.
- **`subqueryRows()`** maneja la caché de subconsultas no correlacionadas y
  cuenta las ejecuciones.
- **`evaluate()`, `evaluateBinary()` y `evaluateAggregate()`** evalúan
  expresiones con lógica de tres valores, aritmética y agregaciones.
- **Plan de ejecución.** Cada decisión se anota en `result.plan` y el IDE la
  muestra.

### Orquestación y CLI

**`service.h/.cpp`** ♻️ (adaptado) — `runScript()` encadena las fases:
lexer → parser → AST → semántica → intérprete. Es la misma estructura del
`analyzeSource()` del Proyecto 2: cada fase se omite, explicando el motivo,
si la anterior falló. Ahora también abre la base y ejecuta en modo
`execute` o `analyze`. `describeDatabase()` y `resetDatabase()` atienden la
pestaña Base de datos.

**`main.cpp`** ♻️ (adaptado) — El CLI conserva los modos `--file` y `--stdin`
del Proyecto 2 y agrega:

- `--db <carpeta>`: dónde está la base;
- `--analyze`: valida sin ejecutar;
- `--catalog`: solo describe la base;
- `--reset`: vacía la base.

Imprime un único JSON. El código de salida es 0 si todo terminó bien, 1 si
hubo errores y 2 si se usó mal el CLI.

### Web

**`app.py`** ✏️ — Sigue siendo solo un puente. Los endpoints son:

- `/api/run`, que ejecuta o analiza;
- `/api/database`, que devuelve el catálogo y los archivos;
- `/api/database/reset`;
- `/api/tests`;
- `/api/files`, `/api/file/<nombre>` y `/api/save` para los ejemplos `.sql`.

**`frontend/templates/index.html`**, **`static/js/app.js`** y
**`static/css/style.css`** ✏️ — Se reutilizaron la gestión de archivos, los
paneles de diagnósticos y tokens, el reporte de pruebas y el visor SVG de
árboles. El visor se generalizó para dibujar dos árboles: antes usaba una
variable global y ahora recibe una "vista" por panel. Lo nuevo es:

- la pestaña **Resultados**: una tarjeta por sentencia, con su tabla de
  resultados, el plan de ejecución (las líneas que usan índice en verde) y
  los archivos escritos;
- la pestaña **AST**;
- la pestaña **Base de datos**: el catálogo más un explorador de los archivos
  reales;
- los botones **Analizar sin ejecutar** y **Reiniciar base de datos**.

### Pruebas, ejemplos y construcción

**`backend/tests/sql_tests.cpp`** 🆕 — 117 casos en 10 categorías: entregas,
almacenamiento y regresiones de robustez. Cada caso crea una base temporal,
carga una base de ejemplo si la necesita, ejecuta el SQL y verifica:

- filas resultantes;
- códigos de error;
- líneas del plan (por ejemplo, que diga "Index Scan");
- el contenido real de `metadata.txt`, `.csv` e `.idx`.
- filas efectivamente leídas, posiciones tras cambios, Unicode, precisión
  decimal, desbordamientos, errores de escritura, bloqueo y recuperación.

El reporte `--json` tiene el formato del Proyecto 2.

**`backend/examples/sql/01…09`** 🆕 — Scripts para la demostración, en orden:
esquema, datos, SELECT/WHERE, UPDATE/DELETE, JOIN/agregaciones,
índices/subconsultas, errores semánticos, errores de ejecución y errores
léxicos/sintácticos.

**`CMakeLists.txt`** ✏️ — Crea la biblioteca `sql_core` (todo el núcleo), el
ejecutable `sql_cli` (en la raíz, para Flask) y `sql_tests` (registrado en
CTest). Compiscript se compila solo con `-DBUILD_COMPISCRIPT=ON`. Con MinGW el
runtime de GCC se enlaza estático, así el `.exe` no depende de que el `PATH`
tenga la misma `libstdc++` (en Git Bash había otra versión y el ejecutable no
arrancaba).

---

## 10. Cómo compilar y ejecutar

```powershell
.\scripts\build_sql.ps1          # Windows
```

```bash
./scripts/build_sql.sh           # macOS / Linux
```

Ambos scripts descargan ANTLR si hace falta, regeneran el parser, compilan y
corren las 117 pruebas. Si ya tienes el runtime C++ de ANTLR descargado, define
`ANTLR4_LOCAL_SOURCE` con su ruta para no descargarlo otra vez.

Para abrir el IDE:

```bash
python app.py                    # abre http://127.0.0.1:5050
```

Recorrido sugerido para la demostración:

1. Ejecuta `01_crear_esquema.sql` y luego `02_insertar_datos.sql`.
2. Abre la pestaña **Base de datos** y muestra `metadata.txt`, un `.csv` y un
   `.idx`.
3. Ejecuta `03`, `05` y `06`. En **Resultados** muestra el plan con
   *Index Scan* e *Index Nested Loop*.
4. Escribe una consulta, presiona **Analizar sin ejecutar** y muestra las
   pestañas **Tokens**, **Árbol sintáctico** y **AST**.
5. Ejecuta `07`, `08` y `09` para mostrar los tres tipos de errores.
6. Presiona **Ejecutar batería de pruebas**.

---

## 11. Preguntas probables y respuestas

**¿Qué reutilizaron del Proyecto 2?**
ANTLR y su integración con CMake; el listener de errores; la serialización
del parse tree; `quoteJson`; la estructura del servicio que encadena fases y
omite las siguientes si una falla; el CLI con salida JSON; Flask como
adaptador; el IDE (archivos, diagnósticos, tokens, visor de árboles); y el
formato y la presentación de la batería de pruebas.

**¿Qué cambió más?**
La gramática, y todo lo que ocurre después del parser. En el Proyecto 2,
después de la semántica venía la generación de TAC. Aquí después del parser
viene un **AST nuevo**, una **semántica contra catálogo** y un
**intérprete** con almacenamiento propio.

**¿Por qué construir un AST si ANTLR ya da un árbol?**
El parse tree tiene cada token y una regla por nivel de precedencia. El AST
solo tiene lo que importa para ejecutar, y la semántica lo **decora** con
tipos y posiciones de columnas. Interpretar el AST es más simple y no depende
de cómo se escribió la gramática.

**¿Por qué la semántica trabaja sobre una copia del catálogo?**
Así puede validar un script completo en orden (un `CREATE TABLE` seguido de
un `INSERT` en esa tabla) sin tocar los archivos. Si hay cualquier error
semántico, no se ejecuta nada.

**¿Cómo saben que usaron el índice?**
El plan de ejecución lo reporta: *"Index Scan … se leyeron 6 registros de 8
sin recorrer la tabla"*. Las pruebas verifican esa línea, y también que el
resultado sea igual al de un recorrido completo.
Además, las regresiones verifican el contador `database.reads.table_rows`:
una consulta fría por clave lee una sola fila; un JOIN de tres tablas lee
solo sus tres candidatas. La vista de archivos lee una vista previa acotada
y el catálogo cuenta filas mediante `.rows`, sin decodificar los CSV completos.

**¿Qué es `__rowid`?**
Un identificador interno estable de cada registro. Los índices guardan
rowids; si guardaran posiciones, cada `DELETE` los invalidaría.

**¿Qué pasa si una sentencia falla a la mitad?**
No deja cambios: trabaja sobre una copia y solo se guarda si termina. Las
sentencias anteriores del script ya quedaron guardadas (autocommit) y las
siguientes se omiten.

**¿Cómo distinguen NULL de texto vacío en el CSV?**
NULL es un campo vacío sin comillas y el texto vacío es `""`, porque el texto
siempre se escribe entre comillas.

**¿Por qué los tokens se llaman `K_SELECT` y no `SELECT`?**
ANTLR crea constantes C++ con esos nombres, y `NULL`, `TRUE`, `FALSE`,
`DELETE` o `IN` ya son macros en C/C++ o en Windows. El prefijo evita el
choque.

**¿Usan algún DBMS?**
No. Todo se guarda en archivos de texto `.txt`, `.csv`, `.rows` e `.idx`.
La única dependencia del núcleo es el runtime de ANTLR.

## 12. Alcance y limitaciones conocidas

- Solo `INNER JOIN`. `LEFT JOIN` no es requisito del enunciado.
- Índices de una sola columna; `PRIMARY KEY` y `FOREIGN KEY` de una columna.
- Política `RESTRICT` para las claves foráneas (no hay `CASCADE`).
- El texto se compara byte a byte (colación binaria). Por eso, en `ORDER BY`,
  las mayúsculas van antes que las minúsculas y las vocales acentuadas después
  de la `z`.
- Cada sentencia es atómica, pero no hay transacciones de varias sentencias
  (`BEGIN`/`COMMIT`), porque no son parte del enunciado.
- Los recorridos completos y las modificaciones cargan la tabla en memoria.
  Las consultas indexadas y los JOIN leen candidatos directamente del CSV.
- `VARCHAR(n)` cuenta puntos de código Unicode; no realiza normalización ni
  cuenta grafemas compuestos como una sola unidad. FLOAT conserva el valor
  binario al persistir; un literal o resultado fuera de rango se diagnostica.
- `HAVING` sin `GROUP BY` crea un grupo implícito; no permite columnas sin
  agrupar. `NULL NOT IN (subconsulta vacía)` produce TRUE.
- La recuperación cubre interrupciones del proceso; no se implementa `fsync`
  para garantizar persistencia frente a un corte eléctrico.
