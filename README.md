# ARCADE RESCUE — arcade-rescue-starter

> **Estado del proyecto: heredado.** El estudio anterior lo dejó a medias.
> Compila "a veces", nadie sabe qué versión está publicada y no existe pipeline.

Snake en C++17 con raylib. Este repositorio es el punto de partida de la
actividad **ARCADE RESCUE: de build roto a release**. El objetivo no es reescribir
el juego: es **rescatarlo** y convertirlo en un producto que se compile, se pruebe,
se empaquete y se distribuya de forma automática.

Lee el documento de la actividad que te entregará el instructor antes de tocar
código: ahí están el cronograma, los entregables y la rúbrica.

---

## 1. Qué hay en el repositorio

```
.
├── CMakeLists.txt        # build del proyecto
├── VERSION               # version del producto
├── Dockerfile            # imagen de contenedor (heredada)
├── assets/               # fuente pixel y icono
├── include/arcade/       # cabeceras del nucleo y del frontend
├── src/                  # logica del juego + frontends
└── tests/                # arnes de pruebas y un par de pruebas iniciales
```

- `include/arcade/game.hpp` y `src/game.cpp`: reglas del juego (sin gráficos).
- `src/frontend_raylib.cpp`: ventana, input y dibujo con raylib.
- `src/frontend_console.cpp`: versión de terminal.
- `src/app.cpp`: bucle de simulación, clock de ticks y modo `--headless`.
- `src/main.cpp`: argumentos de línea de comandos.

El binario soporta `--headless`, que simula partidas sin abrir ventana. Sirve para
CI y para contenedores sin GPU.

## 2. Cómo empezar

Requisitos: CMake ≥ 3.20, compilador con C++17, Git. raylib se descarga sola
durante la configuración (necesita conexión la primera vez).

```bash
# (comando heredado del README anterior)
g++ -std=c++17 -O2 src/main.cpp -o arcade_rescue
./arcade_rescue
```

Si ese comando no funciona, revisa cómo está definido el build en `CMakeLists.txt`.
El proyecto está pensado para compilarse con CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

En Windows con Visual Studio:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Ejecución:

```bash
./build/arcade_rescue            # ventana (raylib)
./build/arcade_rescue --console  # terminal
./build/arcade_rescue --headless --ticks 300 --seed 7
```

Controles: **WASD**/flechas, **ESPACIO** iniciar, **P** pausa, **R** reiniciar,
**ESC** salir. Reglas: +10 puntos por comida, una celda de crecimiento por comida,
victoria al alcanzar la meta de comidas, derrota al chocar con el borde o con el
propio cuerpo.

## 3. Docker

```bash
docker build -t arcade-rescue .
docker run --rm arcade-rescue
```

El Dockerfile heredado no está pensado para CI ni para ejecutarse sin pantalla.
Justifica técnicamente qué debe hacer el contenedor: compilar, probar y ejecutar
un smoke test headless es una opción razonable; jugar con ventana dentro del
contenedor no lo es.

## 4. Pruebas

Hay un arnés mínimo sin dependencias externas en `tests/` con dos pruebas de
humo. **No está enganchado al build**: `ctest` no ejecuta nada todavía.

El equipo debe:

1. Enganchar la carpeta de pruebas al build.
2. Escribir al menos **tres pruebas automatizadas** propias.
3. Conseguir que al menos una de ellas **falle con el código heredado** y pase
   tras la corrección.

## 5. Informe de QA (heredado)

Estos son los síntomas que reportaron los jugadores antes de que el estudio
cerrara. No hay reproducción paso a paso ni stack traces: ese trabajo es tuyo.

- "En una máquina limpia el build no termina."
- "A veces la comida aparece donde ya está la serpiente."
- "El marcador no cuadra con las comidas que llevo."
- "La partida no termina cuando debería."
- "Al ejecutar desde la carpeta de build, la fuente pixel no carga."
- "`ctest` dice que no hay pruebas."

## 6. Reglas de la actividad

- No cambies las reglas del juego para que las pruebas pasen: corrige la causa.
- Los workflows de GitHub Actions se construyen **desde cero**; no se entregan.
- No subas secretos, tokens ni credenciales al repositorio.
- No subas artefactos de build (`build/`, binarios, ZIPs).
- Cada corrección debe tener un commit con un mensaje que explique el *porqué*.

## 7. Entregables (resumen)

Repositorio propio en GitHub con: juego funcional, lint y pruebas, workflows de
CI con matrix Linux/Windows, artifacts descargables, release tagueada, imagen en
GHCR, página en GitHub Pages y un documento de investigación sobre una
herramienta que no se haya visto en clase.

La rúbrica completa y el detalle de cada entregable están en el documento de la
actividad.

---

Licencia MIT. Fuente *Press Start 2P* bajo SIL Open Font License 1.1
(`assets/fonts/OFL.txt`).
