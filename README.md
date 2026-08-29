# Generadores YALex y YAPar

Proyecto académico desarrollado durante **Compiladores 1** para construir, desde
cero, generadores de analizadores léxicos y sintácticos. El repositorio reúne
dos entregas consecutivas del curso:

1. Un generador de analizadores léxicos basado en especificaciones YALex.
2. Un generador de analizadores sintácticos LL(1), SLR(1) y LALR basado en
   especificaciones YAPar.

Ambas etapas están integradas en una interfaz web tipo IDE que permite editar
especificaciones, construir los analizadores, visualizar sus estructuras y
evaluar cadenas o archivos de entrada.

> Esta rama conserva el cierre de **Compiladores 1**. El trabajo de
> Compiladores 2 se desarrolla en la rama `compiladores-2`.

## Contenido

- [Proyecto 1: generador léxico YALex](#proyecto-1-generador-léxico-yalex)
- [Proyecto 2: generador sintáctico YAPar](#proyecto-2-generador-sintáctico-yapar)
- [Pipeline integrado](#pipeline-integrado)
- [Interfaz gráfica](#interfaz-gráfica)
- [Estructura del repositorio](#estructura-del-repositorio)
- [Compilación y ejecución](#compilación-y-ejecución)
- [Ejemplos](#ejemplos)
- [Pruebas realizadas](#pruebas-realizadas)
- [Alcance y limitaciones](#alcance-y-limitaciones)
- [Historial del proyecto](#historial-del-proyecto)

## Proyecto 1: generador léxico YALex

La primera etapa implementa un generador de analizadores léxicos inspirado en
Lex y `ocamllex`. Recibe un archivo `.yal`, interpreta sus definiciones
regulares y genera un analizador léxico en C++.

### Entrada

Una especificación YALex con la forma general:

```text
{ header opcional }

let identificador = expresion_regular

rule punto_de_entrada =
    expresion_regular { accion }
  | expresion_regular { accion }

{ trailer opcional }
```

El lenguaje implementado admite:

- Comentarios `(* ... *)`.
- Definiciones reutilizables mediante `let`.
- Caracteres y cadenas constantes.
- Clases y rangos de caracteres.
- Complemento de conjuntos.
- Alternancia y concatenación.
- Cerraduras `*`, `+` y operador opcional `?`.
- Comodín `_`.
- Prioridad por orden de declaración.
- Selección del lexema más largo.
- Bloques de acciones escritos en C++.

Ejemplo simplificado:

```text
let letter = ['A'-'Z''a'-'z''_']
let digit  = ['0'-'9']
let alnum  = ['A'-'Z''a'-'z''0'-'9''_']

rule tokens =
    [' ''\t''\n''\r']+ { }
  | letter alnum*       { std::cout << "<id, \"" << lxm << "\">\n"; }
  | '+'                 { std::cout << "<OP_PLUS, \"+\">\n"; }
```

### Construcción del analizador léxico

El generador realiza las siguientes fases:

1. Lee la especificación y expande las definiciones `let`.
2. Convierte cada expresión regular en un árbol sintáctico.
3. Combina las reglas y agrega marcadores de aceptación por prioridad.
4. Calcula `nullable`, `firstpos`, `lastpos` y `followpos`.
5. Construye directamente el AFD.
6. Minimiza el AFD mediante refinamiento de particiones.
7. Serializa el autómata en JSON.
8. Genera código fuente C++ para el lexer resultante.
9. Produce archivos DOT para visualizar el árbol y el autómata.

### Salidas

- Código fuente del lexer generado.
- Autómata persistido en JSON.
- Árbol de expresión en formato DOT.
- AFD en formato DOT.
- Tokens reconocidos con lexema, línea y columna.
- Reportes de errores léxicos.

Los módulos principales son `yalex_parser`, `regex_parser`, `regex_node`,
`dfa_builder`, `automata_serializer`, `code_gen` y `dot_gen`.

## Proyecto 2: generador sintáctico YAPar

La segunda etapa extiende el lexer con un generador de analizadores sintácticos
inspirado en Yacc y `ocamlyacc`. Recibe una gramática `.yalp` cuyos terminales
deben corresponder con los tokens producidos por YALex.

### Entrada

Una especificación YAPar con declaraciones de tokens, tokens ignorados y
producciones:

```text
%token id OP_PLUS OP_MUL LPAREN RPAREN
IGNORE WS

%%

expr:
    term expr_tail
;

expr_tail:
    OP_PLUS term expr_tail
  |
;
```

En una producción:

- Los nombres declarados con `%token` son terminales.
- Los nombres de producciones son no terminales.
- `|` separa alternativas.
- `;` termina una producción.
- Una alternativa vacía representa epsilon.
- `IGNORE` permite excluir tokens antes del parsing.

### Algoritmos implementados

#### FIRST y FOLLOW

Se calculan iterativamente hasta alcanzar un punto fijo y se utilizan tanto en
la tabla predictiva LL(1) como en las reducciones SLR(1).

#### LL(1)

- Eliminación de recursión izquierda directa e indirecta.
- Factorización izquierda.
- Construcción de la tabla predictiva.
- Detección de conflictos.
- Parser descendente dirigido por tabla.

#### SLR(1)

- Aumento de la gramática.
- Construcción de la colección canónica LR(0).
- Operaciones `closure` y `goto`.
- Construcción de tablas ACTION/GOTO usando FOLLOW.
- Detección de conflictos shift/reduce y reduce/reduce.
- Parser LR dirigido por tabla.

#### LALR

- Construcción de estados LR(1) con lookaheads.
- Agrupación de estados con el mismo núcleo LR(0).
- Fusión de lookaheads.
- Construcción de ACTION/GOTO para los estados fusionados.
- Uso del mismo motor LR empleado por SLR.

### Resultados del análisis

Los parsers producen:

- Aceptación o rechazo de la entrada.
- Errores sintácticos con ubicación y símbolos esperados.
- Recuperación básica para continuar después de ciertos errores.
- Traza paso a paso de la pila, entrada y acción aplicada.
- Árbol de derivación o CST.
- Tablas de parsing y conflictos encontrados.

## Pipeline integrado

La aplicación conecta las dos etapas de la siguiente manera:

```text
Archivo .yal
    ↓
Parser YALex → árbol de expresión → AFD minimizado
    ↓
Flujo de tokens
    ↓
Archivo .yalp → gramática → FIRST/FOLLOW → tabla seleccionada
    ↓
Parser LL(1), SLR(1) o LALR
    ↓
Resultado + traza + árbol de derivación
    ↓
JSON → interfaz Flask
```

El ejecutable `yapar_cli` funciona como adaptador entre el backend C++ y la
aplicación Flask. Construye el pipeline solicitado y devuelve un documento JSON
que el frontend utiliza para crear las visualizaciones.

El repositorio también contiene un avance experimental de análisis semántico
con CST, AST, tipos, ámbitos y clases. Su explicación técnica está en
[docs/avance-semantico.md](docs/avance-semantico.md). El nuevo proyecto de
Compiscript no se implementará sobre esta gramática experimental: tendrá su
propio pipeline en `compiladores-2`.

## Interfaz gráfica

La interfaz funciona como un IDE educativo y permite:

- Seleccionar archivos `.yal` y `.yalp`.
- Editar y guardar las especificaciones.
- Elegir LL(1), SLR(1) o LALR.
- Construir las tablas correspondientes.
- Visualizar tokens y expresiones regulares.
- Consultar producciones, FIRST y FOLLOW.
- Inspeccionar estados LR(0) o LR(1) fusionados.
- Visualizar ACTION/GOTO o la tabla predictiva LL(1).
- Analizar entradas manuales o archivos.
- Consultar la traza completa del parser.
- Visualizar el árbol de derivación.
- Mostrar errores y ubicación en el código.

## Estructura del repositorio

```text
.
├── app.py                         # Servidor Flask
├── frontend/
│   ├── templates/index.html       # IDE
│   └── static/
│       ├── css/style.css
│       └── js/app.js
├── backend/
│   ├── src/                       # Generadores y parsers en C++
│   ├── examples/                  # Especificaciones y entradas de prueba
│   ├── tests/                     # Pruebas del avance semántico
│   └── build_yapar_cli.ps1        # Compilación en Windows
├── docs/                          # Documentación técnica complementaria
└── build/                         # Artefactos generados, ignorados por Git
```

## Compilación y ejecución

### Requisitos

- Compilador compatible con C++17.
- Python 3.
- Flask.
- Opcional: Graphviz para renderizar archivos DOT.

### macOS o Linux

```bash
chmod +x backend/build_yapar_cli.sh
./backend/build_yapar_cli.sh
python3 -m flask --app app run --port 5050
```

### Windows PowerShell

```powershell
.\backend\build_yapar_cli.ps1
python app.py
```

Después se puede abrir:

```text
http://localhost:5050
```

### Uso directo del CLI

```bash
./yapar_cli \
  --yal backend/examples/aritmetica.yal \
  --yalp backend/examples/aritmetica.yalp \
  --method lalr \
  --input "x + y * z"
```

Los métodos disponibles son `ll1`, `slr` y `lalr`.

## Ejemplos

El directorio `backend/examples` contiene:

- Una gramática de expresiones aritméticas.
- Una gramática SLR de asignaciones y expresiones.
- Un lexer y una gramática para un subconjunto de C++.
- Entradas C++ válidas y con errores.
- Una gramática del avance semántico experimental.

Ejemplo válido:

```text
x + y * z
```

Ejemplo sintácticamente inválido:

```text
x + * y
```

## Pruebas realizadas

Durante la consolidación de esta rama se verificó:

- Compilación completa del CLI con advertencias habilitadas.
- Aceptación de expresiones aritméticas con LL(1), SLR y LALR.
- Construcción del árbol para los tres métodos.
- Aceptación del programa C++ válido con SLR y LALR.
- Detección de un error sintáctico en el ejemplo C++ inválido.
- Ejecución de los casos válidos e inválidos del avance semántico.

## Alcance y limitaciones

Este proyecto es un generador educativo, no un reemplazo completo de Lex,
Yacc, ANTLR o un compilador de C++ real. Entre las limitaciones conocidas:

- La integración web debe fortalecer el reporte de caracteres léxicos no
  reconocidos.
- Las posiciones del tokenizador integrado requieren normalización para
  señalar siempre el inicio del lexema.
- El operador de diferencia de expresiones regulares necesita completar su
  semántica de conjuntos.
- La correspondencia entre tokens YALex y YAPar aún puede validarse de manera
  más estricta.
- El subconjunto C++ no pretende cubrir toda la especificación del lenguaje.
- La recuperación de errores es deliberadamente básica.

Estas limitaciones se documentan para distinguir el alcance comprobado de las
extensiones futuras.

## Historial del proyecto

- `main`: versión estable general del repositorio.
- `compiladores-1`: cierre documentado de YALex y YAPar.
- `compiladores-2`: desarrollo de ANTLR, Compiscript y análisis semántico.
- `compiladores-1-final`: etiqueta del cierre de Compiladores 1.

La rama de Compiladores 1 debe permanecer disponible para consultar el trabajo
original sin mezclarlo con la implementación posterior de Compiscript.

## Autores

Proyecto desarrollado como trabajo grupal del curso. Las contribuciones se
mantienen identificadas mediante el historial individual de commits y el
archivo [CONTRIBUTORS.md](CONTRIBUTORS.md).
