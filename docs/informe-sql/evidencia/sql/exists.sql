SELECT e.nombre
FROM estudiantes e
WHERE EXISTS (SELECT 1 FROM inscripciones i
              WHERE i.estudiante_id = e.id AND i.nota >= 95);
