// Batería de integración del intérprete SQL. Cada caso usa una base de datos
// temporal y aislada; el reporte JSON tiene el mismo formato que la batería
// del Proyecto 2, por eso el IDE lo muestra con el mismo componente.
#include "sql/json.h"
#include "sql/service.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

using Result = sql::RunResult;

// Base de ejemplo que se carga antes de los casos que la necesitan.
const char *SEED = R"SQL(
CREATE TABLE carreras (id INT PRIMARY KEY, nombre VARCHAR(40) NOT NULL UNIQUE);
CREATE TABLE estudiantes (id INT PRIMARY KEY, nombre VARCHAR(50) NOT NULL, edad INT,
  promedio FLOAT, activo BOOLEAN, ingreso DATE, carrera_id INT REFERENCES carreras(id));
CREATE TABLE cursos (id INT PRIMARY KEY, nombre VARCHAR(60) NOT NULL, creditos INT NOT NULL);
CREATE TABLE inscripciones (id INT PRIMARY KEY,
  estudiante_id INT NOT NULL REFERENCES estudiantes(id),
  curso_id INT NOT NULL REFERENCES cursos(id), nota FLOAT);
INSERT INTO carreras VALUES (1, 'Computación'), (2, 'Mecatrónica'), (3, 'Matemática');
INSERT INTO estudiantes VALUES
  (1, 'Ana', 20, 91.5, TRUE, '2023-01-15', 1),
  (2, 'Luis', 17, 78.0, TRUE, '2024-01-10', 2),
  (3, 'Sofía', 22, 85.0, FALSE, '2021-07-01', 1),
  (4, 'Pedro', 18, NULL, TRUE, '2024-01-10', NULL),
  (5, 'María', 19, 88.0, TRUE, '2023-07-03', 3);
INSERT INTO cursos VALUES (1, 'Compiladores', 5), (2, 'Bases de Datos', 4),
  (3, 'Cálculo', 4), (4, 'Física', 4);
INSERT INTO inscripciones VALUES (1, 1, 1, 90.0), (2, 1, 3, 88.0), (3, 2, 3, 61.0),
  (4, 3, 1, 82.0), (5, 5, 3, 85.0), (6, 5, 2, NULL);
)SQL";

struct Outcome {
  Result main;
  Result followup;
  std::string database;
};

struct TestCase {
  std::string category;
  std::string rule;
  std::string name;
  std::string kind; // "Caso válido" | "Error esperado"
  bool seed = true;
  std::string source;
  std::string followup; // segunda ejecución sobre la misma base (persistencia)
  std::string expected;
  std::string explanation;
  std::function<bool(const Outcome &)> verify;
};

struct TestRun {
  TestCase test;
  bool passed = false;
  std::string actual;
  std::vector<std::string> codes;
};

// ── Ayudas para verificar resultados ──

std::vector<std::string> codes(const Result &result) {
  std::vector<std::string> values;
  for (const auto *group : {&result.lexical_diagnostics, &result.syntax_diagnostics,
                            &result.semantic_diagnostics, &result.runtime_diagnostics}) {
    for (const auto &diagnostic : *group)
      values.push_back(diagnostic.code);
  }
  return values;
}

bool hasCode(const Result &result, const std::string &code) {
  for (const auto &value : codes(result)) {
    if (value == code)
      return true;
  }
  return false;
}

// Solo ese código y la ejecución no se completó.
bool rejectedWith(const Result &result, const std::string &code) {
  return !result.ok && hasCode(result, code);
}

const sql::StatementResult &last(const Result &result) {
  static const sql::StatementResult empty;
  return result.statements.empty() ? empty : result.statements.back();
}

std::vector<std::string> column(const sql::StatementResult &statement, size_t index = 0) {
  std::vector<std::string> values;
  for (const auto &row : statement.rows)
    values.push_back(index < row.size() ? row[index].toString() : "?");
  return values;
}

std::vector<std::string> list(std::initializer_list<const char *> values) {
  return std::vector<std::string>(values.begin(), values.end());
}

bool planContains(const sql::StatementResult &statement, const std::string &text) {
  for (const auto &line : statement.plan) {
    if (line.find(text) != std::string::npos)
      return true;
  }
  return false;
}

