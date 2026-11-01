# Intérprete SQL con almacenamiento en archivos

Proyecto 3 de Compiladores 2, desarrollado por:

- Joel Jaquez — #23369
- Luis Gonzalez — #23353

El sistema interpreta y ejecuta sentencias SQL usando su propio mecanismo de
almacenamiento en archivos de texto. **No usa ningún DBMS** (ni SQLite, ni
MySQL, ni PostgreSQL). Está escrito en C++17 con ANTLR 4.13.2 y reutiliza la
infraestructura del Proyecto 2 (Compiscript).

> La explicación completa para la defensa está en
> [docs/proyecto3-interprete-sql.md](docs/proyecto3-interprete-sql.md). Incluye
> qué se reutilizó y qué es nuevo, el árbol de archivos, el flujo de una
> sentencia, el formato de almacenamiento, cada archivo explicado y preguntas
> frecuentes.

## Pipeline

```text
SQL
 │
 ▼
SQLLexer (ANTLR, C++) ─────────────► tabla de tokens ► IDE
 │ tokens
 ▼
SQLParser (ANTLR, C++) ────────────► árbol sintáctico ► IDE
 │ parse tree
 ▼
AstBuilder (Visitor C++) ──────────► AST ► IDE
 │ AST
 ▼
SemanticAnalyzer ── catálogo (metadata.txt)
 │ AST decorado (tipos y columnas resueltas)
 ▼
Executor (intérprete) ─ planificador de índices
 │
 ▼
database/  metadata.txt · <tabla>.csv · <tabla>.rows · indexes/<índice>.idx
 │
 ▼
Resultado JSON ─► Flask (adaptador) ─► IDE web
```

## Cobertura del enunciado

| Requisito | Implementación |
|---|---|
| Lexer y parser | `backend/sql/grammar/SQL.g4`; el código C++ lo genera ANTLR |
| AST | `ast.h`, `ast_builder.cpp` y `ast_serializer.cpp`, visible en el IDE |
| Tipos | `INT`, `FLOAT`, `VARCHAR(n)`, `BOOLEAN`, `DATE`, guardados en el esquema y usados en la semántica y la ejecución |
| SQL requerido | `CREATE/DROP TABLE`, `CREATE/DROP INDEX`, `INSERT`, `UPDATE`, `DELETE`, `SELECT … FROM … WHERE` con `AND`/`OR`, `ORDER BY`, `INNER JOIN`, `COUNT/SUM/AVG/MIN/MAX`, `GROUP BY`, `HAVING`, `IN`, `EXISTS` |
| Extras | `NOT IN`, `NOT EXISTS`, `IS [NOT] NULL`, `DISTINCT`, `AS`, `LIMIT`, `IF [NOT] EXISTS` |
| Restricciones | `PRIMARY KEY`, `NOT NULL`, `UNIQUE`, `FOREIGN KEY`/`REFERENCES` (política RESTRICT) |
| Índices | Archivos `.idx` ordenados. El planificador los usa en `WHERE` y `JOIN` y se mantienen al día con `INSERT`, `UPDATE` y `DELETE`. `PRIMARY KEY` y `UNIQUE` crean índices automáticos |
| Subconsultas | `IN`/`NOT IN`, `EXISTS`/`NOT EXISTS` (correlacionadas o no) y escalares en comparaciones |
| Análisis semántico | 20 reglas (`SEM001`–`SEM020`) con sentencia, línea y columna. Se valida todo el script antes de ejecutar |
| Interfaz gráfica | Editor; pestañas de tokens, árbol sintáctico, AST, errores, resultados con plan de ejecución y base de datos con los archivos reales |

## Almacenamiento

```text
database/
├── metadata.txt          esquemas, tipos, restricciones e índices
├── estudiantes.csv       __rowid + columnas; NULL = campo vacío; texto entre comillas
├── estudiantes.rows      rowid -> posición y longitud en bytes dentro del CSV
├── cursos.csv
└── indexes/
    ├── pk_estudiantes.idx    valor -> rowids, ordenado por valor
    └── idx_estudiantes_edad.idx
```

Cada sentencia prepara todos sus archivos `.tmp` y respalda los originales
antes de reemplazarlos. Un diario permite revertir errores y recuperar una
instalación interrumpida al abrir la base. Un bloqueo del sistema operativo
impide que dos procesos modifiquen la misma base a la vez.

Las consultas indexadas usan `<tabla>.rows` para leer solo los registros
candidatos, incluidos los JOIN. `AND` intersecta candidatos y `OR` los une
cuando ambas ramas tienen acceso por índice. Los cambios de datos reescriben
el CSV y sus posiciones juntos. Las bases antiguas sin `.rows` siguen siendo
compatibles y obtienen este archivo en la siguiente modificación.

## Requisitos

- CMake 3.20 o superior y un compilador C++17.
- Java, para ejecutar la herramienta de ANTLR.
- Python 3 con Flask (`python -m pip install -r requirements.txt`).

## Compilar y probar

```powershell
.\scripts\build_sql.ps1        # Windows
```

```bash
./scripts/build_sql.sh         # macOS / Linux
```

Estos scripts:

1. descargan el JAR de ANTLR si falta;
2. regeneran el lexer, el parser y el Visitor C++;
3. compilan `sql_cli` y `sql_tests`;
4. ejecutan la batería de **117 pruebas**, incluidos errores esperados,
   reapertura, acceso directo, Unicode, rangos numéricos y fallos de disco.

Si ya tienes el runtime C++ de ANTLR descargado, define `ANTLR4_LOCAL_SOURCE`
con su ruta para no descargarlo de nuevo.

```bash
ctest --test-dir build/sql --output-on-failure
```

## Ejecutar el IDE

```bash
python app.py
```

Abre `http://127.0.0.1:5050`. Los ejemplos de `backend/examples/sql/` están
numerados en el orden de las entregas: primero `01` (esquema) y `02` (datos),
luego el resto.

## CLI

```bash
./sql_cli --db database --file backend/examples/sql/03_select_where.sql
./sql_cli --db database --analyze --stdin < script.sql   # valida sin ejecutar
./sql_cli --db database --catalog                        # catálogo y archivos
./sql_cli --db database --reset                          # vacía la base
```

La salida es un único documento JSON. El código de salida es 0 si todo
terminó bien, 1 si hubo errores y 2 si se usó mal el CLI.

## Estructura relevante

```text
backend/sql/
├── grammar/SQL.g4          gramática SQL (fuente de verdad)
├── generated/              lexer, parser y Visitor C++ generados por ANTLR
├── include/sql/            API: valores, AST, catálogo, almacenamiento, semántica, intérprete
└── src/                    implementación y CLI (main.cpp)
backend/examples/sql/       scripts de demostración 01–09
backend/tests/sql_tests.cpp batería de integración
frontend/                   IDE web
app.py                      adaptador Flask (no interpreta SQL)
docs/proyecto3-interprete-sql.md
```

## Historial

- El **Proyecto 2 (Compiscript, generación de TAC)** sigue completo en
  `backend/compiscript/` y en la rama `Compiladores2-Proyecto2`. Se puede
  compilar desde esta rama con:

  ```bash
  cmake -S . -B build/compiscript -DBUILD_COMPISCRIPT=ON
  ```

  Su documentación está en `docs/arquitectura-compiscript.md` y
  `docs/codigo-intermedio-tac.md`.
- El trabajo de Compiladores 1 permanece en la rama `compiladores-1`.
