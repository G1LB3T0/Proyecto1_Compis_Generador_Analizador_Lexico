-- Entrega 4: CREATE / DROP. Crea el esquema universitario del enunciado.
-- Se puede ejecutar varias veces: primero elimina las tablas si existen
-- (en orden inverso a sus claves foráneas).
DROP TABLE IF EXISTS inscripciones;
DROP TABLE IF EXISTS cursos;
DROP TABLE IF EXISTS estudiantes;
DROP TABLE IF EXISTS carreras;

CREATE TABLE carreras (
  id     INT PRIMARY KEY,
  nombre VARCHAR(40) NOT NULL UNIQUE
);

CREATE TABLE estudiantes (
  id         INT PRIMARY KEY,
  nombre     VARCHAR(50) NOT NULL,
  edad       INT,
  promedio   FLOAT,
  activo     BOOLEAN,
  ingreso    DATE,
  carrera_id INT REFERENCES carreras(id)
);

CREATE TABLE cursos (
  id       INT,
  nombre   VARCHAR(60) NOT NULL,
  creditos INT NOT NULL,
  PRIMARY KEY (id)
);

CREATE TABLE inscripciones (
  id            INT PRIMARY KEY,
  estudiante_id INT NOT NULL,
  curso_id      INT NOT NULL,
  nota          FLOAT,
  fecha         DATE,
  FOREIGN KEY (estudiante_id) REFERENCES estudiantes(id),
  FOREIGN KEY (curso_id) REFERENCES cursos(id)
);