std::string readFile(const fs::path &path) {
  std::ifstream file(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool contains(const std::string &text, const std::string &fragment) {
  return text.find(fragment) != std::string::npos;
}

std::string join(const std::vector<std::string> &values, const std::string &separator) {
  std::ostringstream output;
  for (size_t index = 0; index < values.size(); ++index)
    output << (index ? separator : "") << values[index];
  return output.str();
}

std::string describe(const Result &result) {
  const auto found = codes(result);
  if (!found.empty()) {
    std::string text = "Rechazado con " + join(found, ", ");
    for (const auto *group : {&result.lexical_diagnostics, &result.syntax_diagnostics,
                              &result.semantic_diagnostics, &result.runtime_diagnostics}) {
      if (!group->empty())
        return text + ": " + group->front().message;
    }
    return text;
  }
  const auto &statement = last(result);
  std::string text = result.mode == "analyze" ? "Análisis válido" : "Ejecutado";
  text += ": " + statement.message;
  if (!statement.columns.empty() && !statement.rows.empty()) {
    std::vector<std::string> rows;
    for (const auto &row : statement.rows) {
      std::vector<std::string> cells;
      for (const auto &value : row)
        cells.push_back(value.toString());
      rows.push_back("(" + join(cells, ", ") + ")");
      if (rows.size() == 6) {
        rows.push_back("…");
        break;
      }
    }
    text += " " + join(rows, " ");
  }
  return text;
}

// ── Casos ──

std::vector<TestCase> cases() {
  std::vector<TestCase> tests;
  auto valid = [&](std::string category, std::string rule, std::string name,
                   std::string source, std::string expected, std::string explanation,
                   std::function<bool(const Outcome &)> verify, bool seed = true,
                   std::string followup = "") {
    tests.push_back({std::move(category), std::move(rule), std::move(name), "Caso válido",
                     seed, std::move(source), std::move(followup), std::move(expected),
                     std::move(explanation), std::move(verify)});
  };
  auto invalid = [&](std::string category, std::string rule, std::string name,
                     std::string source, std::string code, std::string explanation,
                     bool seed = true) {
    tests.push_back({std::move(category), std::move(rule), std::move(name), "Error esperado",
                     seed, std::move(source), "", "Rechazo con " + code,
                     std::move(explanation),
                     [code](const Outcome &o) { return rejectedWith(o.main, code); }});
  };

  // 1. Léxico y sintaxis
  const std::string L = "1. Lexer y parser";
  valid(L, "Palabras reservadas", "Las palabras clave no distinguen mayúsculas",
        "select NOMBRE from Estudiantes where ID = 1;", "Devuelve Ana",
        "SQL no distingue mayúsculas en palabras clave ni identificadores.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Ana"}); });
  valid(L, "Comentarios", "Los comentarios -- y /* */ se ignoran",
        "-- comentario\nSELECT /* en medio */ nombre FROM estudiantes WHERE id = 2;",
        "Devuelve Luis", "El lexer descarta comentarios con la regla -> skip.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Luis"}); });
  valid(L, "Tokens", "La tabla de tokens conserva tipo, lexema y posición",
        "SELECT nombre FROM estudiantes;", "5 tokens; el primero es K_SELECT en 1:1",
        "Cada token conserva su tipo ANTLR, el texto y la línea/columna.",
        [](const Outcome &o) {
          const auto &t = o.main.tokens;
          return t.size() == 5 && t[0].type == "K_SELECT" && t[0].line == 1 &&
                 t[0].column == 1 && t[1].type == "IDENTIFIER" && t[4].type == "SEMI";
        });
  invalid(L, "Error léxico", "Símbolo no reconocido",
          "SELECT nombre FROM estudiantes WHERE edad > # 18;", "LEX001",
          "El carácter # no pertenece a ningún token de la gramática.");
  invalid(L, "Error sintáctico", "Palabra clave mal escrita",
          "SELEC nombre FROM estudiantes;", "SYN001",
          "SELEC no es una palabra reservada, así que la sentencia no empieza como ninguna regla.");
  invalid(L, "Error sintáctico", "Paréntesis sin cerrar",
          "INSERT INTO cursos VALUES (9, 'Lógica', 3;", "SYN001",
          "La fila de VALUES abre un paréntesis que nunca se cierra.");
  valid(L, "Fases omitidas", "Con errores sintácticos no se construye AST ni se ejecuta",
        "SELECT FROM;", "Semántica y ejecución omitidas, sin AST",
        "Las fases siguientes necesitan un árbol válido; por eso se omiten.",
        [](const Outcome &o) {
          return !o.main.syntax_ok && o.main.semantic_skipped && o.main.execution_skipped &&
                 o.main.ast_json.empty();
        });

  // 2. AST
  const std::string A = "2. AST";
  valid(A, "Precedencia", "AND se agrupa antes que OR",
        "SELECT TRUE OR FALSE AND FALSE AS r;", "TRUE",
        "El AST queda OR(TRUE, AND(FALSE, FALSE)); si OR se agrupara primero daría FALSE.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"TRUE"}); },
        false);
  valid(A, "Precedencia", "* se agrupa antes que +", "SELECT 2 + 3 * 4 AS r;", "14",
        "El AST queda +(2, *(3, 4)).",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"14"}); },
        false);
  valid(A, "Paréntesis", "Los paréntesis cambian la agrupación", "SELECT (2 + 3) * 4 AS r;",
        "20", "Los paréntesis no generan un nodo: solo cambian la forma del árbol.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"20"}); },
        false);
  valid(A, "AST decorado", "La semántica anota tipos en el AST",
        "SELECT nombre FROM estudiantes WHERE edad >= 18;",
        "El AST contiene la sentencia SELECT, WHERE y la columna edad tipada como INT",
        "El AST se serializa después de la semántica, con el tipo de cada columna.",
        [](const Outcome &o) {
          return contains(o.main.ast_json, "\"name\":\"SELECT\"") &&
                 contains(o.main.ast_json, "\"name\":\"WHERE\"") &&
                 contains(o.main.ast_json, "Columna INT");
        });

  // 3. Análisis semántico
  const std::string S = "3. Análisis semántico";
  invalid(S, "Tabla inexistente", "SELECT sobre una tabla que no existe",
          "SELECT nombre FROM alumnos;", "SEM001", "alumnos no está en el catálogo.");
  invalid(S, "Columna inexistente", "Columna que no pertenece a la tabla",
          "SELECT apellido FROM estudiantes;", "SEM002",
          "estudiantes no tiene una columna apellido.");
  invalid(S, "Referencia ambigua", "Columna presente en dos tablas del JOIN",
          "SELECT nombre FROM estudiantes e JOIN carreras c ON e.carrera_id = c.id;", "SEM003",
          "nombre existe en estudiantes y en carreras; hay que calificarla.");
  invalid(S, "Referencia inválida", "Alias que no está en FROM",
          "SELECT x.nombre FROM estudiantes e;", "SEM004", "El alias x no fue declarado.");
  invalid(S, "Tipos incompatibles", "Comparar INT con texto",
          "SELECT nombre FROM estudiantes WHERE edad = 'veinte';", "SEM005",
          "Un INT no puede compararse con un VARCHAR.");
  invalid(S, "Tipos incompatibles", "Sumar texto", "SELECT nombre + 1 FROM estudiantes;",
          "SEM005", "La suma solo acepta operandos numéricos.");
  invalid(S, "INSERT incorrecto", "Cantidad de valores distinta",
          "INSERT INTO cursos VALUES (9, 'Lógica');", "SEM006",
          "cursos tiene 3 columnas y la fila trae 2 valores.");
  invalid(S, "INSERT incorrecto", "Tipo de valor incompatible con la columna",
          "INSERT INTO cursos VALUES ('nueve', 'Lógica', 3);", "SEM005",
          "id es INT y recibe un texto.");
  invalid(S, "INSERT incorrecto", "Columna NOT NULL sin valor",
          "INSERT INTO cursos (id, creditos) VALUES (9, 3);", "SEM007",
          "nombre es NOT NULL y el INSERT no le asigna valor.");
  invalid(S, "INSERT incorrecto", "NULL en la PRIMARY KEY",
          "INSERT INTO cursos VALUES (NULL, 'Lógica', 3);", "SEM007",
          "Una PRIMARY KEY nunca puede ser NULL.");
  invalid(S, "INSERT incorrecto", "Texto más largo que VARCHAR(n)",
          "INSERT INTO carreras VALUES (9, 'Un nombre de carrera que supera los cuarenta caracteres');",
          "SEM008", "carreras.nombre es VARCHAR(40).");
  invalid(S, "INSERT incorrecto", "Fecha inexistente",
          "INSERT INTO estudiantes VALUES (9, 'Eva', 20, 1.0, TRUE, '2024-02-30', 1);", "SEM009",
          "El 30 de febrero no existe.");
  invalid(S, "DDL", "Tabla ya existente", "CREATE TABLE cursos (id INT);", "SEM010",
          "cursos ya fue creada.");
  invalid(S, "DDL", "Columna duplicada", "CREATE TABLE t (a INT, a FLOAT);", "SEM011",
          "Dos columnas con el mismo nombre.", false);
  invalid(S, "DDL", "Clave foránea a una columna que no es clave",
          "CREATE TABLE t (x VARCHAR(50) REFERENCES estudiantes(nombre));", "SEM012",
          "Una FK debe apuntar a una PRIMARY KEY o UNIQUE.");
  invalid(S, "Índice inexistente", "DROP INDEX de un índice que no existe",
          "DROP INDEX idx_fantasma;", "SEM013", "No hay ningún índice con ese nombre.");
  invalid(S, "Índice duplicado", "CREATE INDEX con un nombre ya usado",
          "CREATE INDEX idx_a ON estudiantes(edad); CREATE INDEX idx_a ON cursos(creditos);",
          "SEM014", "El segundo índice repite el nombre idx_a.");
  invalid(S, "Índice duplicado", "Columna que ya tiene índice",
          "CREATE INDEX idx_b ON estudiantes(id);", "SEM014",
          "estudiantes.id ya está indexada por pk_estudiantes.");
  invalid(S, "Subconsulta escalar", "Subconsulta que retorna dos columnas",
          "SELECT nombre FROM estudiantes WHERE edad = (SELECT edad, id FROM estudiantes);",
          "SEM015", "Una subconsulta escalar debe tener una sola columna.");
  invalid(S, "Agregación", "Columna no agrupada",
          "SELECT carrera_id, nombre, COUNT(*) FROM estudiantes GROUP BY carrera_id;", "SEM016",
          "nombre no está en GROUP BY ni dentro de una agregación.");
  invalid(S, "Agregación", "Agregación dentro de WHERE",
          "SELECT nombre FROM estudiantes WHERE COUNT(*) > 1;", "SEM016",
          "Las agregaciones se filtran con HAVING, no con WHERE.");
  invalid(S, "DDL", "Eliminar una tabla referenciada", "DROP TABLE carreras;", "SEM017",
          "estudiantes.carrera_id referencia a carreras.");
  invalid(S, "Condición", "WHERE que no es booleano",
          "SELECT nombre FROM estudiantes WHERE edad;", "SEM019",
          "WHERE necesita una condición BOOLEAN.");
  valid(S, "Ubicación", "Cada error indica sentencia, línea y columna",
        "SELECT nombre FROM estudiantes;\nSELECT apellido\nFROM estudiantes;",
        "SEM002 en la sentencia 2, línea 2, columna 8",
        "Los diagnósticos llevan la posición exacta del token que causa el error.",
        [](const Outcome &o) {
          const auto &d = o.main.semantic_diagnostics;
          return d.size() == 1 && d[0].statement == 2 && d[0].line == 2 && d[0].column == 8;
        });
  valid(S, "Script completo", "Un error semántico impide ejecutar cualquier sentencia",
        "CREATE TABLE nueva (id INT); SELECT nada FROM nueva;",
        "Se rechaza y la tabla nueva no se crea",
        "La semántica analiza todo el script antes de ejecutar: no hay cambios parciales.",
        [](const Outcome &o) {
          return hasCode(o.main, "SEM002") && !fs::exists(fs::path(o.database) / "nueva.csv");
        },
        false);
  valid(S, "Modo análisis", "Analizar sin ejecutar no modifica archivos",
        "CREATE TABLE borrador (id INT);", "Válido, pero no se crea borrador.csv",
        "El botón \"Analizar sin ejecutar\" llega hasta la semántica y se detiene.",
        [](const Outcome &o) {
          return o.main.ok && o.main.mode == "analyze" &&
                 !fs::exists(fs::path(o.database) / "borrador.csv");
        },
        false);

  // 4. CREATE, DROP e INSERT
  const std::string C = "4. CREATE, DROP e INSERT";
  valid(C, "CREATE TABLE", "El esquema se guarda en metadata.txt",
        "CREATE TABLE productos (id INT PRIMARY KEY, nombre VARCHAR(30) NOT NULL, precio FLOAT, "
        "disponible BOOLEAN, alta DATE);",
        "metadata.txt describe los tipos y restricciones; existen productos.csv y pk_productos.idx",
        "El catálogo guarda tipos y restricciones para la semántica y la ejecución.",
        [](const Outcome &o) {
          const fs::path db(o.database);
          const std::string meta = readFile(db / "metadata.txt");
          return o.main.ok && contains(meta, "TABLE productos") &&
                 contains(meta, "COLUMN nombre VARCHAR(30) NOT_NULL") &&
                 contains(meta, "COLUMN alta DATE") && fs::exists(db / "productos.csv") &&
                 fs::exists(db / "indexes" / "pk_productos.idx");
        },
        false);
  valid(C, "INSERT", "Varias filas con todos los tipos",
        "CREATE TABLE p (id INT, precio FLOAT, ok BOOLEAN, d DATE, t VARCHAR(10));\n"
        "INSERT INTO p VALUES (1, 2.5, TRUE, '2024-03-01', 'uno'), (2, 3, FALSE, NULL, NULL);\n"
        "SELECT * FROM p;",
        "(1, 2.5, TRUE, 2024-03-01, uno) (2, 3.0, FALSE, NULL, NULL)",
        "El INT 3 se promueve a FLOAT y NULL se guarda como campo vacío.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 2 && s.rows[1][1].toString() == "3.0" &&
                 s.rows[1][3].isNull() && s.rows[0][3].toString() == "2024-03-01";
        },
        false);
  valid(C, "INSERT", "Lista de columnas: las omitidas quedan en NULL",
        "INSERT INTO estudiantes (id, nombre) VALUES (10, 'Eva'); "
        "SELECT edad, promedio, carrera_id FROM estudiantes WHERE id = 10;",
        "(NULL, NULL, NULL)", "Las columnas sin NOT NULL aceptan quedar vacías.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 1 && s.rows[0][0].isNull() &&
                 s.rows[0][2].isNull();
        });
  valid(C, "CSV", "Texto con comillas, comas y vacío distinto de NULL",
        "CREATE TABLE notas (id INT, t VARCHAR(40));\n"
        "INSERT INTO notas VALUES (1, 'O''Brien, \"Jr\"'), (2, ''), (3, NULL);",
        "Al releer: O'Brien, \"Jr\" | (vacío) | NULL",
        "El CSV entrecomilla todo texto; un campo vacío sin comillas significa NULL.",
        [](const Outcome &o) {
          const auto &s = last(o.followup);
          return o.followup.ok && s.rows.size() == 3 &&
                 s.rows[0][0].toString() == "O'Brien, \"Jr\"" &&
                 s.rows[1][0].kind == sql::Value::Kind::Text && s.rows[1][0].text.empty() &&
                 s.rows[2][0].isNull();
        },
        false, "SELECT t FROM notas ORDER BY id;");
  valid(C, "DROP TABLE", "Borra la tabla, su CSV y sus índices",
        "DROP TABLE inscripciones;", "inscripciones.csv y pk_inscripciones.idx ya no existen",
        "DROP TABLE elimina datos, índices y la entrada del catálogo.",
        [](const Outcome &o) {
          const fs::path db(o.database);
          return o.main.ok && !fs::exists(db / "inscripciones.csv") &&
                 !fs::exists(db / "indexes" / "pk_inscripciones.idx") &&
                 !contains(readFile(db / "metadata.txt"), "TABLE inscripciones");
        });
  valid(C, "IF EXISTS", "IF NOT EXISTS / IF EXISTS no fallan",
        "CREATE TABLE IF NOT EXISTS cursos (id INT); DROP TABLE IF EXISTS fantasma;",
        "Ambas sentencias terminan sin cambios",
        "Las opciones IF evitan el error cuando el objeto ya existe o no existe.",
        [](const Outcome &o) { return o.main.ok; });
  invalid(C, "PRIMARY KEY", "Clave primaria duplicada",
          "INSERT INTO cursos VALUES (1, 'Repetido', 3);", "EXE001",
          "Ya existe un curso con id 1; se detecta con el índice pk_cursos.");
  valid(C, "Atomicidad", "Un INSERT que falla no deja filas a medias",
        "INSERT INTO cursos VALUES (7, 'Nuevo', 3), (1, 'Repetido', 3);",
        "Falla con EXE001 y el curso 7 tampoco se guarda",
        "Cada sentencia trabaja sobre una copia y solo la guarda si termina bien.",
        [](const Outcome &o) {
          return hasCode(o.main, "EXE001") && last(o.followup).rows.size() == 1 &&
                 last(o.followup).rows[0][0].toString() == "0";
        },
        true, "SELECT COUNT(*) FROM cursos WHERE id = 7;");
  invalid(C, "UNIQUE", "Valor UNIQUE repetido",
          "INSERT INTO carreras VALUES (9, 'Computación');", "EXE002",
          "carreras.nombre es UNIQUE y 'Computación' ya existe.");
  invalid(C, "FOREIGN KEY", "Clave foránea sin registro referenciado",
          "INSERT INTO estudiantes VALUES (9, 'Eva', 20, 80.0, TRUE, '2024-01-01', 99);", "EXE003",
          "No existe la carrera 99.");

  // 5. SELECT, WHERE, UPDATE y DELETE
  const std::string W = "5. SELECT, WHERE, UPDATE y DELETE";
  valid(W, "WHERE", "Ejemplo del enunciado: edad >= 18",
        "SELECT nombre FROM estudiantes WHERE edad >= 18;", "Ana, Sofía, Pedro, María",
        "Solo se muestran los nombres que cumplen la condición.",
        [](const Outcome &o) {
          return o.main.ok && column(last(o.main)) == list({"Ana", "Sofía", "Pedro", "María"});
        });
  valid(W, "AND / OR", "Condiciones combinadas",
        "SELECT nombre FROM estudiantes WHERE (edad < 18 OR edad > 21) AND activo = TRUE;",
        "Luis", "Sofía tiene 22 pero no está activa; Luis tiene 17 y sí.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Luis"}); });
  valid(W, "ORDER BY", "Orden descendente, LIMIT y alias",
        "SELECT nombre, promedio AS p FROM estudiantes WHERE promedio IS NOT NULL "
        "ORDER BY p DESC LIMIT 2;",
        "Ana, María", "ORDER BY puede usar el alias definido en el SELECT.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Ana", "María"}); });
  valid(W, "DISTINCT", "Elimina filas repetidas",
        "SELECT DISTINCT curso_id FROM inscripciones ORDER BY curso_id;", "1, 2, 3",
        "El curso 3 aparece tres veces en inscripciones pero una sola en el resultado.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"1", "2", "3"}); });
  valid(W, "NULL", "Lógica de tres valores: NULL no cumple comparaciones",
        "SELECT COUNT(*) FROM estudiantes WHERE promedio > 0 OR promedio <= 0;", "4",
        "Pedro tiene promedio NULL: ambas comparaciones son desconocidas y queda fuera.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"4"}); });
  valid(W, "DATE", "Comparación de fechas",
        "SELECT nombre FROM estudiantes WHERE ingreso < '2023-06-01' ORDER BY ingreso;",
        "Sofía, Ana", "El literal se convierte a DATE porque se compara con una columna DATE.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Sofía", "Ana"}); });
  valid(W, "UPDATE", "SET usa los valores anteriores de la fila",
        "UPDATE cursos SET creditos = id, id = creditos + 10 WHERE id = 4; "
        "SELECT id, creditos FROM cursos WHERE nombre = 'Física';",
        "(14, 4)", "Ambas asignaciones leen la fila original, como exige SQL.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 1 && s.rows[0][0].toString() == "14" &&
                 s.rows[0][1].toString() == "4";
        });
  valid(W, "UPDATE", "Cambiar todas las PRIMARY KEY sin falso duplicado",
        "CREATE TABLE t (id INT PRIMARY KEY); INSERT INTO t VALUES (1), (2), (3);\n"
        "UPDATE t SET id = id + 1; SELECT id FROM t;",
        "2, 3, 4",
        "Primero se retiran las claves viejas del índice y luego se validan las nuevas.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"2", "3", "4"}); },
        false);
  valid(W, "DELETE", "Borra solo las filas que cumplen WHERE",
        "DELETE FROM inscripciones WHERE nota < 70; SELECT COUNT(*) FROM inscripciones;", "5",
        "Solo la inscripción con nota 61 cumple; la de nota NULL no.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"5"}); });
  invalid(W, "FOREIGN KEY", "DELETE de un registro referenciado",
          "DELETE FROM cursos WHERE id = 1;", "EXE004",
          "Hay inscripciones que apuntan al curso 1 (política RESTRICT).");
  invalid(W, "Ejecución", "División entre cero", "SELECT edad / 0 FROM estudiantes;", "EXE006",
          "Solo se descubre al evaluar con datos reales.");
  valid(W, "Persistencia", "Los datos sobreviven entre ejecuciones",
        "INSERT INTO cursos VALUES (8, 'Redes', 4);", "Una segunda ejecución encuentra Redes",
        "La segunda corrida abre la base desde los archivos de texto.",
        [](const Outcome &o) { return o.followup.ok && column(last(o.followup)) == list({"Redes"}); },
        true, "SELECT nombre FROM cursos WHERE id = 8;");

  // 6. JOIN, agregaciones y GROUP BY
  const std::string J = "6. JOIN, agregaciones y GROUP BY";
  valid(J, "INNER JOIN", "Une estudiantes con su carrera",
        "SELECT e.nombre, c.nombre FROM estudiantes e INNER JOIN carreras c "
        "ON e.carrera_id = c.id ORDER BY e.id;",
        "4 filas; Pedro queda fuera por no tener carrera",
        "INNER JOIN solo conserva combinaciones que cumplen ON.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 4 && column(s, 1)[0] == "Computación";
        });
  valid(J, "INNER JOIN", "Tres tablas",
        "SELECT e.nombre, cu.nombre FROM inscripciones i JOIN estudiantes e "
        "ON i.estudiante_id = e.id JOIN cursos cu ON i.curso_id = cu.id WHERE cu.id = 3 "
        "ORDER BY e.nombre;",
        "Ana, Luis, María", "Cada JOIN amplía la fila combinada con otra tabla.",
        [](const Outcome &o) {
          return o.main.ok && column(last(o.main)) == list({"Ana", "Luis", "María"});
        });
  valid(J, "Agregaciones", "COUNT, SUM, AVG, MIN y MAX",
        "SELECT COUNT(*), SUM(edad), AVG(edad), MIN(nombre), MAX(ingreso) FROM estudiantes;",
        "(5, 96, 19.2, Ana, 2024-01-10)", "Las cinco funciones sobre todo el conjunto.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 1 && s.rows[0][0].toString() == "5" &&
                 s.rows[0][1].toString() == "96" && s.rows[0][2].toString() == "19.2" &&
                 s.rows[0][3].toString() == "Ana" && s.rows[0][4].toString() == "2024-01-10";
        });
  valid(J, "Agregaciones", "COUNT(columna) ignora NULL y COUNT(DISTINCT)",
        "SELECT COUNT(promedio), COUNT(DISTINCT ingreso) FROM estudiantes;", "(4, 4)",
        "Pedro no tiene promedio; dos estudiantes comparten fecha de ingreso.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows[0][0].toString() == "4" && s.rows[0][1].toString() == "4";
        });
  valid(J, "GROUP BY", "Agrupa y filtra grupos con HAVING",
        "SELECT curso_id, COUNT(*) AS n FROM inscripciones GROUP BY curso_id "
        "HAVING COUNT(*) >= 2 ORDER BY n DESC;",
        "(3, 3) (1, 2)", "El curso 2 tiene una sola inscripción y no pasa HAVING.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && column(s) == list({"3", "1"}) && column(s, 1) == list({"3", "2"});
        });
  valid(J, "GROUP BY", "JOIN + GROUP BY + AVG",
        "SELECT c.nombre, AVG(e.promedio) AS prom FROM estudiantes e JOIN carreras c "
        "ON e.carrera_id = c.id GROUP BY c.nombre ORDER BY prom DESC;",
        "Computación 88.25, María 88, Mecatrónica 78",
        "Primero se combinan las tablas y luego se agrupa.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 3 && s.rows[0][1].toString() == "88.25";
        });
  valid(J, "Agregaciones", "Agregar una tabla vacía",
        "CREATE TABLE vacia (x INT); SELECT COUNT(*), SUM(x), MAX(x) FROM vacia;",
        "(0, NULL, NULL)", "Sin filas COUNT es 0 y las demás funciones devuelven NULL.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && s.rows.size() == 1 && s.rows[0][0].toString() == "0" &&
                 s.rows[0][1].isNull();
        },
        false);

  // 7. Índices y subconsultas
  const std::string I = "7. Índices";
  valid(I, "CREATE INDEX", "Crea el archivo .idx ordenado",
        "CREATE INDEX idx_edad ON estudiantes(edad);",
        "indexes/idx_edad.idx contiene 17 → rowid 2 como primera clave",
        "El índice guarda valor → rowids, ordenado por valor.",
        [](const Outcome &o) {
          const std::string text = readFile(fs::path(o.database) / "indexes" / "idx_edad.idx");
          return o.main.ok && contains(text, "17,2\n") && contains(readFile(fs::path(o.database) / "metadata.txt"), "INDEX idx_edad ON estudiantes.edad");
        });
  valid(I, "Uso del índice", "WHERE sobre columna indexada usa Index Scan",
        "CREATE INDEX idx_edad ON estudiantes(edad); SELECT nombre FROM estudiantes WHERE edad > 19;",
        "Plan con Index Scan; resultado Ana, Sofía",
        "El intérprete consulta el índice en lugar de recorrer todos los registros.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && planContains(s, "Index Scan en estudiantes usando idx_edad") &&
                 column(s) == list({"Ana", "Sofía"});
        });
  valid(I, "Uso del índice", "Sin índice se hace Full Scan",
        "SELECT nombre FROM estudiantes WHERE edad > 19;", "Plan con Full Scan",
        "edad no está indexada en la base de ejemplo.",
        [](const Outcome &o) { return o.main.ok && planContains(last(o.main), "Full Scan"); });
  valid(I, "Uso del índice", "El índice de la PRIMARY KEY acelera el JOIN",
        "SELECT e.nombre FROM inscripciones i JOIN estudiantes e ON i.estudiante_id = e.id;",
        "Plan con Index Nested Loop sobre pk_estudiantes",
        "Por cada inscripción se busca el estudiante en el índice.",
        [](const Outcome &o) {
          return o.main.ok && planContains(last(o.main), "Index Nested Loop con pk_estudiantes");
        });
  valid(I, "Mantenimiento", "INSERT agrega la clave al índice",
        "CREATE INDEX idx_edad ON estudiantes(edad);\n"
        "INSERT INTO estudiantes VALUES (9, 'Eva', 40, 70.0, TRUE, '2025-01-01', 2);\n"
        "SELECT nombre FROM estudiantes WHERE edad = 40;",
        "Eva (encontrada por el índice)", "El .idx se actualiza en la misma sentencia.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && column(s) == list({"Eva"}) && planContains(s, "Index Scan");
        });
  valid(I, "Mantenimiento", "UPDATE mueve la clave dentro del índice",
        "CREATE INDEX idx_edad ON estudiantes(edad);\n"
        "UPDATE estudiantes SET edad = 50 WHERE id = 1;\n"
        "SELECT COUNT(*) FROM estudiantes WHERE edad = 20;",
        "0 (la clave 20 ya no apunta a Ana)", "Se retira la clave vieja y se agrega la nueva.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"0"}); });
  valid(I, "Mantenimiento", "DELETE retira la clave del índice",
        "CREATE INDEX idx_nota ON inscripciones(nota);\n"
        "DELETE FROM inscripciones WHERE id = 3;",
        "indexes/idx_nota.idx ya no contiene 61.0",
        "El archivo del índice se reescribe sin la clave eliminada.",
        [](const Outcome &o) {
          const std::string text = readFile(fs::path(o.database) / "indexes" / "idx_nota.idx");
          return o.main.ok && !contains(text, "61.0") && contains(text, "90.0");
        });
  valid(I, "DROP INDEX", "Elimina el archivo del índice",
        "CREATE INDEX idx_edad ON estudiantes(edad); DROP INDEX idx_edad;",
        "idx_edad.idx no existe", "DROP INDEX borra el archivo y la entrada del catálogo.",
        [](const Outcome &o) {
          return o.main.ok && !fs::exists(fs::path(o.database) / "indexes" / "idx_edad.idx");
        });
  invalid(I, "Índice UNIQUE", "Índice único sobre valores repetidos",
          "CREATE UNIQUE INDEX idx_fecha ON estudiantes(ingreso);", "EXE002",
          "Luis y Pedro comparten la fecha de ingreso.");
  invalid(I, "Índice automático", "No se puede borrar el índice de la PRIMARY KEY",
          "DROP INDEX pk_cursos;", "SEM012", "Lo mantiene la restricción PRIMARY KEY.");

  const std::string Q = "8. Subconsultas";
  valid(Q, "IN", "Ejemplo del enunciado",
        "SELECT nombre FROM estudiantes WHERE id IN (SELECT estudiante_id FROM inscripciones "
        "WHERE curso_id = 3);",
        "Ana, Luis, María", "La subconsulta devuelve 1, 2 y 5.",
        [](const Outcome &o) {
          return o.main.ok && column(last(o.main)) == list({"Ana", "Luis", "María"});
        });
  valid(Q, "NOT IN", "Estudiantes sin Cálculo",
        "SELECT nombre FROM estudiantes WHERE id NOT IN (SELECT estudiante_id FROM "
        "inscripciones WHERE curso_id = 3);",
        "Sofía, Pedro", "Es el complemento del caso anterior.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Sofía", "Pedro"}); });
  valid(Q, "EXISTS", "Subconsulta correlacionada",
        "SELECT e.nombre FROM estudiantes e WHERE EXISTS (SELECT 1 FROM inscripciones i "
        "WHERE i.estudiante_id = e.id AND i.nota > 85);",
        "Ana", "La subconsulta usa e.id de la fila externa: se evalúa por cada estudiante.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && column(s) == list({"Ana"}) && planContains(s, "correlacionada, se ejecutó 5 veces");
        });
  valid(Q, "NOT EXISTS", "Cursos sin inscripciones",
        "SELECT c.nombre FROM cursos c WHERE NOT EXISTS (SELECT 1 FROM inscripciones i "
        "WHERE i.curso_id = c.id);",
        "Física", "Física es el único curso sin inscripciones.",
        [](const Outcome &o) { return o.main.ok && column(last(o.main)) == list({"Física"}); });
  valid(Q, "Escalar", "Subconsulta escalar en una comparación",
        "SELECT nombre FROM estudiantes WHERE promedio > (SELECT AVG(promedio) FROM estudiantes);",
        "Ana, María", "El promedio general es 85.625.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && column(s) == list({"Ana", "María"}) &&
                 planContains(s, "no correlacionada, se ejecutó 1 vez");
        });
  invalid(Q, "Escalar", "Subconsulta escalar con varios registros",
          "SELECT nombre FROM estudiantes WHERE edad = (SELECT edad FROM estudiantes "
          "WHERE activo = TRUE);",
          "EXE005", "Hay cuatro estudiantes activos; una subconsulta escalar admite uno.");
  valid(Q, "Índice + subconsulta", "IN (SELECT ...) busca cada valor en el índice",
        "SELECT nombre FROM estudiantes WHERE id IN (SELECT estudiante_id FROM inscripciones "
        "WHERE nota >= 88);",
        "Plan con Index Scan sobre pk_estudiantes; resultado Ana",
        "Los valores de la subconsulta se buscan en el índice de la PRIMARY KEY.",
        [](const Outcome &o) {
          const auto &s = last(o.main);
          return o.main.ok && column(s) == list({"Ana"}) &&
                 planContains(s, "Index Scan en estudiantes usando pk_estudiantes");
        });

  // 9. Persistencia en archivos
  const std::string P = "9. Almacenamiento en archivos";
  valid(P, "Formato CSV", "La tabla se guarda con encabezado y __rowid",
        "SELECT 1;", "carreras.csv empieza con __rowid,id,nombre",
        "La primera columna es el identificador interno que usan los índices.",
        [](const Outcome &o) {
          const std::string text = readFile(fs::path(o.database) / "carreras.csv");
          return text.rfind("__rowid,id,nombre\n1,1,\"Computación\"\n", 0) == 0;
        });
  valid(P, "Catálogo", "metadata.txt guarda las claves foráneas e índices automáticos",
        "SELECT 1;", "REFERENCES carreras.id e INDEX pk_carreras ... UNIQUE AUTO",
        "El catálogo describe las restricciones que la semántica valida.",
        [](const Outcome &o) {
          const std::string text = readFile(fs::path(o.database) / "metadata.txt");
          return contains(text, "COLUMN carrera_id INT REFERENCES carreras.id") &&
                 contains(text, "INDEX pk_carreras ON carreras.id UNIQUE AUTO") &&
                 contains(text, "INDEX uq_carreras_nombre ON carreras.nombre UNIQUE AUTO");
        });
  valid(P, "Reapertura", "Una base existente se lee desde disco",
        "UPDATE estudiantes SET nombre = 'Ana María' WHERE id = 1;",
        "La siguiente ejecución lee Ana María desde estudiantes.csv",
        "Cada ejecución del CLI arranca vacía en memoria y carga los archivos.",
        [](const Outcome &o) {
          return o.followup.ok && column(last(o.followup)) == list({"Ana María"});
        },
        true, "SELECT nombre FROM estudiantes WHERE id = 1;");
  return tests;
}

