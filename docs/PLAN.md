<div align="center">

# 🕹️ Arcade Rescue
### De build roto a release

![Duración](https://img.shields.io/badge/duraci%C3%B3n-210%20min-blue)
![Equipo](https://img.shields.io/badge/equipo-3%20personas-orange)
![Lenguaje](https://img.shields.io/badge/C%2B%2B-17-00599C)
![CI](https://img.shields.io/badge/CI-GitHub%20Actions-2088FF)
![Meta](https://img.shields.io/badge/meta-v1.0.0-brightgreen)

**Prioridad:** `build` → `pruebas` → `CI` → `release` → `Docker` → `Pages`

</div>

> [!IMPORTANT]
> Los workflows se escriben **a mano** y se tienen que poder explicar. Sin evidencia en GitHub (runs, artifacts, releases, paquetes, URLs), no cuenta.

---

## 👥 Equipo

| | Persona | Rol | Archivos propios |
|:-:|---|---|---|
| 🔵 | **A:** ________ | Build master y CI | `CMakeLists.txt` · `ci.yml` · `.clang-format` · `.clang-tidy` |
| 🟠 | **B:** ________ | QA y Game Rescue | `src/` · `include/` · `tests/` · `docs/investigacion.md` |
| 🟢 | **C:** ________ | Release, Docker y Pages | `Dockerfile` · `release.yml` · `docs/` · `README.md` |

**Reglas de Git:** una rama por tarea (`fix/…`, `ci/…`, `docker/…`) · un commit = una intención (`fix:`, `ci:`, `docs:`, `test:`) · no editar archivos ajenos sin avisar · sin `build/` ni secretos en el repo.

---

## ⏱️ Cronograma (210 min)

```mermaid
gantt
    dateFormat  YYYY-MM-DD HH:mm
    axisFormat  %H:%M
    title Trabajo en paralelo (minutos)
    section 🔵 A · CI
    Recon y repo           :a1, 2026-01-01 00:00, 15m
    CI de 1 job            :a2, 2026-01-01 00:15, 40m
    Lint en CI             :a3, 2026-01-01 00:55, 30m
    Matrix + run rojo      :a4, 2026-01-01 01:25, 35m
    Checks y protección    :a5, 2026-01-01 02:00, 25m
    Probar artifact        :a6, 2026-01-01 02:25, 25m
    section 🟠 B · QA
    Síntomas               :b1, 2026-01-01 00:00, 15m
    Game Rescue            :b2, 2026-01-01 00:15, 40m
    3+ pruebas             :b3, 2026-01-01 00:55, 30m
    Fix Windows + investig :b4, 2026-01-01 01:25, 35m
    Investigación          :b5, 2026-01-01 02:00, 50m
    section 🟢 C · Release
    Lee Dockerfile         :c1, 2026-01-01 00:00, 15m
    Repara Dockerfile      :c2, 2026-01-01 00:15, 40m
    GHCR                   :c3, 2026-01-01 00:55, 30m
    Release workflow       :c4, 2026-01-01 01:25, 35m
    Tag v1.0.0 + release   :c5, 2026-01-01 02:00, 25m
    Pages                  :c6, 2026-01-01 02:25, 25m
    section 👥 Todos
    README + enlaces       :t1, 2026-01-01 02:50, 15m
    Ensayo de demo         :t2, 2026-01-01 03:05, 25m
```

<details>
<summary><b>Ver el detalle por tramo</b></summary>

| Min | 🔵 A | 🟠 B | 🟢 C |
|---|---|---|---|
| 0–15 | **Todos:** clonar, compilar tal cual, anotar síntomas | Lista de síntomas | Lee el Dockerfile y guarda errores |
| 15–55 | Workflow de 1 job (lint+test+build, Linux) | Corregir bugs, 1 commit por bug | Reparar Dockerfile; esqueleto de `docs/` |
| 55–85 | clang-format / clang-tidy en CI | 3+ pruebas; una falla con el código heredado | Workflow de GHCR |
| 85–120 | Matrix Linux/Windows + **run rojo provocado** | Fallos de Windows; empezar investigación | Workflow de release y artifacts |
| 120–145 | Checks y protección de `main` | Investigación | Tag `v1.0.0` y release |
| 145–170 | Descargar artifact y ejecutarlo fuera del runner | Terminar investigación | `docker pull` de prueba; activar Pages |
| 170–185 | **Todos:** README, uso de IA declarado, enlaces en ventana privada | | |
| 185–210 | **Todos:** ensayo cronometrado de la demo | | |

</details>

> [!TIP]
> Más de 10 minutos atascado: rotar roles o pedir pista.

---

## ✅ Checklists

> Las casillas **marcables con el clic** están en los [Issues del repo](../../issues). Aquí es el resumen.

<details open>
<summary><b>🔵 Persona A · Build master y CI</b></summary>

- [ ] Repo con `.gitignore` y commit inicial «chore: estado inicial del proyecto heredado»
- [ ] CI en `push` y `pull_request` (lint, test y build separados)
- [ ] Matrix Linux + Windows, dos jobs verdes
- [ ] Run rojo provocado y luego revertido
- [ ] `clang-format` versionado; `clang-tidy` o alternativa justificada
- [ ] Artifact descargado y ejecutado en otra máquina

</details>

<details open>
<summary><b>🟠 Persona B · QA y Game Rescue</b></summary>

- [ ] Síntomas con pasos de reproducción
- [ ] Un commit `fix:` por síntoma, con PR
- [ ] 3+ pruebas con `ctest`, deterministas
- [ ] Una prueba falla con el código heredado y pasa tras el fix (guardar evidencia)
- [ ] `docs/investigacion.md` con las 7 preguntas
- [ ] Herramienta implementada (sugerida: ASan/UBSan)

</details>

<details open>
<summary><b>🟢 Persona C · Release, Docker y Pages</b></summary>

- [ ] Dockerfile reparado, smoke test con salida 0
- [ ] Imagen en GHCR etiquetada por versión; `docker pull` funciona
- [ ] Artifacts con nombre por plataforma
- [ ] Tag anotado `v1.0.0` y release con binarios y notas
- [ ] Pages público con instrucciones, descargas y créditos
- [ ] README con build, pruebas, distribución, badges y uso de IA

</details>

---

## 🏁 Definición de terminado

> [!CAUTION]
> Los 8 son obligatorios. Si falta uno, la entrega está incompleta.

| # | Requisito | Dueño |
|:-:|---|:-:|
| 1 | Build limpio en Linux y Windows | 🔵 |
| 2 | Lint en el pipeline | 🔵 |
| 3 | 3+ pruebas, una falla con el código heredado | 🟠 |
| 4 | Artifacts descargables de un run | 🔵 |
| 5 | Release `v1.0.0` con binarios y notas | 🟢 |
| 6 | Imagen en GHCR | 🟢 |
| 7 | Pages publicado con enlaces funcionales | 🟢 |
| 8 | Demo de 5 minutos ensayada | 👥 |

---

## 🎤 Demo

**Reparto:** 🔵 pipeline y run rojo · 🟠 un bug concreto + su prueba + investigación · 🟢 release, GHCR y Pages. *Quien habla no navega.*

**Pestañas abiertas antes de empezar:** Actions (verde y rojo) · release `v1.0.0` · paquete GHCR · Pages · `investigacion.md` · terminal.

<details>
<summary><b>Preguntas probables del profesor</b></summary>

1. ¿Por qué esa prueba y qué bug atrapa?
2. Muestra un run rojo y su arreglo.
3. ¿Dónde vive el token y por qué no está en el repo?
4. ¿Diferencia entre artifact y release?
5. ¿Por qué la imagen no ejecuta la GUI?
6. ¿Qué pasa si cambia la versión de una dependencia?
7. ¿Qué se pierde si cae el runner de Windows?

</details>

---

## ⭐ Retos opcionales (tope +10)

| Reto | Pts | Reto | Pts |
|---|:-:|---|:-:|
| Runner self-hosted | +2 | Docker multi-stage | +1 |
| Cobertura (gcovr) | +2 | Dependabot | +1 |
| Sanitizers | +2 | Release notes automáticas | +1 |
| Firma de artifacts | +2 | Cache de dependencias | +1 |
| Pages disparado por tag | +1 | | |
