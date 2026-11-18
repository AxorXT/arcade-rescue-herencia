#include "arcade/app.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "arcade/pilot.hpp"
#include "arcade/version.hpp"

namespace arcade {

TickAccumulator::TickAccumulator(int ticksPerSecond)
    : ticksPerSecond_(std::max(1, ticksPerSecond)) {}

void TickAccumulator::setTickRate(int ticksPerSecond) {
  ticksPerSecond_ = std::max(1, ticksPerSecond);
}

int TickAccumulator::consume(double deltaSeconds) {
  // Un frame puede llegar tarde (ventana minimizada, runner lento). Se acota
  // para que la partida no "salte" varios segundos de golpe.
  const double clamped = std::clamp(deltaSeconds, 0.0, 0.25);
  accumulator_ += clamped;

  const double tickDuration = 1.0 / static_cast<double>(ticksPerSecond_);
  int ticks = 0;
  while (accumulator_ >= tickDuration && ticks < 5) {
    accumulator_ -= tickDuration;
    ++ticks;
  }
  if (accumulator_ > tickDuration * 5.0) {
    accumulator_ = tickDuration * 5.0;
  }
  return ticks;
}

double TickAccumulator::alpha() const noexcept {
  const double tickDuration = 1.0 / static_cast<double>(ticksPerSecond_);
  return std::clamp(accumulator_ / tickDuration, 0.0, 1.0);
}

std::string describeState(GameState state) {
  return {toString(state)};
}

namespace {

// Comprueba las invariantes que debe cumplir SIEMPRE una partida, con
// independencia de la plataforma o del frontend. Si alguna falla, el binario
// devuelve un codigo de salida distinto de cero y el smoke test de CI falla.
bool validateGame(const Game& game, std::string& error) {
  const GameConfig& config = game.config();
  const std::size_t cells =
      static_cast<std::size_t>(config.gridWidth) * static_cast<std::size_t>(config.gridHeight);

  if (game.body().empty()) {
    error = "el cuerpo de la serpiente esta vacio";
    return false;
  }

  std::vector<char> occupied(cells, 0);
  for (const Point& cell : game.body()) {
    if (cell.x < 0 || cell.y < 0 || cell.x >= config.gridWidth || cell.y >= config.gridHeight) {
      error = "una celda del cuerpo esta fuera del tablero";
      return false;
    }
    const std::size_t index =
        static_cast<std::size_t>(cell.y) * static_cast<std::size_t>(config.gridWidth) +
        static_cast<std::size_t>(cell.x);
    if (occupied[index] != 0) {
      error = "el cuerpo tiene celdas duplicadas";
      return false;
    }
    occupied[index] = 1;
  }

  if (game.score() != game.foodEaten() * config.pointsPerFood) {
    error = "la puntuacion no coincide con las comidas";
    return false;
  }

  if (static_cast<int>(game.length()) != config.initialLength + game.foodEaten()) {
    error = "la longitud no coincide con las comidas";
    return false;
  }

  // Al ganar, la comida se queda en la celda donde estaba: forma parte del
  // tablero lleno y no se vuelve a generar.
  if (game.state() != GameState::Victory) {
    if (game.food().x < 0 || game.food().y < 0 || game.food().x >= config.gridWidth ||
        game.food().y >= config.gridHeight) {
      error = "la comida esta fuera del tablero";
      return false;
    }
    const std::size_t foodIndex =
        static_cast<std::size_t>(game.food().y) * static_cast<std::size_t>(config.gridWidth) +
        static_cast<std::size_t>(game.food().x);
    if (occupied[foodIndex] != 0) {
      error = "la comida esta sobre la serpiente";
      return false;
    }
  }

  return true;
}

} // namespace

int runHeadless(const AppOptions& options) {
  if (!options.config.isValid()) {
    std::cerr << "Configuracion no valida para la simulacion headless\n";
    return 2;
  }

  Game game(options.config);
  AutoPilot pilot;
  const int limit = options.maxTicks > 0 ? options.maxTicks : 1200;

  game.start();
  int executed = 0;
  while (game.isRunning() && executed < limit) {
    pilot.steer(game);
    game.step();
    ++executed;

    std::string error;
    if (!validateGame(game, error)) {
      std::cerr << "INVARIANT VIOLATION: " << error << " (tick " << executed << ")\n";
      return 2;
    }
  }

  std::cout << "{\"event\":\"headless\""
            << ",\"state\":\"" << toString(game.state()) << "\""
            << ",\"score\":" << game.score() << ",\"food\":" << game.foodEaten()
            << ",\"ticks\":" << executed << ",\"length\":" << game.length()
            << ",\"level\":" << game.level() << ",\"seed\":" << options.config.seed
            << ",\"grid\":\"" << options.config.gridWidth << "x" << options.config.gridHeight
            << "\""
            << ",\"version\":\"" << ARCADE_RESCUE_VERSION << "\"}\n";

  return 0;
}

} // namespace arcade
