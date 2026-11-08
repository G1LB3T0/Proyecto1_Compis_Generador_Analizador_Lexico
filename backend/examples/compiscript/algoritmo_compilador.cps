// Prueba de fuego: algoritmos del propio compilador expresados en Compiscript.
// Referencias C++:
// - backend/src/dfa_builder.cpp: DFABuilder::bestRule.
// - backend/compiscript/src/tac_generator.cpp: newTemp y releaseTemp.
// No es autocompilación de C++: se conservan los algoritmos, usando las listas,
// clases y tipos que define la gramática Compiscript. La pila tiene capacidad
// fija porque el lenguaje no define push/pop ni listas redimensionables.

function mejorRegla(posiciones: integer[], reglas: integer[], total: integer): integer {
  let mejor: integer = -1;
  foreach (posicion in posiciones) {
    if (posicion < total && reglas[posicion] >= 0) {
      if (mejor == -1 || reglas[posicion] < mejor) {
        mejor = reglas[posicion];
      }
    }
  }
  return mejor;
}

class PoolTemporales {
  let siguiente: integer = 0;
  let libres: integer = 0;
  let creados: integer = 0;
  let reutilizados: integer = 0;
  let vivosActuales: integer = 0;
  let maximoVivos: integer = 0;
  let pila: integer[] = [0, 0, 0, 0, 0, 0, 0, 0];
  let vivos: boolean[] = [false, false, false, false, false, false, false, false];

  function adquirir(): integer {
    let temporal: integer = -1;
    if (this.libres > 0) {
      this.libres = this.libres - 1;
      temporal = this.pila[this.libres];
      this.reutilizados = this.reutilizados + 1;
    } else {
      temporal = this.siguiente;
      this.siguiente = this.siguiente + 1;
      this.creados = this.creados + 1;
    }
    this.vivos[temporal] = true;
    this.vivosActuales = this.vivosActuales + 1;
    if (this.vivosActuales > this.maximoVivos) {
      this.maximoVivos = this.vivosActuales;
    }
    return temporal;
  }

  function liberar(temporal: integer) {
    if (this.vivos[temporal]) {
      this.vivos[temporal] = false;
      this.vivosActuales = this.vivosActuales - 1;
      this.pila[this.libres] = temporal;
      this.libres = this.libres + 1;
    }
  }
}

let reglas: integer[] = [7, 4, -1, 2];
print(mejorRegla([0, 1, 3], reglas, 4)); // 2: menor prioridad aceptante.
print(mejorRegla([2], reglas, 4));       // -1: ninguna regla aceptante.
print(mejorRegla([8, 0], reglas, 4));    // 7: && evita leer reglas[8].

let pool = new PoolTemporales();
let primero = pool.adquirir();
let segundo = pool.adquirir();
print(primero);                        // 0
print(segundo);                        // 1
pool.liberar(primero);
print(pool.adquirir());                // 0: reutilizado.
let tercero = pool.adquirir();
print(tercero);                        // 2
pool.liberar(segundo);
pool.liberar(tercero);
print(pool.adquirir());                // 2: pila LIFO.
print(pool.creados);                   // 3
print(pool.reutilizados);              // 2
print(pool.maximoVivos);               // 3
