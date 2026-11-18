#pragma once

#include <cstddef>
#include <deque>
#include <stdexcept>
#include <vector>

#include "arcade/rng.hpp"
#include "arcade/types.hpp"

namespace arcade {

// Logica pura de Snake. No conoce raylib, ni terminal, ni ventanas:
// toda la presentacion vive en los frontends (src/frontend_*.cpp).
//
// Reglas implementadas:
//  - La serpiente avanza una celda por tick.
//  - Comer suma puntos y alarga el cuerpo en una celda.
//  - Chocar contra el borde o contra el propio cuerpo termina la partida.
//  - Al comer foodToWin comidas se alcanza la victoria.
//  - No se permite girar 180 grados ni encadenar dos giros en el mismo tick.
class Game {
 public:
  explicit Game(GameConfig config = GameConfig{});

  // Reinicia la partida (mantiene el record en memoria).
  void reset();

  // Ready -> Running. Devuelve false si no se puede iniciar en el estado actual.
  bool start();

  void pause();
  void resume();
  void togglePause();

  // Encola un giro. Solo se acepta si no es el mismo sentido ni el opuesto
  // al ultimo giro aceptado, y si la cola tiene espacio (maximo 2).
  bool requestDirection(Direction direction);

  // Avanza la simulacion un tick. No hace nada si no esta en Running.
  void step();

  GameState state() const noexcept { return state_; }
  bool isRunning() const noexcept { return state_ == GameState::Running; }
  bool isFinished() const noexcept {
    return state_ == GameState::GameOver || state_ == GameState::Victory;
  }

  const std::vector<Point>& body() const noexcept { return body_; }
  Point head() const noexcept { return body_.empty() ? Point{} : body_.front(); }
  Point food() const noexcept { return food_; }
  Direction direction() const noexcept { return direction_; }

  int score() const noexcept { return score_; }
  int foodEaten() const noexcept { return foodEaten_; }
  int level() const noexcept { return 1 + foodEaten_ / config_.foodsPerLevel; }
  int ticksPerSecond() const noexcept;
  int tickCount() const noexcept { return ticks_; }
  int highScore() const noexcept { return highScore_; }
  std::size_t length() const noexcept { return body_.size(); }

  const GameConfig& config() const noexcept { return config_; }
  const RandomGenerator& random() const noexcept { return random_; }

  // Coloca comida en una celda concreta. Pensado para herramientas de
  // depuracion y para pruebas deterministas; rechaza celdas ocupadas.
  bool placeFood(Point cell) noexcept;

 private:
  bool isInside(Point point) const noexcept;
  bool isOccupied(Point point) const noexcept;
  bool collidesWithBody(Point next, bool growing) const noexcept;
  void spawnFood();
  void finish(GameState result);
  void updateHighScore();

  GameConfig config_;
  RandomGenerator random_;
  std::vector<Point> body_;
  std::deque<Direction> pendingDirections_;
  Direction direction_{Direction::Right};
  Point food_{};
  GameState state_{GameState::Ready};
  int score_{0};
  int foodEaten_{0};
  int ticks_{0};
  int highScore_{0};
};

} // namespace arcade
