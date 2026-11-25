# Investigación: Sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)

> Responsable: Claudio (🟠 B · QA y Game Rescue)
> Herramienta elegida: **ASan + UBSan** de GCC/Clang, activados con la opción
> `ARCADE_RESCUE_SANITIZERS` que ya existe en `CMakeLists.txt`.

## Evidencia (enlaces de GitHub)

| Qué | Enlace |
|---|---|
| Run del job de sanitizers en verde (código actual) | _pendiente: pegar URL del run_ |
| Run en rojo provocado (bug de memoria a propósito) | _pendiente: pegar URL del run_ |
| Commit que provoca el rojo | _pendiente_ |
| Commit que lo revierte | _pendiente_ |

---

## 1. ¿Qué es y qué problema resuelve?

Los sanitizers son **instrumentación que el compilador inserta en el binario**
para vigilar en tiempo de ejecución cosas que C++ no comprueba:

- **AddressSanitizer (ASan)**: lecturas/escrituras fuera de un array o
  `std::vector` (heap y stack), uso de memoria ya liberada (*use-after-free*),
  doble `delete` y fugas de memoria.
- **UndefinedBehaviorSanitizer (UBSan)**: comportamiento indefinido, como
  desbordamiento de enteros con signo, desplazamientos inválidos, divisiones
  por cero o conversiones fuera de rango.

El problema que resuelven: en C++ esos errores **no siempre fallan**. El
programa puede seguir funcionando «de casualidad» en una máquina y romperse
en otra (o en Windows sí y en Linux no). Con el sanitizer, el error se
detecta **en el momento exacto** en que ocurre, con la línea y la pila de
llamadas, y el proceso termina con código distinto de cero, así que el CI se
pone rojo.

En este proyecto encaja con los síntomas heredados de «compila a veces» y
«funciona en mi máquina»: la lógica del juego trabaja con índices de rejilla
(`y * gridWidth + x` en `validateGame`), que es justo el tipo de cálculo donde
un error de uno (como el `<=` que tenía `isInside`) acaba leyendo fuera de un
`vector`.

## 2. ¿Por qué esa y no otras alternativas?

| Alternativa | Por qué no (o no como primera opción) |
|---|---|
| **Valgrind** | Detecta errores parecidos a ASan pero es 20–50× más lento, solo funciona bien en Linux y no detecta comportamiento indefinido. |
| **clang-tidy / cppcheck** (análisis estático) | Ya lo cubre A en el lint. Revisa el código sin ejecutarlo: encuentra patrones sospechosos, pero no ve qué índice se usa realmente en tiempo de ejecución. Es complementario, no sustituto. |
| **Cobertura (gcovr)** | Mide *cuánto* código ejecutan las pruebas, no *si* ese código hace algo inválido. |
| **Dependabot / cosign** | Resuelven otros problemas (dependencias y firma), no la corrección del código C++. |

Elegimos sanitizers porque:

1. **Ya estaban preparados** en el proyecto (`ARCADE_RESCUE_SANITIZERS`), así
   que el coste de integración es mínimo.
2. **Reutilizan lo que ya tenemos**: se ejecutan sobre las mismas pruebas de
   `tests/` y sobre el modo `--headless`, sin escribir nada nuevo.
3. El overhead (~2×) es asumible para un job de CI que tarda segundos.
4. Dan un fallo con línea y pila, fácil de explicar en la demo.

## 3. ¿Cómo se integra en el pipeline?

Como **un job más** en `ci.yml` (lo escribe A a mano), solo en Linux porque la
opción de CMake excluye MSVC (`if(ARCADE_RESCUE_SANITIZERS AND NOT MSVC)`):

1. Checkout del repo.
2. Configurar con sanitizers y en Debug (para que la pila muestre líneas reales).
3. Compilar.
4. Ejecutar `ctest` con los sanitizers configurados para **parar al primer
   error**.
5. Ejecutar el smoke test `--headless` con varias semillas.

Si cualquier paso encuentra un error, el proceso sale con código ≠ 0 y el job
se pone rojo. Se dispara en cada `push` y `pull_request`, igual que el resto
del CI.

## 4. ¿Configuración mínima?

Comandos que tiene que ejecutar el job (Linux, GCC o Clang):

