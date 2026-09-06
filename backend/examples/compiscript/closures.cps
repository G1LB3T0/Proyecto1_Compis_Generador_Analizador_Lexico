function crearAcumulador(base: integer): integer {
  function sumar(valor: integer): integer {
    return base + valor;
  }

  return sumar(10);
}

let resultado: integer = crearAcumulador(5);
print(resultado);
