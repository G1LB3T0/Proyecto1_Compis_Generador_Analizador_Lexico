-- Errores semánticos: se detectan TODOS antes de ejecutar y ningún archivo
-- se modifica. Ejecutar después de 01 y 02.
SELECT nombre FROM alumnos;                                  -- tabla inexistente
SELECT apellido FROM estudiantes;                            -- columna inexistente
SELECT nombre FROM estudiantes WHERE edad = 'veinte';        -- tipos incompatibles
INSERT INTO estudiantes VALUES (50, 'Eva');                  -- cantidad de valores
INSERT INTO estudiantes (id, edad) VALUES (51, 20);          -- falta columna NOT NULL
INSERT INTO cursos VALUES (6, 'Nombre de curso demasiado largo para caber en sesenta caracteres', 4);
INSERT INTO estudiantes VALUES (52, 'Eva', 20, 1.0, TRUE, '2024-02-30', 1); -- fecha inválida
SELECT nombre FROM estudiantes e JOIN carreras c ON e.carrera_id = c.id;    -- referencia ambigua
SELECT x.nombre FROM estudiantes e;                          -- alias inválido
CREATE INDEX pk_cursos ON cursos(nombre);                    -- índice duplicado
DROP INDEX idx_que_no_existe;                                -- índice inexistente
SELECT nombre FROM estudiantes WHERE edad = (SELECT edad, id FROM estudiantes); -- 2 columnas
SELECT carrera_id, nombre, COUNT(*) FROM estudiantes GROUP BY carrera_id;       -- no agrupada
DROP TABLE carreras;                                         -- referenciada por FK
