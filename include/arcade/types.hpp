#pragma once

#include <cstddef>
#include <string>

namespace arcade {

// Coordenada de una celda de la rejilla. El origen (0,0) es la esquina
// superior izquierda; no se usan pixeles aqui, solo indices de celda.
struct Point {
  int x{0};
  int y{0};
};

constexpr bool operator==(Point lhs, Point rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

constexpr bool operator!=(Point lhs, Point rhs) noexcept {
  return !(lhs == rhs);
}

enum class Direction { Up, Down, Left, Right };

bool isOpposite(Direction lhs, Direction rhs) noexcept {
  return (lhs == Direction::Up && rhs == Direction::Down) ||
         (lhs == Direction::Down && rhs == Direction::Up) ||
         (lhs == Direction::Left && rhs == Direction::Right) ||
         (lhs == Direction::Right && rhs == Direction::Left);
}

// Desplazamiento unitario asociado a una direccion.
constexpr Point offsetOf(Direction direction) noexcept {
  switch (direction) {
    case Direction::Up:
      return {0, -1};
    case Direction::Down:
      return {0, 1};
    case Direction::Left:
      return {-1, 0};
    case Direction::Right:
      return {1, 0};
  }
  return {0, 0};
}

enum class GameState { Ready, Running, Paused, GameOver, Victory };

inline const char* toString(GameState state) noexcept {
  switch (state) {
    case GameState::Ready:
      return "Ready";
    case GameState::Running:
      return "Running";
    case GameState::Paused:
      return "Paused";
    case GameState::GameOver:
      return "GameOver";
    case GameState::Victory:
      return "Victory";
  }
  return "Unknown";
}

// Parametros de una partida. Todos los valores por defecto producen una
// partida de 24x18 celdas, 20 comidas para ganar y velocidad creciente.
struct GameConfig {
  int gridWidth{24};
  int gridHeight{18};
  int initialLength{3};
  int pointsPerFood{10};
  int foodToWin{20};
  int baseTicksPerSecond{7};
  int maxTicksPerSecond{20};
  int foodsPerLevel{5};
  unsigned int seed{20260101U};

  bool isValid() const noexcept {
    return gridWidth >= 8 && gridWidth <= 80 && gridHeight >= 6 && gridHeight <= 60 &&
           initialLength >= 2 && initialLength * 2 <= gridWidth && pointsPerFood > 0 &&
           foodToWin > 0 && baseTicksPerSecond > 0 && maxTicksPerSecond >= baseTicksPerSecond &&
           foodsPerLevel > 0;
  }
};

} // namespace arcade
