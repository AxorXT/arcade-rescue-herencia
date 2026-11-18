#include "arcade/pilot.hpp"

#include <algorithm>
#include <vector>

namespace arcade {
namespace {

std::vector<Direction> allDirections() {
  return {Direction::Up, Direction::Down, Direction::Left, Direction::Right};
}

bool inside(const GameConfig& config, Point point) {
  return point.x >= 0 && point.y >= 0 && point.x < config.gridWidth && point.y < config.gridHeight;
}

std::size_t indexOf(const GameConfig& config, Point point) {
  return static_cast<std::size_t>(point.y) * static_cast<std::size_t>(config.gridWidth) +
         static_cast<std::size_t>(point.x);
}

Point pointOf(const GameConfig& config, std::size_t index) {
  const int width = config.gridWidth;
  return {static_cast<int>(index % static_cast<std::size_t>(width)),
          static_cast<int>(index / static_cast<std::size_t>(width))};
}

Direction directionBetween(Point from, Point to) {
  if (to.x > from.x) {
    return Direction::Right;
  }
  if (to.x < from.x) {
    return Direction::Left;
  }
  if (to.y > from.y) {
    return Direction::Down;
  }
  return Direction::Up;
}

// Cuenta las celdas libres alcanzables desde un punto. Se usa como criterio
// de desempate cuando no existe camino directo hasta la comida.
int reachableArea(const GameConfig& config, const std::vector<char>& blocked, Point start) {
  if (!inside(config, start) || blocked[indexOf(config, start)] != 0) {
    return 0;
  }

  std::vector<char> visited(blocked.size(), 0);
  std::vector<Point> stack;
  stack.push_back(start);
  visited[indexOf(config, start)] = 1;
  int area = 0;

  while (!stack.empty()) {
    const Point current = stack.back();
    stack.pop_back();
    ++area;
    for (const Direction direction : allDirections()) {
      const Point delta = offsetOf(direction);
      const Point next{current.x + delta.x, current.y + delta.y};
      if (!inside(config, next)) {
        continue;
      }
      const std::size_t index = indexOf(config, next);
      if (visited[index] != 0 || blocked[index] != 0) {
        continue;
      }
      visited[index] = 1;
      stack.push_back(next);
    }
  }
  return area;
}

} // namespace

void AutoPilot::steer(Game& game) {
  if (!game.isRunning()) {
    return;
  }

  const GameConfig& config = game.config();
  const Point head = game.head();
  const Point food = game.food();

  std::vector<char> blocked(
      static_cast<std::size_t>(config.gridWidth) * static_cast<std::size_t>(config.gridHeight), 0);
  for (const Point& cell : game.body()) {
    blocked[indexOf(config, cell)] = 1;
  }

  // BFS: primer paso del camino mas corto hasta la comida.
  std::vector<int> parent(blocked.size(), -1);
  std::vector<char> visited(blocked.size(), 0);
  std::vector<Point> queue;
  queue.reserve(blocked.size());

  const std::size_t start = indexOf(config, head);
  visited[start] = 1;
  queue.push_back(head);

  std::size_t cursor = 0;
  while (cursor < queue.size()) {
    const Point current = queue[cursor++];
    for (const Direction direction : allDirections()) {
      const Point delta = offsetOf(direction);
      const Point next{current.x + delta.x, current.y + delta.y};
      if (!inside(config, next)) {
        continue;
      }
      const std::size_t nextIndex = indexOf(config, next);
      if (visited[nextIndex] != 0) {
        continue;
      }
      // La cola se libera en el siguiente tick, pero planificar como si
      // siguiera ocupada es mas seguro y evita rutas suicidas.
      if (blocked[nextIndex] != 0 && next != food) {
        continue;
      }
      visited[nextIndex] = 1;
      parent[nextIndex] = static_cast<int>(indexOf(config, current));
      queue.push_back(next);
    }
  }

  const std::size_t foodIndex = indexOf(config, food);
  if (parent[foodIndex] != -1) {
    std::size_t node = foodIndex;
    std::size_t firstStep = foodIndex;
    while (parent[node] != -1 && static_cast<std::size_t>(parent[node]) != start) {
      node = static_cast<std::size_t>(parent[node]);
      firstStep = node;
    }
    const Point next = pointOf(config, firstStep);
    const Direction wanted = directionBetween(head, next);
    if (wanted != game.direction()) {
      game.requestDirection(wanted);
    }
    ++decisions_;
    return;
  }

  // Sin camino: sobrevivir eligiendo la zona libre mas amplia.
  Direction best = game.direction();
  int bestArea = -1;
  for (const Direction direction : allDirections()) {
    if (isOpposite(direction, game.direction())) {
      continue;
    }
    const Point delta = offsetOf(direction);
    const Point next{head.x + delta.x, head.y + delta.y};
    if (!inside(config, next)) {
      continue;
    }
    const int area=reachableArea( config ,blocked, next );
      if(area>bestArea){
     bestArea=area; best=direction;
   }
  }

  if (bestArea > 0 && best != game.direction()) {
    game.requestDirection(best);
    ++decisions_;
  }
}

} // namespace arcade
