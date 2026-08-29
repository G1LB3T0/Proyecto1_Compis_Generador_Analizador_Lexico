# YALex + YAPar

Generador educativo de analizadores léxicos y sintácticos en C++ con interfaz Flask. El pipeline soporta DFA directo/minimizado, FIRST/FOLLOW, LR(0), SLR(1), LALR(1), LL(1), trazas y recuperación básica de errores.

## Avance semántico actual

El repositorio ahora puede extraer tipos, ámbitos, símbolos y clases para el subconjunto definido en `backend/examples/semantica.yal` y `semantica.yalp`. La fase está habilitada para SLR y LALR y se entrega en la clave JSON `semantic`.

La explicación completa, ejemplos, contrato JSON, compilación, pruebas y limitaciones están en [docs/avance-semantico.md](docs/avance-semantico.md). La ruta de trabajo posterior está en [plan.md](plan.md).

## Compilación rápida en Windows

```powershell
.\backend\build_yapar_cli.ps1
python app.py
```

La aplicación Flask escucha por defecto en `http://localhost:5050`.