std::vector<TestRun> runTests() {
  const fs::path root = fs::temp_directory_path() / "sql_interpreter_tests";
  std::error_code ignored;
  fs::remove_all(root, ignored);
  std::vector<TestRun> runs;
  size_t number = 0;
  for (auto &test : cases()) {
    const fs::path database = root / ("caso_" + std::to_string(++number));
    fs::create_directories(database, ignored);
    sql::RunOptions options;
    options.database = database.string();
    if (test.seed) {
      const Result seeded = sql::runScript(SEED, options);
      if (!seeded.ok) {
        std::cerr << "La base de ejemplo no se pudo cargar: " << describe(seeded) << '\n';
        std::exit(EXIT_FAILURE);
      }
    }
    Outcome outcome;
    outcome.database = database.string();
    options.execute = test.name.find("sin ejecutar") == std::string::npos;
    outcome.main = sql::runScript(test.source, options);
    options.execute = true;
    if (!test.followup.empty())
      outcome.followup = sql::runScript(test.followup, options);

    TestRun run;
    run.test = test;
    run.passed = test.verify(outcome);
    run.codes = codes(outcome.main);
    run.actual = describe(outcome.main);
    if (!test.followup.empty())
      run.actual += " | Segunda ejecución: " + describe(outcome.followup);
    runs.push_back(std::move(run));
  }
  fs::remove_all(root, ignored);
  return runs;
}

