-- Errores que solo se conocen con los datos. La ejecución se detiene en la
-- primera sentencia que falla y esa sentencia no deja cambios en disco.
-- Prueba descomentando una línea a la vez.

-- Clave primaria duplicada (detectada con el índice pk_estudiantes):
INSERT INTO estudiantes VALUES (1, 'Copia de Ana', 20, 90.0, TRUE, '2023-01-15', 1);

-- Clave foránea sin registro referenciado:
-- INSERT INTO estudiantes VALUES (60, 'Sin carrera', 20, 90.0, TRUE, '2023-01-15', 99);

-- Registro referenciado por otra tabla (RESTRICT):
-- DELETE FROM cursos WHERE id = 1;

-- Subconsulta escalar que retorna más de un registro:
-- SELECT nombre FROM estudiantes WHERE edad = (SELECT edad FROM estudiantes WHERE activo = TRUE);

-- División entre cero:
-- SELECT nombre, edad / 0 FROM estudiantes;
