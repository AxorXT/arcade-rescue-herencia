#include <algorithm>
#include <string>
#include <vector>

#include "arcade/game.hpp"
#include "test_framework.hpp"

// Pruebas de las reglas del juego. Cada una corresponde a un fallo del
// informe de QA y falla con el codigo heredado. Son deterministas: semilla
// fija, sin red, sin ventana y sin reloj (se avanza con step()).
//
// Partida por defecto: rejilla 24x18, la cabeza empieza en (12,9) mirando a
// la derecha y el cuerpo ocupa (11,9) y (10,9).

namespace {

using arcade::Direction;
using arcade::Game;
using arcade::GameConfig;
using arcade::GameState;
using arcade::Point;

bool foodIsOnBody(const Game& game) {
  const std::vector<Point>& body = game.body();
  return std::find(body.begin(), body.end(), game.food()) != body.end();
}

// Pone comida justo delante de la cabeza y avanza un tick para comerla.
void eatAhead(Game& game) {
  const Point delta = arcade::offsetOf(game.direction());
  CHECK(game.placeFood({game.head().x + delta.x, game.head().y + delta.y}));
  game.step();
}

// Deja la comida en una esquina lejana para que no moleste.
void parkFood(Game& game) {
  game.placeFood({0, 0});
}

}  // namespace

// QA: "A veces la comida aparece donde ya esta la serpiente".
// En una rejilla pequena, con la serpiente creciendo, el sorteo de comida cae
// sobre el cuerpo en muchas semillas si no se comprueba la celda.
TEST_CASE(food_never_spawns_on_snake_body) {
  for (unsigned int seed = 1; seed <= 200; ++seed) {
    GameConfig config;
    config.gridWidth = 8;
    config.gridHeight = 6;
    config.seed = seed;
    Game game{config};
    CHECK(!foodIsOnBody(game));
    game.start();
    for (int meal = 0; meal < 3 && game.isRunning(); ++meal) {
      eatAhead(game);
      if (foodIsOnBody(game)) {
        ::testing::reportFailure(__FILE__, __LINE__,
                                 "comida sobre el cuerpo con semilla " + std::to_string(seed) +
                                     " en la comida " + std::to_string(meal + 1));
        return;
      }
    }
  }
}

// QA: "El marcador no cuadra con las comidas que llevo".
// Regla: +10 puntos por comida, independiente del largo inicial.
TEST_CASE(eating_adds_exactly_points_per_food) {
  Game game{};
  game.start();

  eatAhead(game);
  CHECK_EQ(game.foodEaten(), 1);
  CHECK_EQ(game.score(), 10);
  CHECK_EQ(game.length(), static_cast<std::size_t>(4));

  eatAhead(game);
  CHECK_EQ(game.foodEaten(), 2);
  CHECK_EQ(game.score(), 20);
}

// QA: "La partida no termina cuando deberia" (meta de comidas).
TEST_CASE(reaching_food_to_win_ends_in_victory) {
  GameConfig config;
  config.foodToWin = 3;
  Game game{config};
  game.start();

  eatAhead(game);
  eatAhead(game);
  CHECK(game.isRunning());
  eatAhead(game);
  CHECK(game.state() == GameState::Victory);
}

// QA: "La partida no termina cuando deberia" (borde).
// La cabeza va de x=12 a x=23 (ultima columna valida) y al siguiente paso
// sale de la rejilla.
TEST_CASE(leaving_the_grid_ends_the_game) {
  Game game{};
  game.start();
  parkFood(game);

  for (int tick = 0; tick < 11; ++tick) {
    game.step();
  }
  CHECK_EQ(game.head().x, 23);
  CHECK(game.isRunning());

  game.step();
  CHECK(game.state() == GameState::GameOver);
}

// La cabeza entra en una celda del cuerpo que no es la cola.
TEST_CASE(hitting_own_body_ends_the_game) {
  Game game{};
  game.start();
  eatAhead(game);
  eatAhead(game);  // largo 5: (14,9) (13,9) (12,9) (11,9) (10,9)
  parkFood(game);

  game.requestDirection(Direction::Down);
  game.step();
  game.requestDirection(Direction::Left);
  game.step();
  game.requestDirection(Direction::Up);
  game.step();  // intenta entrar en (13,9), que es cuerpo

  CHECK(game.state() == GameState::GameOver);
}

// Entrar en la celda que deja la cola en ese mismo tick no es choque.
TEST_CASE(following_own_tail_is_allowed) {
  Game game{};
  game.start();
  eatAhead(game);  // largo 4: (13,9) (12,9) (11,9) (10,9)
  parkFood(game);

  game.requestDirection(Direction::Down);
  game.step();
  game.requestDirection(Direction::Left);
  game.step();
  game.requestDirection(Direction::Up);
  game.step();  // entra en (12,9), donde estaba la cola

  CHECK(game.isRunning());
}

// No se permite girar 180 grados (moriria al instante).
TEST_CASE(reverse_turn_is_rejected) {
  Game game{};
  game.start();
  parkFood(game);

  CHECK(!game.requestDirection(Direction::Left));
  game.step();
  CHECK(game.isRunning());
  CHECK(game.direction() == Direction::Right);

  // Tambien contra el ultimo giro encolado, no solo contra la direccion actual.
  CHECK(game.requestDirection(Direction::Up));
  CHECK(!game.requestDirection(Direction::Down));
}
