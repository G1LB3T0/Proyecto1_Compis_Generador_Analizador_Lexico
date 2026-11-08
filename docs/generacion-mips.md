# Generación y ejecución MIPS de Compiscript

## Objetivo y alcance

La fase 3 agrega un backend MIPS32 al compilador de Compiscript. Recibe el TAC
estructurado y la tabla de símbolos producidos por las fases anteriores. Su
salida es ensamblador que MARS ensambla a código máquina y ejecuta en su
simulador.

```text
archivo .cps
  → lexer y parser ANTLR
  → análisis semántico y tabla de símbolos
  → TAC y registros de activación abstractos
  → MipsGenerator y frames físicos
  → ensamblador .asm
  → ensamblado y ejecución en MARS
```

El enunciado exige un programa MIPS ejecutable en un simulador. Los artefactos
de código máquina corresponden a esa arquitectura y a su entorno de servicios
MARS. El código fuente aceptado sigue siendo Compiscript, definido en
`backend/compiscript/grammar/Compiscript.g4`.

El proyecto conserva las decisiones del frontend documentadas en
[decisiones-compiscript.md](decisiones-compiscript.md): soporte de `float`,
concatenación de dos cadenas, cuerpos de control con una sentencia y `switch`
con selector booleano.

## Componentes

| Componente | Responsabilidad |
|---|---|
| `TacGenerator` | Traducir el árbol validado a cuádruplos con símbolos resueltos. |
| `MipsGenerator` | Seleccionar instrucciones, determinar memoria y generar rutinas y llamadas MIPS. |
| `MipsFrame` | Describir el registro de activación físico de cada rutina. |
| `MipsResult` | Conservar ensamblador, diagnósticos, métricas y frames. |
| `service.cpp` | Ejecutar las fases en orden y omitir las que no tienen una entrada válida. |
| CLI | Recibir `.cps` y exponer la compilación mediante el mismo núcleo C++. |
| Adaptador del simulador | Invocar MARS y recoger resultado, salida, errores y código máquina. |
| IDE Flask | Editar, compilar, descargar y ejecutar el programa y las pruebas. |

El generador usa `op`, `arg1`, `arg2` y `result` de cada cuádruplo. La
representación textual TAC es una vista para lectura humana, no una entrada
que deba volver a tokenizarse.

## Organización del programa

Las rutinas se separan del flujo de nivel global antes de emitir `.text`.
Esto permite declarar funciones antes o después de sus llamadas y declarar
funciones dentro de otras sin ejecutar sus cuerpos durante la declaración.

Cada llamada crea una activación independiente. Los temporales y variables
locales de una llamada recursiva no comparten almacenamiento con los de la
activación que la invocó. La dirección de retorno y el estado de los registros
preservados forman parte de ese protocolo.

Los enlaces de acceso siguen la estructura léxica de funciones del programa.
El padre de definición puede ser diferente del llamante: una función hermana
o una llamada recursiva conservan el entorno exterior que les corresponde.
Una función declarada dentro de un método puede acceder a su `this` léxico sin
convertirse en un método con un argumento receptor adicional.

## Selección de instrucciones y asignación de registros

La selección cubre las familias de TAC documentadas en
[codigo-intermedio-tac.md](codigo-intermedio-tac.md): datos y expresiones,
saltos, funciones y retornos, listas, objetos, campos, métodos y operaciones
auxiliares del lenguaje.

El almacenamiento de respaldo en memoria conserva los valores cuando un
registro debe reutilizarse. Esta política favorece la corrección en las
llamadas y en los puntos donde convergen caminos de control. La asignación
física es independiente del reciclaje de nombres `%tN` que realiza el TAC:
un nombre temporal puede representar valores distintos en instrucciones
sucesivas.

El resultado expone `register_allocations`, `spills` e `instruction_count`.
Estas métricas describen el ensamblador generado; no son el número dinámico de
instrucciones ejecutadas por MARS.

## Compilación y errores

El backend solo se ejecuta cuando léxico, sintaxis, semántica y TAC terminaron
correctamente. Una omisión conserva `skipped` y una explicación en `reason`.
Un error de generación tiene código, categoría, mensaje, línea y columna.

Los resultados de compilación y ejecución son diferentes. Un programa puede
tener sintaxis y tipos válidos y encontrar un error al ejecutarse. Por eso el
IDE presenta por separado los diagnósticos del compilador y la salida del
simulador.

## Uso directo de MARS

### Preparación y flujo del proyecto

Después de compilar el núcleo C++ con el script correspondiente a tu sistema,
prepara el simulador:

```bash
python3 scripts/setup_mars.py
```

El JAR oficial se guarda en `tools/Mars.jar` y su contenido se verifica con un
SHA-256 conocido. Las variables `MARS_JAR` y `JAVA` permiten seleccionar una
instalación existente del simulador o del ejecutable Java.

