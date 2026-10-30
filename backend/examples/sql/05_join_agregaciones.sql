-- Entrega 6: INNER JOIN, COUNT/SUM/AVG/MIN/MAX, GROUP BY y HAVING.

-- Estudiantes con el nombre de su carrera.
SELECT e.nombre, c.nombre AS carrera
FROM estudiantes e
INNER JOIN carreras c ON e.carrera_id = c.id
ORDER BY carrera, e.nombre;

-- Tres tablas: qué curso lleva cada estudiante y su nota.
SELECT e.nombre AS estudiante, cu.nombre AS curso, i.nota
FROM inscripciones i
JOIN estudiantes e ON i.estudiante_id = e.id
JOIN cursos cu ON i.curso_id = cu.id
WHERE i.nota IS NOT NULL
ORDER BY curso, i.nota DESC;

-- Agregaciones globales.
SELECT COUNT(*) AS estudiantes, AVG(promedio) AS promedio_general,
       MIN(edad) AS menor, MAX(edad) AS mayor, SUM(edad) AS suma_edades
FROM estudiantes;

-- GROUP BY + HAVING: carreras con más de un estudiante.
SELECT c.nombre AS carrera, COUNT(*) AS inscritos, AVG(e.promedio) AS promedio
FROM estudiantes e
INNER JOIN carreras c ON e.carrera_id = c.id
GROUP BY c.nombre
HAVING COUNT(*) > 1
ORDER BY inscritos DESC;

-- Nota promedio por curso.
SELECT cu.nombre, cu.creditos, COUNT(i.nota) AS evaluados, AVG(i.nota) AS nota_media
FROM cursos cu
JOIN inscripciones i ON i.curso_id = cu.id
GROUP BY cu.nombre, cu.creditos
ORDER BY nota_media DESC;
