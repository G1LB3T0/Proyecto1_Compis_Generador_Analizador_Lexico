// Batería de integración: ejecuta casos válidos y fallidos por regla semántica.
#include "compiscript/json.h"
#include "compiscript/service.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace {

using Result = compiscript::AnalysisResult;

struct TestCase {
  std::string category;
  std::string rule;
  std::string name;
  std::string kind;
  std::string source;
  std::string expected;
  std::string explanation;
  std::function<bool(const Result &)> verify;
};

struct TestRun {
  TestCase test;
  Result result;
  bool passed = false;
  std::string actual;
  std::vector<std::string> codes;
};

std::vector<std::string> diagnosticCodes(const Result &result) {
  std::vector<std::string> values;
  for (const auto &diagnostic : result.syntax_diagnostics)
    values.push_back(diagnostic.code);
  for (const auto &diagnostic : result.semantic.diagnostics)
    values.push_back(diagnostic.code);
  return values;
}

bool hasCode(const Result &result, const std::string &code) {
  const auto values = diagnosticCodes(result);
  for (const auto &value : values) {
    if (value == code)
      return true;
  }
  return false;
}

std::string join(const std::vector<std::string> &values,
                 const std::string &separator) {
  std::ostringstream output;
  for (size_t index = 0; index < values.size(); ++index) {
    if (index)
      output << separator;
    output << values[index];
  }
  return output.str();
}

std::string actualResult(const Result &result) {
  std::vector<std::string> messages;
  for (const auto &diagnostic : result.syntax_diagnostics) {
    messages.push_back(diagnostic.code + " (línea " +
                       std::to_string(diagnostic.line) + "): " +
                       diagnostic.message);
  }
  for (const auto &diagnostic : result.semantic.diagnostics) {
    messages.push_back(diagnostic.code + " (línea " +
                       std::to_string(diagnostic.line) + "): " +
                       diagnostic.message);
  }
  if (!messages.empty())
    return join(messages, " | ");
  if (result.ok)
    return "Análisis léxico, sintáctico y semántico completado sin errores.";
  if (result.semantic.skipped)
    return "El análisis semántico se omitió porque la entrada no es sintácticamente válida.";
  return "El análisis finalizó sin diagnósticos detallados.";
}

TestCase valid(const std::string &category, const std::string &rule,
               const std::string &name, const std::string &source,
               const std::string &expected =
                   "El programa debe ser aceptado sin errores.") {
  const std::string explanation =
      "Este ejemplo es correcto porque el caso «" + name +
      "» utiliza de forma válida la regla «" + rule +
      "». Todos los valores y operaciones respetan lo que esa regla permite.";
  return {category, rule, name, "Caso válido", source, expected, explanation,
          [](const Result &result) { return result.ok; }};
}

TestCase invalid(const std::string &category, const std::string &rule,
                 const std::string &name, const std::string &source,
                 const std::string &code,
                 const std::string &explanation) {
  return {category, rule, name, "Error esperado", source,
          "El programa debe ser rechazado con " + code + ".", explanation,
          [code](const Result &result) { return hasCode(result, code); }};
}

TestCase custom(const std::string &category, const std::string &rule,
                const std::string &name, const std::string &source,
                const std::string &kind, const std::string &expected,
                std::function<bool(const Result &)> verify) {
  return {category, rule, name, kind, source, expected,
          "Este caso comprueba que " + expected, std::move(verify)};
}