std::string stringArray(const std::vector<std::string> &values) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < values.size(); ++index)
    output << (index ? "," : "") << sql::quoteJson(values[index]);
  output << ']';
  return output.str();
}

std::string simpleExplanation(const TestRun &run) {
  if (run.test.kind == "Error esperado") {
    std::string message = "Este SQL es incorrecto a propósito. " + run.test.explanation;
    if (run.passed)
      message += " El intérprete lo rechazó con " + join(run.codes, ", ") +
                 ". Por eso la prueba aprueba: rechazarlo es el comportamiento correcto.";
    else
      message += " La prueba falla porque no se produjo el código de error esperado.";
    return message;
  }
  std::string message = run.test.explanation;
  if (run.test.seed)
    message += " (Se ejecuta sobre la base de ejemplo: carreras, estudiantes, cursos e inscripciones.)";
  message += run.passed ? " El resultado coincide con lo esperado."
                        : " El resultado obtenido no coincide con lo esperado.";
  return message;
}

std::string reportJson(const std::vector<TestRun> &runs) {
  size_t passed = 0, valid_cases = 0, expected_error_cases = 0;
  std::vector<std::string> category_order;
  std::map<std::string, std::vector<const TestRun *>> categories;
  for (const auto &run : runs) {
    passed += run.passed ? 1 : 0;
    valid_cases += run.test.kind == "Caso válido" ? 1 : 0;
    expected_error_cases += run.test.kind == "Error esperado" ? 1 : 0;
    if (!categories.count(run.test.category))
      category_order.push_back(run.test.category);
    categories[run.test.category].push_back(&run);
  }

  std::ostringstream output;
  output << "{\"ok\":" << (passed == runs.size() ? "true" : "false")
         << ",\"summary\":{\"total\":" << runs.size() << ",\"passed\":" << passed
         << ",\"failed\":" << (runs.size() - passed) << ",\"valid_cases\":" << valid_cases
         << ",\"expected_error_cases\":" << expected_error_cases << "},\"categories\":[";
  for (size_t category_index = 0; category_index < category_order.size(); ++category_index) {
    if (category_index)
      output << ',';
    const auto &name = category_order[category_index];
    const auto &items = categories[name];
    size_t category_passed = 0;
    for (const auto *item : items)
      category_passed += item->passed ? 1 : 0;
    output << "{\"name\":" << sql::quoteJson(name) << ",\"total\":" << items.size()
           << ",\"passed\":" << category_passed << ",\"tests\":[";
    for (size_t test_index = 0; test_index < items.size(); ++test_index) {
      if (test_index)
        output << ',';
      const auto &run = *items[test_index];
      output << "{\"name\":" << sql::quoteJson(run.test.name)
             << ",\"rule\":" << sql::quoteJson(run.test.rule)
             << ",\"kind\":" << sql::quoteJson(run.test.kind)
             << ",\"passed\":" << (run.passed ? "true" : "false")
             << ",\"source\":" << sql::quoteJson(run.test.source)
             << ",\"explanation\":" << sql::quoteJson(simpleExplanation(run))
             << ",\"expected\":" << sql::quoteJson(run.test.expected)
             << ",\"actual\":" << sql::quoteJson(run.actual)
             << ",\"codes\":" << stringArray(run.codes) << '}';
    }
    output << "]}";
  }
  output << "]}";
  return output.str();
}

} // namespace

int main(int argc, char **argv) {
  const auto runs = runTests();
  size_t passed = 0;
  for (const auto &run : runs)
    passed += run.passed ? 1 : 0;

  if (argc > 1 && std::string(argv[1]) == "--json") {
    std::cout << reportJson(runs) << '\n';
  } else {
    for (const auto &run : runs) {
      if (!run.passed) {
        std::cerr << "FALLO [" << run.test.category << " / " << run.test.rule << "] "
                  << run.test.name << '\n'
                  << "  Esperado: " << run.test.expected << '\n'
                  << "  Obtenido: " << run.actual << '\n';
      }
    }
    std::cout << "sql_tests: " << passed << '/' << runs.size() << " pruebas aprobadas\n";
  }
  return passed == runs.size() ? EXIT_SUCCESS : EXIT_FAILURE;
}
