class Animal {
  let nombre: string;

  function constructor(nombre: string) {
    this.nombre = nombre;
  }

  function hablar(): string {
    return this.nombre + " hace ruido.";
  }
}

class Perro : Animal {
  function hablar(): string {
    return this.nombre + " ladra.";
  }
}

function factorial(n: integer): integer {
  if (n <= 1) return 1;
  return n * factorial(n - 1);
}

let perro: Perro = new Perro("Toby");
let notas: integer[] = [90, 85, 100];

foreach (nota in notas) {
  print(nota);
}

print(perro.hablar());
print(factorial(5));