Para guardar el ensamblador desde el CLI C++:

```bash
./compiscript_cli --file programa.cps --emit-mips -o programa.asm
```

La forma `./compiscript_cli --emit-mips programa.cps -o programa.asm` es
equivalente. Sin `--emit-mips`, el CLI conserva la salida JSON con los
resultados de todas las fases, incluido `mips.text`.

El adaptador de ejecución admite directamente Compiscript o ensamblador:

```bash
python3 scripts/run_mips.py programa.cps -o programa.asm
python3 scripts/run_mips.py programa.asm
python3 scripts/run_mips.py programa.cps --package programa.zip
python3 scripts/run_mips.py programa.cps --assemble-only --package programa.zip
```

`--json` muestra el reporte estructurado, `--max-steps N` modifica el límite de
instrucciones, entre 32 y 10 000 000, y `--delayed-branches` activa la opción correspondiente del
simulador. El reporte distingue `assembly` y `execution`, conserva `stdout`,
`stderr`, `returncode`, `timed_out` y `step_limit`, y marca `ok` según el
resultado real del proceso.

El ZIP de código máquina incluye:

| Archivo | Contenido |
|---|---|
| `program.asm` | Ensamblador del programa y su entorno auxiliar. |
| `program.text.bin` | Volcado binario del segmento de instrucciones ensambladas. |
| `program.data.bin` | Volcado binario del segmento de datos iniciales. |
| `manifest.json` | Arquitectura, simulador, direcciones de segmentos, tamaños y comando de ejecución. |
| `LEEME.txt` | Instrucciones para abrir y ejecutar el programa en MARS. |

Los binarios son volcados sin cabecera ELF. Para reproducir la ejecución se
usa `program.asm`, que permite a MARS reconstruir direcciones, datos y código
de forma coherente.

### Comandos del simulador

MARS 4.5 admite la ejecución sin interfaz gráfica. Para un ensamblador ya
generado, el comando de referencia es:

```bash
java -jar tools/Mars.jar nc sm ae65 se70 1000000 programa.asm
```

