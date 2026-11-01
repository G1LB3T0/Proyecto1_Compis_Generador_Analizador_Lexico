SELECT nombre, promedio
FROM estudiantes
WHERE promedio > (SELECT AVG(promedio) FROM estudiantes)
ORDER BY promedio DESC;
