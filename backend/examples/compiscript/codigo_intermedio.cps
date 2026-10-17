// Ejemplo integral para inspeccionar TAC, temporales y registros de activación.
class Acumulador {
  let total: integer;

  function constructor(inicial: integer) {
    this.total = inicial;
  }

  function sumar(valor: integer): integer {
    this.total = this.total + valor;
    return this.total;
  }
}

function procesar(datos: integer[]): integer {
  let acumulador = new Acumulador(0);
  foreach (dato in datos) {
    acumulador.sumar(dato);
  }
  return acumulador.total;
}

let valores = [1, 2, 3, 4];
let resultado = procesar(valores);
print(resultado);