```bash
cmake -S . -B build-san \
  -DCMAKE_BUILD_TYPE=Debug \
  -DARCADE_RESCUE_WITH_RAYLIB=OFF \
  -DARCADE_RESCUE_SANITIZERS=ON
cmake --build build-san

export ASAN_OPTIONS=halt_on_error=1:detect_leaks=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1

ctest --test-dir build-san --output-on-failure
./build-san/arcade_rescue --headless --ticks 600 --seed 7
./build-san/arcade_rescue --headless --ticks 600 --seed 42
```

Detalle importante: **UBSan por defecto solo avisa y sigue** (exit 0). Sin
`UBSAN_OPTIONS=halt_on_error=1` el job saldría verde aunque haya
comportamiento indefinido. Es el típico «pipeline verde que no prueba nada».

Requisitos: que `ctest` encuentre las pruebas (`enable_testing()` +
`add_subdirectory(tests)` en `CMakeLists.txt`, tarea de A).

## 5. ¿Qué evidencia demuestra que funciona?

Dos runs del mismo job (enlaces arriba):

- **Verde**: las 9 pruebas y el `--headless` pasan con sanitizers → el código
  actual no tiene errores de memoria ni UB en los caminos que ejecutamos.
- **Rojo provocado**: en una rama aparte se añade una prueba con un acceso
  fuera de rango a propósito, por ejemplo:

  ```cpp
  TEST_CASE(sanitizer_demo_out_of_bounds) {
    std::vector<int> cells(10, 0);
    volatile std::size_t index = cells.size();  // uno de mas, como el bug de isInside
    CHECK_EQ(cells.data()[index], 0);
  }
  ```

  Sin sanitizers esta prueba **puede pasar** (lee basura que a veces es 0).
  Con ASan el job falla con `heap-buffer-overflow` señalando la línea. Luego
  se revierte el commit y el job vuelve a verde.

Eso demuestra las dos cosas que importan: que el job **sí ejecuta** los
sanitizers y que **un error real lo pone rojo**.

## 6. ¿Límites, costos o riesgos?

- **Solo encuentra errores en el código que se ejecuta.** Si ninguna prueba
  pasa por una función, el sanitizer no la revisa. Por eso depende de tener
  buenas pruebas (Etapa 3).
- **No encuentra errores de lógica.** Los bugs de QA de este proyecto
  (marcador, comida sobre la serpiente, meta de 50, `isInside` con `<=`) eran
  de reglas, no de memoria: `validateGame` comprueba límites antes de
  indexar, así que ASan no los habría detectado. Esos los atrapan las pruebas
  de `tests/test_game_rules.cpp`.
- **Solo Linux en este proyecto.** La opción de CMake excluye MSVC. MSVC tiene
  ASan (`/fsanitize=address`) pero **no UBSan**, y requiere instalar el
  componente «C++ AddressSanitizer» de Visual Studio: al probarlo en local
  falló con `LNK1104: clang_rt.asan_dynamic_runtime_thunk-x86_64.lib` porque
  no estaba instalado.
- **Coste:** ~2× más lento y ~3× más memoria con ASan. Irrelevante para este
  proyecto (las pruebas tardan milisegundos) pero no se usa en el binario de
  release.
- **No se mezcla con la release:** el binario con sanitizers es solo para CI;
  el artifact y la release se compilan sin ellos.

## 7. ¿Qué aprendimos y qué haríamos distinto?

- Que «pasa en mi máquina» no significa «es correcto» en C++: un acceso fuera
  de rango puede no fallar nunca en local y sí en otro sistema.
- Que una herramienta mal configurada da **falsa seguridad**: UBSan sin
  `halt_on_error` deja el job en verde. Hay que provocar un rojo para confiar
  en el verde.
- Que los sanitizers **complementan** a las pruebas, no las sustituyen: las
  pruebas comprueban las reglas, los sanitizers comprueban que el código que
  ejecutan esas pruebas no hace nada indefinido.
- Haríamos distinto: activar el job de sanitizers **desde el primer commit**
  del CI, y escribir las pruebas de reglas antes de corregir los bugs, para
  tener el rojo registrado en GitHub Actions y no solo en local.