std::vector<TestCase> buildTests() {
  std::vector<TestCase> tests;

  tests.push_back(valid("Proceso completo", "Pipeline de análisis",
                        "Programa mínimo válido", "let x: integer = 1;",
                        "ANTLR y el analizador semántico deben aceptar la entrada."));
  tests.push_back(invalid("Proceso completo", "Análisis léxico",
                          "Carácter no reconocido", "let x = 1; @", "LEX001",
                          "El lexer debe rechazar el carácter @."));
  tests.push_back(custom(
      "Proceso completo", "Análisis sintáctico", "Expresión incompleta",
      "let x: integer = ;", "Error esperado",
      "ANTLR debe detectar el error y la fase semántica debe quedar omitida.",
      [](const Result &result) {
        return !result.syntax_ok && result.semantic.skipped;
      }));
  tests.push_back(custom(
      "Proceso completo", "Clasificación de tokens",
      "Categorías descriptivas", "let edad: float = 2 + 3.5;", "Caso válido",
      "edad, :, float, + y 3.5 deben clasificarse con nombres comprensibles.",
      [](const Result &result) {
        bool identifier = false, separator = false, data_type = false;
        bool arithmetic = false, decimal = false;
        for (const auto &token : result.tokens) {
          identifier = identifier ||
                       (token.text == "edad" &&
                        token.category == "Identificador");
          separator = separator ||
                      (token.text == ":" && token.category == "Separador");
          data_type = data_type ||
                      (token.text == "float" &&
                       token.category == "Tipo de dato");
          arithmetic = arithmetic ||
                       (token.text == "+" &&
                        token.category == "Operador aritmético");
          decimal = decimal ||
                    (token.text == "3.5" &&
                     token.category == "Literal decimal");
        }
        return result.ok && identifier && separator && data_type && arithmetic &&
               decimal;
      }));

  tests.push_back(valid("Sistema de tipos", "Aritmética",
                        "Operaciones con integer",
                        "let x: integer = (8 + 2) * 3 - 4 % 2;"));
  tests.push_back(valid("Sistema de tipos", "Promoción numérica",
                        "integer puede promoverse a float",
                        "let x: float = 2 + 3.5;"));
  tests.push_back(valid("Sistema de tipos", "División",
                        "División numérica produce float",
                        "let promedio: float = 9 / 2;"));
  tests.push_back(invalid("Sistema de tipos", "Aritmética",
                          "Booleano usado en multiplicación",
                          "let x: integer = true * 2;", "SEM_ARITHMETIC_TYPE",
                          "La aritmética solo admite números."));
  tests.push_back(invalid("Sistema de tipos", "División",
                          "Booleano usado en división", "let x = true / 2;",
                          "SEM_ARITHMETIC_TYPE",
                          "El operador / necesita dos valores numéricos."));
  tests.push_back(valid("Sistema de tipos", "Operadores lógicos",
                        "Expresión lógica booleana",
                        "let x: boolean = true && !false;"));
  tests.push_back(valid("Sistema de tipos", "Operador lógico OR",
                        "OR combina dos booleanos",
                        "let disponible: boolean = false || true;"));
  tests.push_back(invalid("Sistema de tipos", "Operadores lógicos",
                          "Integer usado con AND", "let x = 1 && true;",
                          "SEM_LOGICAL_TYPE",
                          "Los operandos de && deben ser boolean."));
  tests.push_back(invalid("Sistema de tipos", "Operador lógico OR",
                          "String usado con OR", "let x = \"sí\" || false;",
                          "SEM_LOGICAL_TYPE",
                          "Los operandos de || deben ser boolean."));
  tests.push_back(valid("Sistema de tipos", "Comparaciones",
                        "Comparación numérica compatible",
                        "let x: boolean = 1 < 2.5;"));
  tests.push_back(valid("Sistema de tipos", "Operadores de comparación",
                        "Comparadores restantes compatibles",
                        "let a = 1 != 2; let b = 1 <= 2; let c = 2 > 1; let d = 2 >= 2;"));
  tests.push_back(invalid("Sistema de tipos", "Comparaciones",
                          "Comparación incompatible", "let x = 1 == \"uno\";",
                          "SEM_COMPARE_TYPE",
                          "No se deben comparar valores incompatibles."));
  tests.push_back(invalid("Sistema de tipos", "Comparaciones relacionales",
                          "Orden entre string e integer",
                          "let x = \"uno\" < 2;", "SEM_COMPARE_TYPE",
                          "No existe un orden compatible entre un texto y un número."));
  tests.push_back(valid("Sistema de tipos", "Asignación",
                        "Asignación del mismo tipo",
                        "let x: integer = 1; x = 2;"));
  tests.push_back(invalid("Sistema de tipos", "Asignación",
                          "Asignación de tipo distinto",
                          "let x: integer = 1; x = \"dos\";", "SEM_ASSIGN_TYPE",
                          "Una variable integer no debe recibir string."));
  tests.push_back(valid("Sistema de tipos", "Constantes",
                        "Constante inicializada", "const x: integer = 1;"));
  tests.push_back(invalid("Sistema de tipos", "Constantes",
                          "Constante sin inicializar", "const x: integer;",
                          "SYN001",
                          "La gramática exige inicialización en una constante."));
  tests.push_back(invalid("Sistema de tipos", "Constantes",
                          "Reasignación de constante",
                          "const x: integer = 1; x = 2;", "SEM_ASSIGN_CONST",
                          "Una constante no puede cambiar después de declararse."));
  tests.push_back(invalid("Sistema de tipos", "Tipos declarados",
                          "Tipo inexistente", "let x: Fantasma;",
                          "SEM_TYPE_UNKNOWN", "Todo tipo declarado debe existir."));
  tests.push_back(invalid("Sistema de tipos", "Tipo void",
                          "Variable de tipo void", "let x: void;",
                          "SEM_TYPE_VOID",
                          "void representa ausencia de valor, no una variable."));

  tests.push_back(valid("Manejo de ámbito", "Ámbito global y local",
                        "Variable local dentro de función",
                        "let global = 1; function f(): integer { let local = 2; return global + local; }"));
  tests.push_back(valid("Manejo de ámbito", "Sombreado",
                        "Nombre repetido en ámbito hijo",
                        "let x = 1; { let x = 2; print(x); } print(x);"));
  tests.push_back(invalid("Manejo de ámbito", "Declaración previa",
                          "Identificador no declarado", "print(x);",
                          "SEM_SCOPE_UNDECLARED",
                          "No puede usarse un nombre que no esté visible."));
  tests.push_back(invalid("Manejo de ámbito", "Unicidad local",
                          "Declaración duplicada en el mismo ámbito",
                          "let x = 1; const x = 2;", "SEM_SCOPE_DUPLICATE",
                          "Un mismo ámbito no admite dos símbolos con el mismo nombre."));
  tests.push_back(valid("Manejo de ámbito", "Ámbitos anidados",
                        "El bloque hijo accede al padre",
                        "let x = 1; { { print(x); } }"));
  tests.push_back(invalid("Manejo de ámbito", "Vida del símbolo",
                          "Variable usada fuera de su bloque",
                          "{ let temporal = 1; } print(temporal);",
                          "SEM_SCOPE_UNDECLARED",
                          "El símbolo local deja de estar visible al salir del bloque."));
  tests.push_back(custom(
      "Manejo de ámbito", "Construcción de scopes", "Tabla de ámbitos reales",
      "let global = 1; function f(x: integer): integer { let local = x; { let interno = local; } return local; }",
      "Caso válido",
      "El resultado debe contener ámbitos global, de función y de bloque.",
      [](const Result &result) {
        std::set<std::string> kinds;
        for (const auto &scope : result.semantic.scopes)
          kinds.insert(scope.kind);
        return result.ok && kinds.count("global") && kinds.count("function") &&
               kinds.count("block");
      }));

  tests.push_back(valid("Funciones y procedimientos", "Llamada y retorno",
                        "Función con argumento correcto",
                        "function doble(x: integer): integer { return x * 2; } let y = doble(4);"));
  tests.push_back(invalid("Funciones y procedimientos", "Aridad",
                          "Cantidad incorrecta de argumentos",
                          "function f(x: integer): integer { return x; } f();",
                          "SEM_FUNCTION_ARITY",
                          "La llamada debe respetar el número de parámetros."));
  tests.push_back(invalid("Funciones y procedimientos", "Tipos de argumentos",
                          "Argumento con tipo incorrecto",
                          "function f(x: integer): integer { return x; } f(true);",
                          "SEM_FUNCTION_ARGUMENT",
                          "Cada argumento debe ser compatible con su parámetro."));
  tests.push_back(valid("Funciones y procedimientos", "Procedimientos",
                        "Función void sin valor de retorno",
                        "function saludar(): void { print(\"hola\"); return; } saludar();"));
  tests.push_back(valid("Funciones y procedimientos", "Retorno",
                        "Función retorna el tipo prometido",
                        "function f(): integer { return 1; }"));
  tests.push_back(valid("Funciones y procedimientos", "Retorno por caminos",
                        "If y else retornan valor",
                        "function signo(x: integer): integer { if (x >= 0) { return 1; } else { return -1; } }"));
  tests.push_back(invalid("Funciones y procedimientos", "Tipo de retorno",
                          "Retorno incompatible",
                          "function f(): integer { return \"x\"; }",
                          "SEM_FUNCTION_RETURN",
                          "El valor retornado debe coincidir con la firma."));
  tests.push_back(invalid("Funciones y procedimientos", "Retorno obligatorio",
                          "Función sin return",
                          "function f(): integer { print(1); }",
                          "SEM_FUNCTION_MISSING_RETURN",
                          "Una función no void debe garantizar un retorno."));
  tests.push_back(valid("Funciones y procedimientos", "Recursión",
                        "Factorial se llama a sí mismo",
                        "function factorial(n: integer): integer { if (n <= 1) return 1; return n * factorial(n - 1); }"));
  tests.push_back(valid("Funciones y procedimientos", "Declaración anticipada",
                        "Llamada antes de la declaración",
                        "let x = sumar(1, 2); function sumar(a: integer, b: integer): integer { return a + b; }"));
  tests.push_back(custom(
      "Funciones y procedimientos", "Closures", "Captura de variable externa",
      "function exterior(base: integer): integer { function sumar(valor: integer): integer { return base + valor; } return sumar(2); }",
      "Caso válido",
      "La función sumar debe registrar que captura el parámetro base.",
      [](const Result &result) {
        for (const auto &symbol : result.semantic.symbols) {
          if (symbol.name == "sumar" && !symbol.captures.empty())
            return result.ok;
        }
        return false;
      }));
  tests.push_back(invalid("Funciones y procedimientos", "Unicidad",
                          "Función duplicada",
                          "function f() {} function f() {}",
                          "SEM_SCOPE_DUPLICATE",
                          "No puede declararse dos veces la misma función en un ámbito."));
  tests.push_back(invalid("Funciones y procedimientos", "Invocación",
                          "Variable usada como función", "let x = 1; x();",
                          "SEM_FUNCTION_NOT_CALLABLE",
                          "Solo funciones, métodos y clases invocables admiten ()."));

  tests.push_back(valid("Control de flujo", "Condición de if",
                        "If con boolean", "if (true) { print(1); }"));
  tests.push_back(invalid("Control de flujo", "Condición de if",
                          "If con integer", "if (1) { print(1); }",
                          "SEM_TYPE_MISMATCH",
                          "La condición de if debe ser boolean."));
  tests.push_back(valid("Control de flujo", "Ciclo while",
                        "While con condición booleana",
                        "let activo = true; while (activo) { activo = false; }"));
  tests.push_back(invalid("Control de flujo", "Ciclo while",
                          "While con string", "while (\"sí\") { break; }",
                          "SEM_TYPE_MISMATCH",
                          "La condición de while debe ser boolean."));
  tests.push_back(valid("Control de flujo", "Ciclo do-while",
                        "Do-while válido",
                        "let activo = false; do { activo = true; } while (!activo);"));
  tests.push_back(invalid("Control de flujo", "Ciclo do-while",
                          "Do-while con condición numérica",
                          "do { print(1); } while (1);", "SEM_TYPE_MISMATCH",
                          "La condición de do-while debe ser boolean."));
  tests.push_back(valid("Control de flujo", "Ciclo for", "For completo",
                        "for (let i: integer = 0; i < 3; i = i + 1) { print(i); }"));
  tests.push_back(invalid("Control de flujo", "Ciclo for",
                          "For con condición no booleana",
                          "for (; 1; ) { break; }", "SEM_TYPE_MISMATCH",
                          "La condición del for debe ser boolean."));
  tests.push_back(valid("Control de flujo", "Selección switch",
                        "Switch booleano",
                        "switch (true) { case true: print(1); default: print(0); }"));
  tests.push_back(invalid("Control de flujo", "Selección switch",
                          "Switch con selector integer",
                          "switch (1) { case 1: print(1); }",
                          "SEM_TYPE_MISMATCH",
                          "La especificación del proyecto exige selector boolean."));
  tests.push_back(invalid("Control de flujo", "Casos de switch",
                          "Case incompatible con el selector",
                          "switch (true) { case 1: print(1); }",
                          "SEM_SWITCH_CASE",
                          "Cada case debe ser compatible con el selector."));
  tests.push_back(valid("Control de flujo", "Sentencia break",
                        "Break dentro de ciclo", "while (true) { break; }"));
  tests.push_back(invalid("Control de flujo", "Sentencia break",
                          "Break fuera de un ciclo", "break;",
                          "SEM_FLOW_BREAK",
                          "La rúbrica permite break únicamente dentro de un ciclo."));
  tests.push_back(invalid("Control de flujo", "Sentencia break",
                          "Break dentro de switch pero fuera de ciclo",
                          "switch (true) { case true: break; }",
                          "SEM_FLOW_BREAK",
                          "Aunque otros lenguajes lo permiten, la rúbrica limita break a los ciclos."));
  tests.push_back(valid("Control de flujo", "Sentencia continue",
                        "Continue dentro de ciclo",
                        "for (; true; ) { continue; }"));
  tests.push_back(invalid("Control de flujo", "Sentencia continue",
                          "Continue fuera de ciclo", "continue;",
                          "SEM_FLOW_CONTINUE",
                          "continue solo es válido dentro de un ciclo."));
  tests.push_back(invalid("Control de flujo", "Sentencia return",
                          "Return fuera de función", "return 1;",
                          "SEM_FLOW_RETURN",
                          "return solo es válido dentro de una función."));
  tests.push_back(invalid("Control de flujo", "Código inalcanzable",
                          "Sentencia después de return",
                          "function f(): integer { return 1; print(2); }",
                          "SEM_FLOW_DEAD_CODE",
                          "Una sentencia posterior a return no puede ejecutarse."));
  tests.push_back(invalid("Control de flujo", "Código inalcanzable",
                          "Sentencia después de break",
                          "while (true) { break; print(1); }",
                          "SEM_FLOW_DEAD_CODE",
                          "Dentro del mismo bloque, lo escrito después de break no puede ejecutarse."));
  tests.push_back(invalid("Control de flujo", "Código inalcanzable",
                          "Sentencia después de continue",
                          "while (true) { continue; print(1); }",
                          "SEM_FLOW_DEAD_CODE",
                          "Dentro del mismo bloque, continue salta a la siguiente iteración y deja inalcanzable lo posterior."));

  tests.push_back(valid("Clases y objetos", "Declaración e instancia",
                        "Clase con campo, constructor y método",
                        "class Persona { let nombre: string; function constructor(nombre: string) { this.nombre = nombre; } function saludar(): string { return this.nombre; } } let p = new Persona(\"Ana\"); print(p.saludar());"));
  tests.push_back(invalid("Clases y objetos", "Acceso a miembros",
                          "Miembro inexistente",
                          "class A {} let a = new A(); print(a.x);",
                          "SEM_CLASS_MEMBER",
                          "El miembro solicitado debe existir en la clase."));
  tests.push_back(invalid("Clases y objetos", "Acceso a propiedades",
                          "Propiedad sobre un integer", "let x = 1; print(x.valor);",
                          "SEM_CLASS_PROPERTY",
                          "Solo un objeto de clase expone propiedades."));
  tests.push_back(valid("Clases y objetos", "Constructor predeterminado",
                        "Clase sin constructor acepta cero argumentos",
                        "class A {} let a = new A();"));
  tests.push_back(invalid("Clases y objetos", "Constructor",
                          "Argumentos sin constructor declarado",
                          "class A {} let a = new A(1);",
                          "SEM_CLASS_CONSTRUCTOR",
                          "Sin constructor explícito solo se aceptan cero argumentos."));
  tests.push_back(invalid("Clases y objetos", "Constructor",
                          "Tipo incorrecto en constructor",
                          "class A { function constructor(x: integer) {} } let a = new A(true);",
                          "SEM_FUNCTION_ARGUMENT",
                          "Los argumentos del constructor respetan su firma."));
  tests.push_back(valid("Clases y objetos", "Uso de this",
                        "this dentro de un método",
                        "class A { let x: integer; function valor(): integer { return this.x; } }"));
  tests.push_back(invalid("Clases y objetos", "Uso de this",
                          "this fuera de una clase", "print(this);",
                          "SEM_CLASS_THIS",
                          "this solo existe dentro de métodos y constructores."));
  tests.push_back(valid("Clases y objetos", "Herencia y subtipo",
                        "Objeto hijo asignado a variable padre",
                        "class Animal {} class Perro : Animal {} let perro = new Perro(); let animal: Animal = perro;"));
  tests.push_back(invalid("Clases y objetos", "Instanciación",
                          "Clase inexistente", "let a = new A();",
                          "SEM_CLASS_UNKNOWN",
                          "new requiere una clase declarada."));
  tests.push_back(invalid("Clases y objetos", "Herencia",
                          "Clase base inexistente", "class A : B {}",
                          "SEM_CLASS_BASE_UNKNOWN",
                          "La clase base debe estar declarada."));
  tests.push_back(invalid("Clases y objetos", "Herencia",
                          "Ciclo de herencia", "class A : B {} class B : A {}",
                          "SEM_CLASS_CYCLE",
                          "La jerarquía no puede contener ciclos."));
  tests.push_back(invalid("Clases y objetos", "Tipo de campos",
                          "Asignación incompatible a un campo",
                          "class A { let x: integer; } let a = new A(); a.x = true;",
                          "SEM_ASSIGN_TYPE",
                          "Los campos conservan el tipo con que fueron declarados."));

  tests.push_back(valid("Listas y estructuras de datos", "Literal de lista",
                        "Lista homogénea", "let datos = [1, 2, 3];"));
  tests.push_back(valid("Listas y estructuras de datos", "Promoción numérica",
                        "Lista combina integer y float",
                        "let datos: float[] = [1, 2.5, 3];"));
  tests.push_back(invalid("Listas y estructuras de datos", "Homogeneidad",
                          "Lista con tipos incompatibles", "let datos = [1, true];",
                          "SEM_LIST_ELEMENT",
                          "Los elementos deben compartir un tipo compatible."));
  tests.push_back(invalid("Listas y estructuras de datos", "Tipo declarado",
                          "Lista no coincide con anotación",
                          "let datos: integer[] = [1, 2.5];", "SEM_ASSIGN_TYPE",
                          "El literal debe ser asignable al tipo de lista declarado."));
  tests.push_back(valid("Listas y estructuras de datos", "Listas anidadas",
                        "Matriz de integer",
                        "let matriz: integer[][] = [[1, 2], [3, 4]]; print(matriz[0][1]);"));
  tests.push_back(valid("Listas y estructuras de datos", "Lista vacía",
                        "Lista vacía con tipo explícito",
                        "let datos: integer[] = [];"));
  tests.push_back(valid("Listas y estructuras de datos", "Índice",
                        "Acceso con índice integer",
                        "let datos = [10, 20]; print(datos[0]);"));
  tests.push_back(invalid("Listas y estructuras de datos", "Índice",
                          "Índice booleano", "let datos = [1, 2]; print(datos[true]);",
                          "SEM_LIST_INDEX",
                          "El índice de una lista debe ser integer."));
  tests.push_back(invalid("Listas y estructuras de datos", "Acceso",
                          "Indexación de un valor que no es lista",
                          "let x = 1; print(x[0]);", "SEM_LIST_ACCESS",
                          "Solo una lista puede indexarse."));
  tests.push_back(valid("Listas y estructuras de datos", "Recorrido foreach",
                        "Foreach sobre lista",
                        "let datos = [1, 2]; foreach (dato in datos) { print(dato); }"));
  tests.push_back(invalid("Listas y estructuras de datos", "Recorrido foreach",
                          "Foreach sobre integer",
                          "foreach (dato in 1) { print(dato); }", "SEM_LIST_FOREACH",
                          "foreach requiere una colección de tipo lista."));

  tests.push_back(valid("Reglas generales", "Concatenación",
                        "Suma de strings",
                        "let saludo: string = \"Hola, \" + \"mundo\";"));
  tests.push_back(invalid("Reglas generales", "Objetivo de asignación",
                          "Asignación al resultado de una llamada",
                          "function f(): integer { return 1; } f() = 2;",
                          "SEM_ASSIGN_TARGET",
                          "El lado izquierdo debe ser una variable, campo o índice asignable."));
  tests.push_back(invalid("Reglas generales", "Sentido de expresiones",
                          "Multiplicación de una función",
                          "function f(): integer { return 1; } let x = f * 2;",
                          "SEM_ARITHMETIC_TYPE",
                          "El nombre f representa una función, no un número que pueda multiplicarse."));
  tests.push_back(valid("Reglas generales", "Operador ternario",
                        "Ternario con ramas compatibles",
                        "let x: integer = true ? 1 : 2;"));
  tests.push_back(invalid("Reglas generales", "Operador ternario",
                          "Condición ternaria no booleana",
                          "let x = 1 ? 2 : 3;", "SEM_TYPE_MISMATCH",
                          "La condición del ternario debe ser boolean."));
  tests.push_back(invalid("Reglas generales", "Operador ternario",
                          "Ramas ternarias incompatibles",
                          "let x = true ? 1 : \"dos\";", "SEM_TERNARY_TYPE",
                          "Las dos ramas deben producir tipos compatibles."));
  tests.push_back(invalid("Reglas generales", "Parámetros",
                          "Parámetro duplicado",
                          "function f(x: integer, x: integer) { return; }",
                          "SEM_SCOPE_DUPLICATE",
                          "Los parámetros comparten el ámbito de la función."));

  return tests;
}

