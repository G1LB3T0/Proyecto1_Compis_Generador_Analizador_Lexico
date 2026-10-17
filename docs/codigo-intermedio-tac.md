# Lenguaje intermedio TAC de Compiscript

## Propósito y posición en el compilador

El Proyecto 2 agrega una fase de generación de código intermedio después del
análisis semántico. El generador nunca intenta corregir ni reinterpretar un
programa inválido:

```text
fuente .cps
  → lexer y parser ANTLR
  → árbol sintáctico
  → SemanticAnalyzer
  → tabla de símbolos y asociaciones semánticas
  → TacGenerator (solo cuando no hay errores)
  → cuádruplos + texto TAC + registros de activación
```

`SemanticAnalyzer` y `TacGenerator` son Visitors diferentes. El primero decide
qué significa cada identificador, comprueba tipos y construye ámbitos. El
segundo consume esa información y vuelve a recorrer el mismo árbol para emitir
instrucciones. Así no se genera código parcial para programas rechazados y no
se duplican las reglas de resolución de nombres.

El diseño toma como referencia conceptual el capítulo 6 del libro del dragón
para cuádruplos, traducción de expresiones, control de flujo y procedimientos,
y las secciones 7.2–7.3 para registros de activación y acceso a datos no
locales. El enunciado del proyecto sigue siendo la fuente de requisitos.

## Representación interna

Cada instrucción es un cuádruplo `TacInstruction`:

| Campo | Significado |
|---|---|
| `op` | Código estable de la operación. |
| `arg1` | Primer operando, si la forma lo necesita. |
| `arg2` | Segundo operando, si la forma lo necesita. |
| `result` | Destino, etiqueta o nombre principal. |
| `index` | Posición de la instrucción en la secuencia. |
| `line`, `column` | Origen aproximado en el archivo `.cps`. |
| `text` | Serialización legible del mismo cuádruplo. |

La implementación conserva los cuádruplos para pruebas, IDE y futuras fases.
El texto no se vuelve a analizar para recuperar información.

### Direcciones y nombres

| Forma | Ejemplo | Uso |
|---|---|---|
| Constante | `4`, `true`, `"hola"`, `null` | Literales del programa. |
| Global | `@total#3` | Símbolo sin función propietaria. |
| Local o parámetro | `%valor#7` | Símbolo dentro de un registro de activación. |
| Temporal | `%t0` | Resultado intermedio generado por el compilador. |
| Captura | `env[%base#1]` | Valor obtenido mediante el enlace de acceso. |
| Receptor | `%this` | Objeto implícito de un método. |
| Etiqueta | `L_while_cond_2` | Destino simbólico de un salto. |

El sufijo `#id` evita que dos variables sombreadas compartan una dirección TAC.

## Conjunto de instrucciones

### Datos y expresiones

| `op` | Forma textual | Semántica |
|---|---|---|
| `declare` | `declare x : T` | Reserva una dirección simbólica. |
| `assign` | `x = y` | Copia el valor de `y`. |
| `add`, `sub`, `mul`, `div`, `mod` | `t = x op y` | Operación aritmética o concatenación validada. |
| `neg`, `not` | `t = -x`, `t = !x` | Operación unaria. |
| `eq`, `ne`, `lt`, `le`, `gt`, `ge` | `t = x relop y` | Comparación con resultado booleano. |

### Control de flujo

| `op` | Forma textual | Semántica |
|---|---|---|
| `label` | `L:` | Define un destino simbólico. |
| `goto` | `goto L` | Salto incondicional. |
| `if_true` | `if x goto L` | Salta cuando `x` es verdadero. |
| `if_false` | `ifFalse x goto L` | Salta cuando `x` es falso. |

`&&` y `||` se traducen con saltos de cortocircuito; no se evalúa el operando
derecho cuando el izquierdo ya determina el resultado. `break` usa la etiqueta
de salida del bucle y `continue` usa la etiqueta de condición o incremento que
corresponde al tipo de ciclo.

### Funciones, procedimientos y closures

| `op` | Forma textual | Semántica |
|---|---|---|
| `begin_func`, `end_func` | `func f:`, `endfunc f` | Delimita una rutina. |
| `param_in` | `param_in x, n` | Vincula un parámetro formal. |
| `param` | `param x` | Pasa un argumento evaluado. |
| `call` | `t = call f, n` | Invoca una función. |
| `call_method` | `t = call_method C.m, n` | Invoca un método con receptor implícito. |
| `return` | `return x` | Devuelve control y un valor opcional. |
| `capture` | `capture x from access_link` | Declara acceso a un nombre no local. |
| `static_link` | `static_link f = current_env` | Prepara el entorno léxico de una llamada anidada. |

Las funciones se pueden llamar antes de su aparición textual porque el
análisis semántico predeclara firmas y el TAC usa etiquetas simbólicas. La
recursividad usa la misma etiqueta de función. Las capturas registradas en la
tabla semántica se convierten en accesos `env[...]` y aparecen también en el
registro de activación.

### Clases, objetos y listas

| `op` | Forma textual | Semántica |
|---|---|---|
| `begin_class`, `end_class` | `class C:`, `endclass C` | Delimita metadatos y métodos de clase. |
| `field_decl`, `field_init` | `field x : T`, `field x = v` | Define un atributo y su valor inicial. |
| `new_object` | `t = new C` | Reserva una instancia. |
| `load_field` | `t = objeto.campo` | Lee un atributo. |
| `store_field` | `objeto.campo = v` | Escribe un atributo. |
| `new_list` | `t = newlist n` | Crea una lista con `n` posiciones. |
| `load_index` | `t = lista[i]` | Lee un elemento. |
| `store_index` | `lista[i] = v` | Escribe un elemento. |
| `length` | `t = length lista` | Obtiene el límite usado por `foreach`. |