`nc` elimina el aviso inicial, `sm` comienza en la etiqueta global `main`,
`ae65` y `se70` asignan códigos de salida a errores de ensamblado y simulación,
y el entero limita los pasos ejecutados. Estas opciones provienen de la
[documentación oficial de línea de comandos de MARS](https://dpetersanderson.github.io/Help/MarsHelpCommand.html).

MARS permite exportar la memoria ensamblada, por ejemplo:

```bash
java -jar tools/Mars.jar nc a ae65 dump .text HexText programa.hex programa.asm
java -jar tools/Mars.jar nc a ae65 dump .text Binary programa.bin programa.asm
```

Un volcado del segmento `.text` contiene instrucciones máquina; el programa
completo también depende de sus datos y del entorno de ejecución. El paquete
de entrega conserva el ensamblador y la información necesaria para volver a
ensamblarlo y ejecutarlo en MARS.

La impresión utiliza servicios de MARS para enteros y dobles. Las cadenas se
conservan como bytes UTF-8 y se escriben al descriptor estándar de salida con
el servicio 15; esto evita que el servicio de impresión de cadenas interprete
cada byte UTF-8 como un carácter independiente. La reserva dinámica utiliza
el servicio de memoria. El contrato de estos
servicios está en la
[tabla oficial de syscalls](https://dpetersanderson.github.io/Help/SyscallHelp.html).

## Pruebas y criterio de aceptación

Se verifican tres niveles complementarios:

1. Las pruebas C++ de semántica y TAC validan entradas correctas e incorrectas
   y comprueban que una entrada inválida impida generar código de fases
   posteriores.
2. Las pruebas de ejecución generan ensamblador desde `.cps`, lo ejecutan en
   MARS y comparan su salida con un resultado esperado independiente.
3. Las pruebas de integración del IDE llaman las rutas Flask con el compilador
   y MARS reales y comprueban compilación, ejecución, errores y descarga de
   código máquina.

La batería C++ contiene 115 casos; la de ejecución MIPS, 66 casos —54 válidos y
12 errores esperados—; y la del IDE, 12. En total son 193 casos automatizados.

La cobertura de ejecución incluye expresiones, conversiones numéricas,
cortocircuito, caminos y ciclos, llamadas recursivas, argumentos y retornos,
valores que sobreviven a llamadas, funciones anidadas, cadenas, listas,
objetos, métodos y errores. Los casos negativos distinguen un diagnóstico
esperado de un fallo inesperado del ensamblador.

```bash
ctest --test-dir build/compiscript --output-on-failure
python3 backend/tests/mips_execution_tests.py
python3 backend/tests/mips_execution_tests.py --filter "Llamadas" --json
python3 backend/tests/ide_integration_tests.py
```

Las pruebas del IDE requieren instalar primero `requirements.txt` en el
entorno Python que se utiliza para ejecutar el comando. Se ejecutan por
separado de CTest.

CTest registra la prueba MIPS cuando al configurar CMake encuentra Python,
Java y `tools/Mars.jar`. Un JAR alternativo puede seleccionarse mediante
`-DCOMPISCRIPT_MARS_JAR=/ruta/Mars.jar`. Los scripts de construcción preparan
MARS antes de configurar CMake para incluir ambas baterías.

La prueba
[`algoritmo_compilador.cps`](../backend/examples/compiscript/algoritmo_compilador.cps)
adapta dos algoritmos del propio repositorio:

- `DFABuilder::bestRule`, en `backend/src/dfa_builder.cpp`: elige la regla
  aceptante de menor prioridad y utiliza cortocircuito para evitar una lectura
  fuera de rango.
- `TacGenerator::newTemp` y `releaseTemp`, en
  `backend/compiscript/src/tac_generator.cpp`: administra temporales vivos,
  una pila de libres y contadores de creación, reutilización y máximo vivo.

La adaptación emplea listas y una clase Compiscript. La pila del ejemplo tiene
capacidad fija de ocho posiciones porque el lenguaje no define `push`, `pop`
ni redimensionamiento de listas. Las referencias C++ y las decisiones de la
adaptación aparecen en comentarios del archivo.

```bash
python3 scripts/run_mips.py backend/examples/compiscript/algoritmo_compilador.cps -o algoritmo.asm --package algoritmo.zip
```

Su salida esperada es:

```text
2
-1
7
0
1
0
2
2
3
2
3
```

La prueba recorre el pipeline completo y compara esa salida con la ejecución
MIPS. Mantiene la sintaxis Compiscript como contrato de entrada; no constituye
autocompilación de los archivos C++ del compilador.

## Representación física de valores

El destino es MIPS32: las direcciones y las palabras son de 32 bits. Cada
variable, temporal, argumento y elemento de lista ocupa un slot de ocho bytes:
una palabra de carga útil seguida por una palabra de etiqueta de tipo. Esta
representación permite conservar el tipo efectivo cuando un nombre temporal
se recicla o una variable `unknown`, como la de `catch`, recibe un valor.

| Etiqueta | Tipo | Carga útil |
|---:|---|---|
| 0 | `null` | Cero. |
| 1 | `integer` | Entero con signo de 32 bits. |
| 2 | `boolean` | Cero o uno. |
| 3 | `string` | Dirección de una cadena terminada en cero. |
| 4 | `float` | Dirección de un doble de 64 bits alineado a ocho bytes. |
| 6 | Lista | Dirección de su cabecera. |
| 7 | Objeto | Dirección de su cabecera. |
| 8 | Función | Dirección de su código. |

Los dobles se almacenan por referencia para que el slot mantenga el mismo
tamaño que los demás valores. Los literales flotantes residen en `.data`;
los resultados calculados se alojan en memoria dinámica. La ampliación
`integer → float` se realiza al almacenar en un destino flotante y al pasar
parámetros o devolver valores de ese tipo.

Los literales enteros deben ser representables entre `-2147483648` y
`2147483647`; el mínimo negativo se reconoce como la negación de su literal
positivo. La suma, resta, multiplicación y negación enteras conservan los 32
bits bajos del resultado. Los dobles deben ser finitos: los literales fuera de
rango se diagnostican al compilar y los resultados no finitos se reportan al
ejecutar, en lugar de propagarse silenciosamente como infinito o NaN. La
división produce un resultado flotante y la división o el resto por cero
producen un error de ejecución capturable.

Las cadenas utilizan UTF-8, incluida la concatenación y la salida al
simulador. Las secuencias `\n`, `\r`, `\t`, `\0`, `\"` y `\\` se
decodifican al construir los literales; el terminador cero delimita su contenido
visible como cadena en el entorno de ejecución.

Las listas comienzan con una cabecera de ocho bytes: longitud en el offset
`0` y una palabra reservada en `4`. El elemento `i` se encuentra en
`base + 8 + 8*i`. Una lista anidada almacena referencias a otras listas.

Los objetos tienen una cabecera de ocho bytes, con una referencia al nombre
de clase en `0`. Un campo está en `base + 8 + offset_TAC`; los campos de la
clase base preceden a los de la clase derivada. Los inicializadores de campos
se ejecutan para cada instancia, antes de invocar su constructor.

La resolución de métodos sigue el símbolo determinado por el análisis
semántico y utiliza llamadas directas. La herencia conserva campos y búsqueda
de miembros; esta convención utiliza despacho estático, sin tabla virtual.
Las funciones anidadas acceden a los frames léxicos mientras esos frames están
activos. La gramática y el sistema semántico no definen funciones de primera
clase con un entorno que escape de su activación.

La memoria dinámica se obtiene del simulador y se conserva durante la
ejecución del programa; esta fase no incorpora un recolector de basura.

## Convención de llamadas

Cada rutina reserva un frame físico alineado a ocho bytes. `$fp` apunta a su
inicio y `$sp` delimita el espacio usado por la activación y las llamadas que
realiza. La cabecera ocupa 64 bytes:

| Offset desde `$fp` | Contenido |
|---:|---|
| 0 | `$fp` anterior: enlace de control. |
| 4 | Frame del padre léxico: enlace de acceso. |
| 8 | Dirección de retorno `$ra`. |
| 12–40 | Valores preservados de `$s0` a `$s7`. |
| 44 | Manejador de errores activo al entrar en la rutina. |
| 48 | Carga útil del receptor `this`, cuando existe. |
| 52 | Etiqueta del receptor. |
| 56–63 | Espacio reservado de la cabecera. |
| 64 en adelante | Slots de parámetros locales, variables y temporales. |

El llamante evalúa los argumentos de izquierda a derecha y conserva cada
resultado antes de evaluar el siguiente. Reserva `8*n` bytes en la pila y
escribe allí los slots en orden. En los métodos, el receptor precede a los
argumentos explícitos.

Antes del salto, el llamante coloca en `$a0` el frame del padre léxico de la
rutina destino y utiliza `jal`. El llamado reserva su frame, preserva los
registros y copia sus parámetros desde `$fp + frame_size + 8*i`; el índice
incluye el receptor implícito cuando corresponde.

El retorno utiliza `$v0` para la carga útil y `$v1` para la etiqueta. El
epílogo restaura el manejador de errores de entrada, `$sp`, `$ra`, los registros
guardados y `$fp`, y retorna con `jr $ra`. Después, el llamante retira el área
de argumentos. Las llamadas recursivas siguen el mismo protocolo y tienen
slots independientes.

El campo `parameter_bytes` describe el área de argumentos entrantes. Esa área
es propiedad del llamante y se encuentra fuera del `frame_size` reservado por
el llamado. `saved_register_bytes` representa los 64 bytes de cabecera de esta
convención, que incluyen enlaces y receptor además de los registros guardados.

## Política concreta de `getReg`

El generador dispone de tres pares para la caché de valores:
`$s1/$s2`, `$s3/$s4` y `$s5/$s6`. El primer registro de cada par contiene la
carga útil y el segundo su etiqueta. `$s0` conserva el receptor; los `$t` se
usan como registros de trabajo en direccionamiento y rutinas auxiliares.

`getReg` reutiliza un par que ya contiene el valor, elige uno vacío o
selecciona el menos recientemente usado. Si el par elegido tiene un resultado
modificado, emite un spill a su slot estable antes de reutilizarlo. Los valores
se vuelcan y los descriptores se invalidan antes de llamadas y en las fronteras
del control de flujo, de modo que la memoria refleja los resultados que deben
estar disponibles después.

Esta es una asignación conservadora con respaldo en pila. No intenta una
asignación global por coloreo de grafos; la evaluación del proyecto permite el
uso de pila. Las métricas permiten observar los guardados efectivos y las
selecciones de pares de registros.

## Relación con la evaluación

| Componente del enunciado | Evidencia |
|---|---|
| Llamadas y retornos — 20 puntos | Frames físicos, protocolo de parámetros y retornos, preservación de estado y pruebas recursivas. |
| Asignación de registros — 20 puntos | Selección de registros, almacenamiento en pila y métricas observables. |
| Código MIPS — 60 puntos | Ensamblador generado desde TAC, ejecución real en MARS y comparación automatizada de resultados y errores. |
| IDE y documentación | Edición, compilación, descarga, ejecución, reportes y procedimientos reproducibles. |

## Referencias de diseño

Se consultaron las siguientes secciones de *Compilers: Principles, Techniques,
and Tools*, segunda edición, disponible como material local en
`Instrucciones/`:

- 7.2.2: contenido de los registros de activación.
- 7.2.3: responsabilidades del llamante y del llamado.
- 7.3.5–7.3.6: enlaces de acceso y llamadas a funciones anidadas, hermanas y
  recursivas.
- 8.6.1–8.6.3: descriptores de registros y direcciones, generación de
  instrucciones y selección de registros con `getReg` y spills.

La implementación concreta y sus convenciones están definidas en este
repositorio. El libro se utiliza como referencia conceptual y no se copia ni
redistribuye como parte de la documentación entregable.