std::vector<TestRun> runTests() {
  std::vector<TestRun> runs;
  for (const auto &test : buildTests()) {
    Result result = compiscript::analyzeSource(test.source, test.name + ".cps");
    runs.push_back(
        {test, result, test.verify(result), actualResult(result),
         diagnosticCodes(result)});
  }
  return runs;
}

std::string stringArray(const std::vector<std::string> &values) {
  std::ostringstream output;
  output << '[';
  for (size_t index = 0; index < values.size(); ++index) {
    if (index)
      output << ',';
    output << compiscript::quoteJson(values[index]);
  }
  output << ']';
  return output.str();
}

std::string simpleExplanation(const TestRun &run) {
  if (run.test.kind == "Error esperado") {
    std::string message = "Este código es incorrecto a propósito. " +
                          run.test.explanation;
    if (run.passed) {
      message += " El analizador detectó exactamente ese problema";
      if (!run.codes.empty())
        message += " mediante " + join(run.codes, ", ");
      message +=
          ". Por eso la prueba aparece aprobada: rechazar este programa es el comportamiento correcto.";
    } else {
      message +=
          " La prueba falla porque el analizador no produjo el diagnóstico que debía producir.";
    }
    return message;
  }

  std::string message = run.test.explanation;
  if (run.passed) {
    message +=
        " El analizador recorrió el programa sin encontrar errores, por eso la prueba es aprobada.";
  } else {
    message +=
        " Sin embargo, el analizador produjo un error inesperado y por eso la prueba falla.";
  }
  return message;
}

