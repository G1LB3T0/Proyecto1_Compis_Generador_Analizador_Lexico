SELECT c.nombre AS curso, COUNT(*) AS inscritos,
       MAX(i.nota) AS nota_maxima
FROM cursos c INNER JOIN inscripciones i ON c.id = i.curso_id
GROUP BY c.nombre
HAVING COUNT(*) >= 2
ORDER BY inscritos DESC, curso;
