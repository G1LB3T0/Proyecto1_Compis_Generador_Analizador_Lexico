-- Entrega 4: INSERT. Cada fila se valida (tipos, NOT NULL, PRIMARY KEY,
-- UNIQUE y FOREIGN KEY) y se guarda en el .csv de su tabla.
INSERT INTO carreras VALUES
  (1, 'Ingeniería en Computación'),
  (2, 'Ingeniería Mecatrónica'),
  (3, 'Licenciatura en Matemática');

INSERT INTO estudiantes (id, nombre, edad, promedio, activo, ingreso, carrera_id) VALUES
  (1, 'Ana López',      20, 91.5,  TRUE,  '2023-01-15', 1),
  (2, 'Luis Pérez',     17, 78.0,  TRUE,  '2024-01-10', 2),
  (3, 'Sofía Castillo', 22, 85.25, FALSE, '2021-07-01', 1),
  (4, 'Pedro Méndez',   18, NULL,  TRUE,  '2024-01-10', NULL),
  (5, 'María García',   19, 88.0,  TRUE,  '2023-07-03', 3),
  (6, 'Jorge Ruiz',     25, 69.5,  FALSE, '2019-01-14', 2),
  (7, 'Lucía Herrera',  21, 95.0,  TRUE,  '2022-01-17', 1),
  (8, 'Diego Morales',  16, 72.0,  TRUE,  '2025-01-13', 3);

INSERT INTO cursos VALUES
  (1, 'Compiladores', 5),
  (2, 'Bases de Datos', 4),
  (3, 'Cálculo', 4),
  (4, 'Robótica', 3),
  (5, 'Álgebra Lineal', 4),
  (6, 'Física I', 4);

INSERT INTO inscripciones VALUES
  (1,  1, 1, 90.0, '2025-07-01'),
  (2,  1, 2, 94.0, '2025-07-01'),
  (3,  1, 3, 88.5, '2025-01-10'),
  (4,  2, 4, 75.0, '2025-07-02'),
  (5,  2, 3, 61.0, '2025-01-12'),
  (6,  3, 1, 82.0, '2024-07-01'),
  (7,  3, 3, 79.0, '2024-01-08'),
  (8,  5, 5, 91.0, '2025-01-09'),
  (9,  5, 3, 85.0, '2025-01-09'),
  (10, 6, 4, 58.0, '2023-07-03'),
  (11, 7, 1, 97.0, '2025-07-01'),
  (12, 7, 2, 96.5, '2025-07-01'),
  (13, 8, 5, NULL, '2025-07-04');
