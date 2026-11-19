# Plan de equipo — Arcade Rescue (210 min)

Prioridad si falta tiempo: **build → pruebas → CI → release → Docker → Pages**.

## Roles y archivos propios

| Persona | Rol | Archivos propios |
|---|---|---|
| A: ________ | Build master y CI | `CMakeLists.txt`, `.github/workflows/ci.yml`, `.clang-format`, `.clang-tidy` |
| B: ________ | QA y Game Rescue | `src/`, `include/`, `tests/`, `docs/investigacion.md` |
| C: ________ | Release, Docker y Pages | `Dockerfile`, `.github/workflows/release.yml`, `docker.yml`, `pages.yml`, `docs/` (sitio), `README.md` |

Reglas de Git:
- Una rama por tarea (`fix/...`, `ci/...`, `docker/...`) y PR a `main`.
- Un commit = una intención (`fix:`, `ci:`, `docs:`, `test:`).
- No editar archivos de otra persona sin avisar.
- Sin `build/` ni secretos en el repo. Los workflows se escriben a mano (regla de la actividad).
- Más de 10 min atascado: rotar roles o pedir pista.

## Cronograma

| Min | A (CI) | B (QA/juego) | C (Release/Docker/Pages) |
|---|---|---|---|
| 0–15 | **Todos:** clonar, compilar tal cual, anotar síntomas. A crea repo + `.gitignore` y sube el estado inicial. | Lista de síntomas | Lee el Dockerfile y guarda sus errores |
| 15–55 | Workflow de 1 job (lint+test+build, Linux) | Reproducir y corregir bugs, 1 commit por bug | Reparar Dockerfile (build + smoke test headless); esqueleto de `docs/` |
| 55–85 | clang-format / clang-tidy en CI | 3+ pruebas; una falla con el código heredado (guardar evidencia del rojo) | Workflow de publicación en GHCR (`packages: write`) |
| 85–120 | Matrix Linux/Windows + **run rojo provocado** | Arreglar fallos de Windows; empezar investigación | Workflow de release; artifacts por plataforma (fallar si vacío) |
| 120–145 | Revisar checks, proteger `main` | Investigación | Tag anotado `v1.0.0` + release con binarios y notas |
| 145–170 | Descargar artifact y ejecutarlo fuera del runner | Terminar investigación y su demo | `docker pull` de prueba; activar Pages |
| 170–185 | **Todos:** README, declaración de uso de IA, enlaces en ventana privada | | |
| 185–210 | **Todos:** ensayo cronometrado de la demo | | |

## Checklist por persona

### A — Build master y CI
- [ ] Repo `arcade-rescue` creado, `.gitignore` antes del primer commit
- [ ] Commit inicial "chore: estado inicial del proyecto heredado"
- [ ] CI en `push` y `pull_request`, con lint, test y build separados/nombrados
- [ ] Matrix Linux + Windows (dos jobs verdes)
- [ ] Un run rojo provocado en una rama y luego revertido
- [ ] clang-format versionado; clang-tidy (o alternativa justificada)
- [ ] Artifact descargado y ejecutado en otra máquina

### B — QA y Game Rescue
- [ ] Síntomas con pasos de reproducción (puntaje vs HUD, cierres, build de contenedor)
- [ ] Un commit `fix:` por síntoma, con PR
- [ ] 3+ pruebas con `ctest`, deterministas (sin red, GUI ni reloj)
- [ ] Una prueba que falla con el código heredado y pasa tras el fix (guardar evidencia)
- [ ] `docs/investigacion.md` con las 7 preguntas y evidencia (sugerida: ASan/UBSan)

### C — Release, Docker y Pages
- [ ] Dockerfile reparado y documentado, smoke test con código de salida 0
- [ ] Imagen en GHCR etiquetada por versión; `docker pull` funciona
- [ ] Artifacts con nombre por plataforma
- [ ] Tag anotado `v1.0.0`, release con binarios (Linux y Windows) y notas legibles
- [ ] Pages público: qué es, cómo se juega, cómo se compila, descargas, enlaces a repo/release/GHCR, créditos
- [ ] README con build, ejecución, pruebas, distribución, badges y uso de IA declarado

## Definición de terminado (los 8 son obligatorios)
1. Build limpio en Linux y Windows
2. Lint en el pipeline
3. 3+ pruebas, una falla con el código heredado
4. Artifacts descargables de un run
5. Release `v1.0.0` con binarios y notas
6. Imagen en GHCR
7. Pages publicado con enlaces funcionales
8. Demo de 5 min ensayada

## Demo (el que habla no navega)
- A: pipeline y run rojo · B: bug concreto + prueba + investigación · C: release, GHCR y Pages
- Pestañas abiertas antes: Actions (verde y rojo), release, GHCR, Pages, `investigacion.md`, terminal
- Preguntas probables: por qué esa prueba, run rojo y su arreglo, dónde vive el token, artifact vs release, por qué la imagen no ejecuta la GUI, qué pasa si cae el runner de Windows

## Retos opcionales (tope +10)
Self-hosted +2 · cobertura +2 · sanitizers +2 · firma +2 · cache +1 · multi-stage +1 · Dependabot +1 · release notes auto +1 · Pages por tag +1
