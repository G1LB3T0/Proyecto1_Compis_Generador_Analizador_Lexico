-- Entrega 5: SELECT, WHERE, AND, OR, ORDER BY.

-- Ejemplo del enunciado: nombres de los estudiantes mayores de edad.
SELECT nombre
FROM estudiantes
WHERE edad >= 18;

-- Varias condiciones con AND / OR y orden descendente.
SELECT nombre, edad, promedio
FROM estudiantes
WHERE (edad >= 18 AND activo = TRUE) OR promedio > 90
ORDER BY promedio DESC;

-- Fechas (DATE) comparadas con un literal con formato YYYY-MM-DD.
SELECT nombre, ingreso
FROM estudiantes
WHERE ingreso >= '2023-01-01'
ORDER BY ingreso, nombre;

-- NULL: IS NULL / IS NOT NULL.
SELECT nombre FROM estudiantes WHERE carrera_id IS NULL;

-- Expresiones aritméticas, alias, DISTINCT y LIMIT.
SELECT nombre, promedio / 10 AS sobre_diez
FROM estudiantes
WHERE promedio IS NOT NULL
ORDER BY sobre_diez DESC
LIMIT 3;

SELECT DISTINCT carrera_id FROM estudiantes ORDER BY 1;