std::string reportJson(const std::vector<TestRun> &runs) {
  size_t passed = 0, valid_cases = 0, expected_error_cases = 0;
  std::vector<std::string> category_order;
  std::map<std::string, std::vector<const TestRun *>> categories;
  for (const auto &run : runs) {
    passed += run.passed ? 1 : 0;
    valid_cases += run.test.kind == "Caso válido" ? 1 : 0;
    expected_error_cases += run.test.kind == "Error esperado" ? 1 : 0;
    if (!categories.count(run.test.category))
      category_order.push_back(run.test.category);
    categories[run.test.category].push_back(&run);
  }

  std::ostringstream output;
  output << "{\"ok\":" << (passed == runs.size() ? "true" : "false")
         << ",\"summary\":{\"total\":" << runs.size()
         << ",\"passed\":" << passed
         << ",\"failed\":" << (runs.size() - passed)
         << ",\"valid_cases\":" << valid_cases
         << ",\"expected_error_cases\":" << expected_error_cases
         << "},\"categories\":[";

  for (size_t category_index = 0; category_index < category_order.size();
       ++category_index) {
    if (category_index)
      output << ',';
    const auto &name = category_order[category_index];
    const auto &items = categories[name];
    size_t category_passed = 0;
    for (const auto *item : items)
      category_passed += item->passed ? 1 : 0;
    output << "{\"name\":" << compiscript::quoteJson(name)
           << ",\"total\":" << items.size()
           << ",\"passed\":" << category_passed << ",\"tests\":[";
    for (size_t test_index = 0; test_index < items.size(); ++test_index) {
      if (test_index)
        output << ',';
      const auto &run = *items[test_index];
      output << "{\"name\":" << compiscript::quoteJson(run.test.name)
             << ",\"rule\":" << compiscript::quoteJson(run.test.rule)
             << ",\"kind\":" << compiscript::quoteJson(run.test.kind)
             << ",\"passed\":" << (run.passed ? "true" : "false")
             << ",\"source\":" << compiscript::quoteJson(run.test.source)
             << ",\"explanation\":"
             << compiscript::quoteJson(simpleExplanation(run))
             << ",\"expected\":" << compiscript::quoteJson(run.test.expected)
             << ",\"actual\":" << compiscript::quoteJson(run.actual)
             << ",\"codes\":" << stringArray(run.codes) << '}';
    }
    output << "]}";
  }
  output << "]}";
  return output.str();
}

} // namespace

int main(int argc, char **argv) {
  const auto runs = runTests();
  size_t passed = 0;
  for (const auto &run : runs)
    passed += run.passed ? 1 : 0;

  if (argc > 1 && std::string(argv[1]) == "--json") {
    std::cout << reportJson(runs) << '\n';
  } else {
    for (const auto &run : runs) {
      if (!run.passed) {
        std::cerr << "FALLO [" << run.test.category << " / " << run.test.rule
                  << "] " << run.test.name << '\n'
                  << "  Esperado: " << run.test.expected << '\n'
                  << "  Obtenido: " << run.actual << '\n';
      }
    }
    std::cout << "compiscript_tests: " << passed << '/' << runs.size()
              << " pruebas aprobadas\n";
  }
  return passed == runs.size() ? EXIT_SUCCESS : EXIT_FAILURE;
}
