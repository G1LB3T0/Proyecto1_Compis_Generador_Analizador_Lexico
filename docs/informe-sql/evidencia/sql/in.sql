SELECT nombre
FROM estudiantes
WHERE id IN (SELECT estudiante_id
             FROM inscripciones WHERE curso_id = 3)
ORDER BY nombre;