El constructor recibe `%this` como argumento implícito. Ese receptor no forma
parte de la aridad declarada por el usuario.

### Otras operaciones

`print`, `try_begin`, `catch` y `try_end` representan las construcciones del
subconjunto Compiscript que no requieren descomposición adicional en esta fase.
`begin_program` y `end_program` delimitan el código de nivel global.

## Ejemplos de traducción

### Precedencia y temporales

```typescript
let x = a + b * 4;
```

```text
declare @x#2 : integer
%t0 = @b#1 * 4
%t1 = @a#0 + %t0
@x#2 = %t1
```

### Condición y ciclo

```typescript
while (x < 10) {
  x = x + 1;
}
```

```text
L_while_cond_0:
%t0 = @x#0 < 10
ifFalse %t0 goto L_while_end_1
%t0 = @x#0 + 1
@x#0 = %t0
goto L_while_cond_0
L_while_end_1:
```

### Función y llamada

```typescript
function doble(x: integer): integer { return x * 2; }
let y = doble(4);
```

```text
func fn_doble_0:
param_in %x#1, 0
%t0 = %x#1 * 2
return %t0
endfunc fn_doble_0
declare @y#2 : integer
param 4
%t0 = call fn_doble_0, 1
@y#2 = %t0
```

### Lista

```typescript
let datos = [10, 20];
datos[1] = datos[0] + 5;
```

```text
%t0 = newlist 2
%t0[0] = 10
%t0[1] = 20
@datos#0 = %t0
%t0 = @datos#0[1]
%t1 = @datos#0[0]
%t2 = %t1 + 5
@datos#0[1] = %t2
```

La primera lectura del lado izquierdo conserva la localización asignable. Una
fase de optimización posterior puede eliminar esa carga si su valor no se usa.

## Temporales y reciclaje

Cada función y el nivel global tienen un `TempPool` independiente:

1. `newTemp` toma un nombre de la lista libre o crea `%tN`.
2. Una instrucción se emite antes de liberar sus operandos.
3. Cuando el valor ya fue consumido, `releaseTemp` devuelve el nombre al pool.
4. El generador registra temporales creados, reutilizaciones y máximo de valores
   simultáneamente vivos.
5. El máximo simultáneo determina el espacio de temporales del registro de
   activación, no el número total de expresiones del programa.

No se realiza propagación de constantes, eliminación de código muerto TAC ni
asignación de registros físicos; esas optimizaciones pertenecen a fases
posteriores.

## Tabla de símbolos y memoria abstracta

Cada `Symbol` agrega:

- `size`: tamaño del valor;
- `offset`: desplazamiento en globales, objetos o frames;
- `lexical_depth`: profundidad del entorno;
- `storage`: `global`, `local`, `captured-local`, `parameter`, `field`, `code`
  o `type`;
- `label`: etiqueta para funciones, métodos y clases.

Supuestos de tamaño:

- `boolean`: 1 byte;
- `integer`: 4 bytes;
- `float`: 8 bytes;
- string, lista, objeto y función: referencia de 8 bytes;
- cada slot del layout se alinea a 8 bytes para mantener offsets simples.

Los campos heredados preceden a los campos de la clase derivada.

## Registros de activación

El frame abstracto usa offsets relativos al frame pointer:

| Offset | Contenido |
|---:|---|
| `0` | Enlace de control. |
| `8` | Enlace de acceso para ámbitos léxicos y closures. |
| `16` | Dirección de retorno. |
| `24` | Valor de retorno. |
| `32...` | Parámetros formales. |
| Negativos | Variables locales y capturadas. |
| Área adicional | Máximo de temporales simultáneamente vivos. |

El resultado expone bytes de parámetros, locales y temporales, tamaño total,
profundidad léxica y símbolos capturados para que el diseño sea inspeccionable
desde el IDE y las pruebas.

## Supuestos de traducción

- Solo se genera TAC si léxico, sintaxis y semántica terminaron sin errores.
- Las etiquetas son simbólicas y todavía no son direcciones de máquina.
- `switch` no tiene caída implícita: cada `case` salta al final. Esta decisión es
  coherente con la regla del proyecto que reserva `break` para bucles.
- Los argumentos se evalúan de izquierda a derecha.
- `%this` se pasa antes de los argumentos explícitos de un método.
- Las listas son referencias dinámicas; `foreach` se baja a índice, `length`,
  comparación, lectura y salto.
- `try/catch` se conserva mediante operaciones abstractas porque el proyecto no
  define todavía un ABI de excepciones.
- El TAC es una representación intermedia, no un intérprete ni código objeto.

## Compilar, ejecutar y verificar

macOS o Linux:

```bash
./scripts/build_compiscript.sh
```

Windows PowerShell:

```powershell
.\scripts\build_compiscript.ps1
```

CLI:

```bash
./compiscript_cli --file backend/examples/compiscript/codigo_intermedio.cps
```

El JSON incluye `tac.text`, `tac.instructions`, `tac.activation_records` y las
métricas de temporales. Para usar el IDE:

```bash
python3 app.py
```

Abrir `http://127.0.0.1:5050`, analizar el archivo y seleccionar **Código TAC**.

La batería se ejecuta con:

```bash
ctest --test-dir build/compiscript --output-on-failure
```

Incluye casos de precedencia, lógica de cortocircuito, control de flujo,
`switch`, recursividad, closures, clases, listas, reciclaje de temporales,
layout de símbolos y omisión de TAC ante entradas sintáctica o semánticamente
inválidas.
