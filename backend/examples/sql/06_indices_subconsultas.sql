-- Entrega 7: índices y subconsultas.

-- Cada índice se guarda en database/indexes/<nombre>.idx.
DROP INDEX IF EXISTS idx_estudiantes_edad;
DROP INDEX IF EXISTS idx_inscripciones_curso;
CREATE INDEX idx_estudiantes_edad ON estudiantes(edad);
CREATE INDEX idx_inscripciones_curso ON inscripciones(curso_id);

-- WHERE sobre una columna indexada: el plan muestra "Index Scan".
SELECT nombre, edad FROM estudiantes WHERE edad >= 21;
SELECT nombre, edad FROM estudiantes WHERE edad IN (16, 25);

-- Ejemplo del enunciado: IN con subconsulta.
SELECT nombre
FROM estudiantes
WHERE id IN (SELECT estudiante_id
             FROM inscripciones
             WHERE curso_id = 3);

-- NOT IN: estudiantes que no llevan Compiladores.
SELECT nombre FROM estudiantes
WHERE id NOT IN (SELECT estudiante_id FROM inscripciones WHERE curso_id = 1)
ORDER BY nombre;

-- EXISTS correlacionado: se evalúa por cada estudiante.
SELECT e.nombre FROM estudiantes e
WHERE EXISTS (SELECT 1 FROM inscripciones i
              WHERE i.estudiante_id = e.id AND i.nota >= 95);

-- NOT EXISTS: cursos sin inscripciones.
SELECT cu.nombre FROM cursos cu
WHERE NOT EXISTS (SELECT 1 FROM inscripciones i WHERE i.curso_id = cu.id);

-- Subconsulta escalar en una comparación.
SELECT nombre, promedio FROM estudiantes
WHERE promedio > (SELECT AVG(promedio) FROM estudiantes)
ORDER BY promedio DESC;

-- El índice se actualiza con INSERT, UPDATE y DELETE.
INSERT INTO estudiantes VALUES (9, 'Elena Ríos', 21, 80.0, TRUE, '2025-07-01', 1);
UPDATE estudiantes SET edad = 30 WHERE nombre = 'Jorge Ruiz';
SELECT nombre, edad FROM estudiantes WHERE edad >= 21 ORDER BY edad;
DELETE FROM estudiantes WHERE id = 9;
