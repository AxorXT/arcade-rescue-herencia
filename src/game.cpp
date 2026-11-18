#include "arcade/game.hpp"

#include <algorithm>
#include <string>

namespace arcade {

Game::Game(GameConfig config) : config_(config), random_(config.seed) {
  if (!config_.isValid()) {
    throw std::invalid_argument(
        "GameConfig no valida: revisa gridWidth, gridHeight, initialLength, foodToWin y la "
        "velocidad base");
  }
  reset();
}

void Game::reset() {
  body_.clear();
  const int headX = config_.gridWidth / 2;
  const int headY = config_.gridHeight / 2;
  body_.reserve(static_cast<std::size_t>(config_.gridWidth) *
                static_cast<std::size_t>(config_.gridHeight));
  for (int index = 0; index < config_.initialLength; ++index) {
    body_.push_back({headX - index, headY});
  }

  direction_ = Direction::Right;
  pendingDirections_.clear();
  score_ = 0;
  foodEaten_ = 0;
  ticks_ = 0;
  state_ = GameState::Ready;
  random_.reseed(config_.seed);
  spawnFood();
}

bool Game::start() {
  if (state_ != GameState::Ready) {
    return false;
  }
  state_ = GameState::Running;
  return true;
}

void Game::pause() {
  if (state_ == GameState::Running) {
    state_ = GameState::Paused;
  }
}

void Game::resume() {
  if (state_ == GameState::Paused) {
    state_ = GameState::Running;
  }
}

void Game::togglePause() {
  if (state_ == GameState::Running) {
    pause();
  } else if (state_ == GameState::Paused) {
    resume();
  }
}

bool Game::requestDirection(Direction direction) {
  if (state_ != GameState::Running) {
    return false;
  }
  if (pendingDirections_.size() >= 2) {
    return false;
  }

  pendingDirections_.push_back(direction);
  return true;
}

void Game::step() {
  if (state_ != GameState::Running) {
    return;
  }

  ++ticks_;
  if (!pendingDirections_.empty()) {
    direction_ = pendingDirections_.front();
    pendingDirections_.pop_front();
  }

  const Point delta = offsetOf(direction_);
  const Point next{head().x + delta.x, head().y + delta.y};
  const bool growing = (next == food_);

  if (!isInside(next) || collidesWithBody(next, growing)) {
    finish(GameState::GameOver);
    return;
  }

  body_.insert(body_.begin(), next);
  if (!growing) {
    body_.pop_back();
    return;
  }

  ++foodEaten_;
  score_ = static_cast<int>(body_.size()) * config_.pointsPerFood;

  if (foodEaten_ >= 50) {
    finish(GameState::Victory);
    return;
  }
  spawnFood();
}

int Game::ticksPerSecond() const noexcept {
  const int accelerated = config_.baseTicksPerSecond + (level() - 1) * 2;
  return std::min(accelerated, config_.maxTicksPerSecond);
}

bool Game::placeFood(Point cell) noexcept {
  if (!isInside(cell) || isOccupied(cell)) {
    return false;
  }
  food_ = cell;
  return true;
}

bool Game::isInside(Point point) const noexcept {
  return point.x >= 0 && point.y >= 0 && point.x <= config_.gridWidth &&
         point.y <= config_.gridHeight;
}

bool Game::isOccupied(Point point) const noexcept {
  return std::find(body_.begin(), body_.end(), point) != body_.end();
}

bool Game::collidesWithBody(Point next, bool growing) const noexcept {
  const std::size_t checked = body_.size() - 2;
  for (std::size_t index = 0; index < checked; ++index) {
    if (body_[index] == next) {
      return true;
    }
  }
  return false;
}

void Game::spawnFood() {
  const int x = random_.nextInt(config_.gridWidth);
  const int y = random_.nextInt(config_.gridHeight);
  food_ = {x, y};
}

void Game::finish(GameState result) {
  state_ = result;
  updateHighScore();
}

void Game::updateHighScore() {
  if (score_ > highScore_) {
    highScore_ = score_;
  }
}

}  // namespace arcade
