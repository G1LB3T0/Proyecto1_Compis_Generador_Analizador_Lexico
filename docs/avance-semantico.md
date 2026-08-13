# Avance semántico: tipos, ámbitos y clases

## Alcance implementado

Este avance agrega una fase semántica pequeña y demostrable sobre el pipeline existente. No pretende ser todavía un comprobador completo del lenguaje.

Incluye:

- Árbol sintáctico concreto construido durante `shift/reduce`.
- AST mínimo con clases, atributos, métodos, parámetros, variables y bloques.
- Tipos primitivos y tipos de clase.
- Firmas de métodos como tipos compuestos.
- Ámbitos global, de clase, de método y de bloque.
- Símbolos de clase, atributo, método, parámetro y variable.
- Modelo de clases con sus atributos y métodos.
- Errores por declaración duplicada y tipo desconocido.
- Salida JSON determinista para SLR y LALR.

No incluye:

- Reglas de tipos para expresiones y asignaciones.
- Resolución de usos de identificadores.
- Validación de llamadas o `return`.
- Herencia, sobrecarga o modificadores de acceso.
- Semántica sobre LL(1).
- Visualización específica en el frontend.

Estas exclusiones son intencionales: permiten presentar un avance que responde a “sacar tipos, ámbitos y clases” sin mezclar la comprobación completa del lenguaje.

## Dónde ocurre cada parte

```text
Lexer
  -> vector<Token>
Parser SLR/LALR
  -> ParseTreeNode (CST)
ASTBuilder
  -> ASTNode
SemanticAnalyzer
  -> SemanticModel
       |- types
       |- scopes
       |- symbols
       |- classes
       `- diagnostics
semantic_json
  -> objeto "semantic" en la respuesta del CLI/API
```

### Tipos

`SemanticAnalyzer::seedPrimitiveTypes` registra `int`, `float`, `double`, `char`, `bool`, `string` y `void`. La predeclaración agrega cada clase como un tipo. Cada método agrega una firma con tipos de parámetros y retorno.

### Ámbitos

El analizador crea un ámbito al entrar en el programa, una clase, un método o un bloque. Cada ámbito conserva `parent_id`, de modo que la estructura forma un árbol y prepara la futura búsqueda léxica de nombres.

### Clases

La primera pasada predeclara todas las clases. La segunda crea su ámbito y registra atributos y métodos. Esto permite que una variable use como tipo una clase declarada más adelante.

## Fundamento académico

La implementación respeta la separación mostrada en **01 - Compiladores v 1.2**, diapositiva 7: análisis léxico, análisis sintáctico, análisis semántico y representación intermedia son fases diferentes, mientras la tabla de símbolos apoya el front-end.

El CST/AST funciona como los atributos sintetizados explicados en **02 - Traducción orientada por la sintaxis v 1.0**: cada reducción combina los nodos de su lado derecho y sintetiza el nodo del lado izquierdo. Los ámbitos representan el contexto heredado; se construyen en una pasada posterior para evitar efectos laterales dependientes del orden de reducción.

En términos de Cardelli, la cadena de ámbitos será el entorno `Γ`. Este avance construye `Γ` y asocia declaraciones con tipos; el juicio completo `Γ ⊢ e : T` se implementará cuando se agreguen expresiones.

La organización también sigue el Dragon Book: tablas de símbolos (2.7), traducción dirigida por la sintaxis (capítulo 5), árboles (6.1), tipos y declaraciones (6.3), y posteriormente comprobación de tipos (6.5).

## Gramática de demostración

Los archivos `backend/examples/semantica.yal` y `semantica.yalp` definen el subconjunto aceptado. Por ejemplo:

```cpp
class Cuenta {
    int saldo;
    float retirar(float monto) {
        bool autorizado;
        { int temporal; }
    }
};
Cuenta principal;
```

El modelo esperado contiene:

```text
Tipos: int, float, double, char, bool, string, void, Cuenta,
       Cuenta.retirar(float):float

Ámbito global
|- clase Cuenta: Cuenta
|- variable principal: Cuenta
`- ámbito de clase Cuenta
   |- campo saldo: int
   `- método retirar: (float) -> float
      `- ámbito del método
         |- parámetro monto: float
         `- bloque
            |- variable autorizado: bool
            `- bloque anidado
               `- variable temporal: int
```

## Diagnósticos activos

| Código | Significado |
|---|---|
| `SEM002` | Nombre duplicado en el mismo ámbito |
| `SEM003` | Tipo utilizado pero no declarado |
| `SEM010` | Clase duplicada |

El ocultamiento en un ámbito hijo no se reporta como duplicado. Aún no se resuelven usos de nombres, por lo que este avance no emite “variable no declarada”.

## Contrato JSON

Cuando la sintaxis es válida con SLR o LALR, la respuesta añade:

```json
{
  "semantic": {
    "ok": true,
    "skipped": false,
    "ast": {},
    "types": [],
    "scopes": [],
    "symbols": [],
    "classes": [],
    "diagnostics": []
  }
}
```

Si hay errores sintácticos, no se interpreta un árbol incompleto. Con LL(1), `semantic.skipped` es `true` hasta implementar la normalización descrita en `plan.md`.

Las gramáticas anteriores que no contienen los no terminales del contrato semántico continúan funcionando, pero también devuelven `semantic.skipped = true`. Esto evita presentar como correcto un modelo vacío construido a partir de una gramática que no describe clases o declaraciones.

## Compilación

Se requiere un compilador con C++17. En PowerShell:

```powershell
.\backend\build_yapar_cli.ps1
```

El script produce `yapar_cli.exe` en la raíz del repositorio y lista explícitamente los módulos para evitar múltiples funciones `main`.

## Pruebas

Prueba unitaria del modelo:

```powershell
g++ -std=c++17 -I backend/src `
  backend/src/semantic_model.cpp `
  backend/src/semantic_analyzer.cpp `
  backend/src/semantic_json.cpp `
  backend/src/ast.cpp `
  backend/src/parse_tree.cpp `
  backend/tests/semantic_tests.cpp `
  -o semantic_tests.exe
.\semantic_tests.exe
```

Después de compilar el CLI, la prueba de integración ejecuta los mismos ejemplos con SLR y LALR y exige modelos idénticos:

```powershell
python backend/tests/verify_semantic_cli.py
```

`semantic_valid.txt` debe terminar sin diagnósticos. `semantic_errors.txt` debe producir una clase duplicada, dos declaraciones duplicadas y dos tipos desconocidos.

## Próximo avance recomendado

El siguiente incremento debe agregar expresiones al AST y resolución de nombres con búsqueda desde el ámbito actual hacia el padre. Después podrá implementarse el juicio de tipos `Γ ⊢ e : T`. No conviene abordar todavía herencia, sobrecarga ni generación de código.
