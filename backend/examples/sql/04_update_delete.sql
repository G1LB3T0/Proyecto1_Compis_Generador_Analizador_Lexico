-- Entrega 5: UPDATE y DELETE. Los índices se mantienen al día.

-- Cada valor nuevo se calcula con los valores ANTERIORES de la fila.
UPDATE estudiantes SET edad = edad + 1, activo = TRUE WHERE id = 2;

UPDATE inscripciones SET nota = 100.0 WHERE estudiante_id = 7 AND curso_id = 2;

-- Solo se borra lo que nadie referencia (las FK usan política RESTRICT).
DELETE FROM inscripciones WHERE nota < 60;

SELECT id, estudiante_id, curso_id, nota FROM inscripciones ORDER BY id;
SELECT id, nombre, edad, activo FROM estudiantes WHERE id = 2;
