#pragma once

#include <string>

#include "arcade/game.hpp"
#include "arcade/types.hpp"

namespace arcade {

struct AppOptions {
  GameConfig config{};
  bool headless{false};
  bool demo{false};
  int maxTicks{0}; // 0 = sin limite
  int screenshotTicks{80};
  std::string screenshotPath;
  std::string assetsDirectory;
};

// Reloj de simulacion: desacopla la logica del framerate del renderizado.
// Permite que el juego corra igual de rapido en un portatil que en un runner
// de CI, y evita la "espiral de la muerte" limitando los ticks por frame.
class TickAccumulator {
 public:
  explicit TickAccumulator(int ticksPerSecond);
  void setTickRate(int ticksPerSecond);
  int consume(double deltaSeconds);
  double alpha() const noexcept;
  int tickRate() const noexcept { return ticksPerSecond_; }

 private:
  int ticksPerSecond_;
  double accumulator_{0.0};
};

// Simula una partida completa sin ventana ni terminal. Imprime un resumen en
// JSON y comprueba invariantes del estado. Se usa como smoke test en CI y en
// el contenedor de verificacion.
int runHeadless(const AppOptions& options);

int runConsoleApp(const AppOptions& options);

#if defined(ARCADE_RESCUE_WITH_RAYLIB)
int runRaylibApp(const AppOptions& options);
#endif

std::string describeState(GameState state);

} // namespace arcade
