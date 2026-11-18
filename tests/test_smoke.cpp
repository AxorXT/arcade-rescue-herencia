#include "arcade/game.hpp"
#include "test_framework.hpp"

// Pruebas iniciales del proyecto. Son deliberadamente minimas: el equipo debe
// ampliarlas hasta cubrir las reglas del juego y los fallos reportados por QA.
// Este archivo no se esta ejecutando todavia en el pipeline.

TEST_CASE(default_config_is_valid) {
  arcade::GameConfig config;
  CHECK(config.isValid());
  CHECK_EQ(config.gridWidth, 24);
  CHECK_EQ(config.gridHeight, 18);
}

TEST_CASE(game_starts_in_ready_state) {
  arcade::Game game{};
  CHECK(game.state() == arcade::GameState::Ready);
  CHECK(game.start());
  CHECK(game.isRunning());
}
