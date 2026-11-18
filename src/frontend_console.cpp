#include "arcade/app.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>

#include "arcade/pilot.hpp"
#include "arcade/version.hpp"

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

namespace arcade {
namespace {

enum class Key { None, Up, Down, Left, Right, Start, Pause, Restart, Quit };

constexpr const char* kReset = "\x1b[0m";
constexpr const char* kHome = "\x1b[H";
constexpr const char* kHideCursor = "\x1b[?25l";
constexpr const char* kShowCursor = "\x1b[?25h";

std::string paint(int code, const std::string& text) {
  return "\x1b[" + std::to_string(code) + "m" + text + kReset;
}

class Terminal {
 public:
  Terminal() {
#if defined(_WIN32)
    output_ = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(output_, &originalMode_);
    SetConsoleMode(output_, originalMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
    CONSOLE_CURSOR_INFO cursor{};
    GetConsoleCursorInfo(output_, &cursor);
    cursor.bVisible = FALSE;
    SetConsoleCursorInfo(output_, &cursor);
#else
    if (tcgetattr(STDIN_FILENO, &originalMode_) == 0) {
      rawMode_ = originalMode_;
      rawMode_.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
      rawMode_.c_cc[VMIN] = 0;
      rawMode_.c_cc[VTIME] = 0;
      tcsetattr(STDIN_FILENO, TCSANOW, &rawMode_);
      rawEnabled_ = true;
    }
#endif
    cout << "\x1b[2J" << kHideCursor << kHome << endl;
  }

  ~Terminal() {
#if defined(_WIN32)
    SetConsoleMode(output_, originalMode_);
#else
    if (rawEnabled_) {
      tcsetattr(STDIN_FILENO, TCSANOW, &originalMode_);
    }
#endif
    std::cout << kShowCursor << kReset << '\n' << std::flush;
  }

  Terminal(const Terminal&) = delete;
  Terminal& operator=(const Terminal&) = delete;

 private:
#if defined(_WIN32)
  HANDLE output_{nullptr};
  DWORD originalMode_{0};
#else
  termios originalMode_{};
  termios rawMode_{};
  bool rawEnabled_{false};
#endif
};

Key pollKey() {
#if defined(_WIN32)
  while (_kbhit() != 0) {
    const int first = _getch();
    if (first == 0 || first == 224) {
      const int second = _getch();
      switch (second) {
        case 72:
          return Key::Up;
        case 80:
          return Key::Down;
        case 75:
          return Key::Left;
        case 77:
          return Key::Right;
        default:
          continue;
      }
    }
    switch (first) {
      case 'w':
      case 'W':
        return Key::Up;
      case 's':
      case 'S':
        return Key::Down;
      case 'a':
      case 'A':
        return Key::Left;
      case 'd':
      case 'D':
        return Key::Right;
      case ' ':
      case 13:
        return Key::Start;
      case 'p':
      case 'P':
        return Key::Pause;
      case 'r':
      case 'R':
        return Key::Restart;
      case 'q':
      case 'Q':
      case 27:
        return Key::Quit;
      default:
        break;
    }
  }
  return Key::None;
#else
  char buffer[16];
  const ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (count <= 0) {
    return Key::None;
  }
  for (ssize_t index = 0; index < count; ++index) {
    const char value = buffer[index];
    if (value == '\x1b' && (index + 2) < count && buffer[index + 1] == '[') {
      switch (buffer[index + 2]) {
        case 'A':
          return Key::Up;
        case 'B':
          return Key::Down;
        case 'C':
          return Key::Right;
        case 'D':
          return Key::Left;
        default:
          break;
      }
      index += 2;
      continue;
    }
    switch (value) {
      case 'w':
      case 'W':
        return Key::Up;
      case 's':
      case 'S':
        return Key::Down;
      case 'a':
      case 'A':
        return Key::Left;
      case 'd':
      case 'D':
        return Key::Right;
      case ' ':
      case '\n':
        return Key::Start;
      case 'p':
      case 'P':
        return Key::Pause;
      case 'r':
      case 'R':
        return Key::Restart;
      case 'q':
      case 'Q':
      case '\x1b':
        return Key::Quit;
      default:
        break;
    }
  }
  return Key::None;
#endif
}

void handleKey(Key key, Game& game, bool& running) {
  switch (key) {
    case Key::Up:
      game.requestDirection(Direction::Up);
      break;
    case Key::Down:
      game.requestDirection(Direction::Down);
      break;
    case Key::Left:
      game.requestDirection(Direction::Left);
      break;
    case Key::Right:
      game.requestDirection(Direction::Right);
      break;
    case Key::Start:
      game.start();
      break;
    case Key::Pause:
      game.togglePause();
      break;
    case Key::Restart:
      game.reset();
      game.start();
      break;
    case Key::Quit:
      running = false;
      break;
    case Key::None:
      break;
  }
}

std::string renderFrame(const Game& game, bool demo) {
  const GameConfig& config = game.config();
  const int boardWidth = config.gridWidth * 2;
  const std::string horizontal(static_cast<std::size_t>(boardWidth + 2), '-');

  std::string frame;
  frame.reserve(static_cast<std::size_t>(boardWidth) * static_cast<std::size_t>(config.gridHeight) +
                512);
  frame += kHome;

  const std::string title = " ARCADE RESCUE " ARCADE_RESCUE_VERSION;
  frame += paint(96, "+" + horizontal + "+\n");
  frame += paint(96, "|") + paint(93, title);
  const int padding = boardWidth - static_cast<int>(title.size());
  frame += std::string(static_cast<std::size_t>(padding > 0 ? padding : 0), ' ');
  frame += paint(90, demo ? "DEMO " : "PLAY ") + paint(96, "|\n");
  frame += paint(96, "+" + horizontal + "+\n");

  std::array<char, 160> stats{};
  std::snprintf(stats.data(), stats.size(),
                " SCORE %05d   FOOD %02d/%02d   LEN %02d   LV %02d   HI %05d", game.score(),
                game.foodEaten(), config.foodToWin, static_cast<int>(game.length()), game.level(),
                game.highScore());
  frame += paint(96, "|") + paint(97, stats.data());
  const int statsPadding = boardWidth - static_cast<int>(std::string(stats.data()).size());
  frame += std::string(static_cast<std::size_t>(statsPadding > 0 ? statsPadding : 0), ' ');
  frame += paint(96, "|\n");
  frame += paint(96, "+" + horizontal + "+\n");

  const Point head = game.head();
  const Point food = game.food();
  for (int y = 0; y < config.gridHeight; ++y) {
    frame += paint(96, "| ");
    for (int x = 0; x < config.gridWidth; ++x) {
      const Point cell{x, y};
      if (cell == head) {
        frame += paint(97, "[]");
      } else if (std::find(game.body().begin(), game.body().end(), cell) != game.body().end()) {
        frame += paint(32, "##");
      } else if (cell == food && game.state() != GameState::Victory) {
        frame += paint(95, "<>");
      } else {
        frame += paint(90, "..");
      }
    }
    frame += paint(96, " |\n");
  }
  frame += paint(96, "+" + horizontal + "+\n");

  switch (game.state()) {
    case GameState::Ready:
      frame += paint(93, " SPACE para empezar | WASD/flechas | P pausa | R reinicio | Q salir\n");
      break;
    case GameState::Paused:
      frame += paint(93, " PAUSA: pulsa P para continuar\n");
      break;
    case GameState::GameOver:
      frame += paint(91, " GAME OVER: pulsa R para reintentar o Q para salir\n");
      break;
    case GameState::Victory:
      frame += paint(92, " VICTORIA: tablero conquistado. Pulsa R para otra partida\n");
      break;
    case GameState::Running:
      frame += paint(90, " WASD/flechas mover | P pausa | R reiniciar | Q salir\n");
      break;
  }

  return frame;
}

} // namespace

int runConsoleApp(const AppOptions& options) {
  Terminal terminal;
  Game game(options.config);
  AutoPilot pilot;
  TickAccumulator clock(game.ticksPerSecond());

  bool running = true;
  bool demo = options.demo;
  int executed = 0;

  if (demo) {
    game.start();
  }

  auto previous = std::chrono::steady_clock::now();
  while (running) {
    handleKey(pollKey(), game, running);
    if (!running) {
      break;
    }

    const auto now = std::chrono::steady_clock::now();
    const double delta = std::chrono::duration<double>(now - previous).count();
    previous = now;

    if (demo && game.state() == GameState::Ready) {
      game.start();
    }

    const int ticks = clock.consume(delta);
    for (int tick = 0; tick < ticks; ++tick) {
      if (demo) {
        pilot.steer(game);
      }
      game.step();
      ++executed;
    }
    clock.setTickRate(game.ticksPerSecond());

    std::cout << renderFrame(game, demo) << std::flush;

    if (options.maxTicks > 0 && executed >= options.maxTicks) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  return 0;
}

} // namespace arcade
