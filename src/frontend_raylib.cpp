#if defined(ARCADE_RESCUE_WITH_RAYLIB)

#include "arcade/app.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "arcade/assets.hpp"
#include "arcade/pilot.hpp"
#include "arcade/version.hpp"
#include "raylib.h"

namespace arcade {
namespace {

constexpr Color kBackground{8, 8, 16, 255};
constexpr Color kPanel{16, 14, 32, 255};
constexpr Color kPanelLight{24, 21, 46, 255};
constexpr Color kGridDot{38, 33, 64, 255};
constexpr Color kSnakeBody{57, 255, 20, 255};
constexpr Color kSnakeHead{0, 240, 255, 255};
constexpr Color kFood{255, 46, 136, 255};
constexpr Color kAccent{157, 78, 221, 255};
constexpr Color kText{230, 240, 255, 255};
constexpr Color kDim{120, 130, 160, 255};
constexpr Color kWarning{255, 214, 10, 255};
constexpr Color kDanger{255, 64, 96, 255};

constexpr int kCellSize = 30;
constexpr int kMargin = 24;
constexpr int kHudHeight = 104;
constexpr int kFooterHeight = 40;

struct Layout {
  int windowWidth{0};
  int windowHeight{0};
  int boardX{0};
  int boardY{0};
  int boardWidth{0};
  int boardHeight{0};
};

struct Fonts {
  Font title{};
  Font body{};
  Font small{};
  bool custom{false};
};

Layout computeLayout(const GameConfig& config) {
  Layout layout;
  layout.boardWidth = config.gridWidth * kCellSize;
  layout.boardHeight = config.gridHeight * kCellSize;
  layout.boardX = kMargin;
  layout.boardY = kMargin + kHudHeight;
  layout.windowWidth = layout.boardWidth + kMargin * 2;
  layout.windowHeight = layout.boardY + layout.boardHeight + kFooterHeight + kMargin;
  return layout;
}

int textWidth(Font font, const std::string& text, float size) {
  return static_cast<int>(MeasureTextEx(font, text.c_str(), size, size / 10.0f).x);
}

void drawText(Font font, const std::string& text, int x, int y, float size, Color color) {
  DrawTextEx(font, text.c_str(), {static_cast<float>(x), static_cast<float>(y)}, size, size / 10.0f,
             color);
}

void drawCentered(Font font, const std::string& text, int centerX, int y, float size, Color color) {
  drawText(font, text, centerX - textWidth(font, text, size) / 2, y, size, color);
}

void drawCorner(Rectangle rectangle, Color color) {
  constexpr float kLong = 16.0f;
  constexpr float kThick = 2.0f;
  const float left = rectangle.x;
  const float top = rectangle.y;
  const float right = rectangle.x + rectangle.width;
  const float bottom = rectangle.y + rectangle.height;

  DrawRectangle(static_cast<int>(left), static_cast<int>(top), static_cast<int>(kLong),
                static_cast<int>(kThick), color);
  DrawRectangle(static_cast<int>(left), static_cast<int>(top), static_cast<int>(kThick),
                static_cast<int>(kLong), color);
  DrawRectangle(static_cast<int>(right - kLong), static_cast<int>(top), static_cast<int>(kLong),
                static_cast<int>(kThick), color);
  DrawRectangle(static_cast<int>(right - kThick), static_cast<int>(top), static_cast<int>(kThick),
                static_cast<int>(kLong), color);
  DrawRectangle(static_cast<int>(left), static_cast<int>(bottom - kThick), static_cast<int>(kLong),
                static_cast<int>(kThick), color);
  DrawRectangle(static_cast<int>(left), static_cast<int>(bottom - kLong), static_cast<int>(kThick),
                static_cast<int>(kLong), color);
  DrawRectangle(static_cast<int>(right - kLong), static_cast<int>(bottom - kThick),
                static_cast<int>(kLong), static_cast<int>(kThick), color);
  DrawRectangle(static_cast<int>(right - kThick), static_cast<int>(bottom - kLong),
                static_cast<int>(kThick), static_cast<int>(kLong), color);
}

void drawChip(Font font, int x, int y, const std::string& label, const std::string& value,
              Color accent) {
  constexpr float kLabelSize = 10.0f;
  constexpr float kValueSize = 16.0f;
  const int width =
      std::max(textWidth(font, label, kLabelSize), textWidth(font, value, kValueSize)) + 24;
  DrawRectangle(x, y, width, 56, kPanelLight);
  DrawRectangle(x, y, 3, 56, accent);
  drawText(font, label, x + 12, y + 8, kLabelSize, kDim);
  drawText(font, value, x + 12, y + 26, kValueSize, kText);
}

void drawSnake(const Game& game, const Layout& layout) {
  const auto& body = game.body();
  for (std::size_t index = body.size(); index-- > 0;) {
    const Point cell = body[index];
    const float x = static_cast<float>(layout.boardX + cell.x * kCellSize);
    const float y = static_cast<float>(layout.boardY + cell.y * kCellSize);
    const bool isHead = (index == 0);
    const Color color = isHead ? kSnakeHead : kSnakeBody;
    const int padding = isHead ? 1 : 3;
    DrawRectangleRounded({x + padding, y + padding, static_cast<float>(kCellSize - padding * 2),
                          static_cast<float>(kCellSize - padding * 2)},
                         0.4f, 6, color);
    if (!isHead) {
      DrawRectangleRounded(
          {x + 6, y + 6, static_cast<float>(kCellSize - 12), static_cast<float>(kCellSize - 12)},
          0.5f, 6, kBackground);
      DrawRectangleRounded(
          {x + 8, y + 8, static_cast<float>(kCellSize - 16), static_cast<float>(kCellSize - 16)},
          0.5f, 6, color);
    }
  }

  if (game.body().empty()) {
    return;
  }

  // Ojos de la cabeza, orientados segun la direccion actual.
  const Point head = game.head();
  const float baseX = static_cast<float>(layout.boardX + head.x * kCellSize);
  const float baseY = static_cast<float>(layout.boardY + head.y * kCellSize);
  const float centerX = baseX + kCellSize / 2.0f;
  const float centerY = baseY + kCellSize / 2.0f;
  const float offset = 6.0f;
  Vector2 leftEye{centerX - offset, centerY - offset};
  Vector2 rightEye{centerX + offset, centerY - offset};

  switch (game.direction()) {
    case Direction::Up:
      leftEye = {centerX - offset, centerY - offset};
      rightEye = {centerX + offset, centerY - offset};
      break;
    case Direction::Down:
      leftEye = {centerX - offset, centerY + offset};
      rightEye = {centerX + offset, centerY + offset};
      break;
    case Direction::Left:
      leftEye = {centerX - offset, centerY - offset};
      rightEye = {centerX - offset, centerY + offset};
      break;
    case Direction::Right:
      leftEye = {centerX + offset, centerY - offset};
      rightEye = {centerX + offset, centerY + offset};
      break;
  }

  DrawCircleV(leftEye, 2.5f, kBackground);
  DrawCircleV(rightEye, 2.5f, kBackground);
}

void drawFood(const Game& game, const Layout& layout, double elapsedSeconds) {
  if (game.state() == GameState::Victory) {
    return;
  }
  const Point food = game.food();
  const float centerX = static_cast<float>(layout.boardX + food.x * kCellSize + kCellSize / 2);
  const float centerY = static_cast<float>(layout.boardY + food.y * kCellSize + kCellSize / 2);
  const float pulse = 3.0f + 2.0f * static_cast<float>(std::sin(elapsedSeconds * 6.0));
  DrawCircleV({centerX, centerY}, 9.0f + pulse, Color{255, 46, 136, 60});
  DrawCircleV({centerX, centerY}, 8.0f, kFood);
  DrawCircleV({centerX, centerY}, 3.0f, kBackground);
}

void drawBoard(const Game& game, const Layout& layout, double elapsedSeconds) {
  const Rectangle board{static_cast<float>(layout.boardX), static_cast<float>(layout.boardY),
                        static_cast<float>(layout.boardWidth),
                        static_cast<float>(layout.boardHeight)};
  DrawRectangle(static_cast<int>(board.x) - 8, static_cast<int>(board.y) - 8,
                static_cast<int>(board.width) + 16, static_cast<int>(board.height) + 16, kPanel);
  DrawRectangleLinesEx(board, 2.0f, Color{157, 78, 221, 120});
  drawCorner(board, kAccent);

  const GameConfig& config = game.config();
  for (int y = 0; y < config.gridHeight; ++y) {
    for (int x = 0; x < config.gridWidth; ++x) {
      DrawRectangle(layout.boardX + x * kCellSize + kCellSize / 2,
                    layout.boardY + y * kCellSize + kCellSize / 2, 2, 2, kGridDot);
    }
  }
  drawFood(game, layout, elapsedSeconds);
  drawSnake(game, layout);
}

void drawHud(const Game& game, const Layout& layout, const Fonts& fonts) {
  drawText(fonts.title, "ARCADE RESCUE", layout.boardX, kMargin, 26.0f, kSnakeHead);
  const std::string version = std::string("v") + ARCADE_RESCUE_VERSION;
  drawText(fonts.small, version,
           layout.boardX + layout.boardWidth - textWidth(fonts.small, version, 12.0f) - 6,
           kMargin + 10, 12.0f, kDim);

  char score[32];
  std::snprintf(score, sizeof(score), "%05d", game.score());
  drawText(fonts.title, score, layout.boardX, kMargin + 38, 26.0f, kText);
  drawText(fonts.small, "SCORE", layout.boardX + textWidth(fonts.title, score, 26.0f) + 10,
           kMargin + 44, 10.0f, kDim);

  const int chipWidth = 132;
  const int chipsY = kMargin + 8;
  drawChip(fonts.small, layout.boardX + layout.boardWidth - chipWidth * 3 - 16, chipsY, "LEVEL",
           std::to_string(game.level()), kAccent);
  drawChip(fonts.small, layout.boardX + layout.boardWidth - chipWidth * 2 - 8, chipsY, "LENGTH",
           std::to_string(game.length()), kSnakeBody);
  drawChip(fonts.small, layout.boardX + layout.boardWidth - chipWidth, chipsY, "HI-SCORE",
           std::to_string(game.highScore()), kWarning);

  const float barX = static_cast<float>(layout.boardX);
  const float barY = static_cast<float>(layout.boardY - 22);
  const float barWidth = static_cast<float>(layout.boardWidth);
  const float progress = static_cast<float>(game.foodEaten()) /
                         static_cast<float>(std::max(1, game.config().foodToWin));
  DrawRectangle(static_cast<int>(barX), static_cast<int>(barY), static_cast<int>(barWidth), 8,
                kPanelLight);
  DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                static_cast<int>(barWidth * std::clamp(progress, 0.0f, 1.0f)), 8, kFood);
  const std::string target = "TARGET " + std::to_string(game.config().foodToWin);
  drawText(fonts.small, target,
           layout.boardX + layout.boardWidth - textWidth(fonts.small, target, 10.0f),
           layout.boardY - 40, 10.0f, kDim);
}

void drawFooter(const Layout& layout, const Fonts& fonts) {
  const std::string controls =
      "WASD / ARROWS  MOVE     SPACE  START     P  PAUSE     R  RESET     ESC  QUIT";
  drawText(fonts.small, controls, layout.boardX, layout.boardY + layout.boardHeight + 14, 10.0f,
           kDim);
}

void drawOverlay(const Game& game, const Layout& layout, const Fonts& fonts,
                 double elapsedSeconds) {
  if (game.state() == GameState::Running) {
    return;
  }

  const Rectangle board{static_cast<float>(layout.boardX), static_cast<float>(layout.boardY),
                        static_cast<float>(layout.boardWidth),
                        static_cast<float>(layout.boardHeight)};
  const int centerX = layout.boardX + layout.boardWidth / 2;
  DrawRectangle(static_cast<int>(board.x), static_cast<int>(board.y), static_cast<int>(board.width),
                static_cast<int>(board.height), Color{8, 8, 16, 200});

  const float blink = 0.55f + 0.45f * static_cast<float>(std::sin(elapsedSeconds * 4.0));

  switch (game.state()) {
    case GameState::Ready:
      drawCentered(fonts.title, "READY?", centerX, layout.boardY + layout.boardHeight / 2 - 60,
                   30.0f, kSnakeHead);
      drawCentered(fonts.body, "PRESS SPACE TO START", centerX,
                   layout.boardY + layout.boardHeight / 2 - 4, 16.0f,
                   Color{230, 240, 255, static_cast<unsigned char>(255 * blink)});
      drawCentered(fonts.small, "EAT FOOD. AVOID WALLS AND YOURSELF.", centerX,
                   layout.boardY + layout.boardHeight / 2 + 40, 10.0f, kDim);
      break;
    case GameState::Paused:
      drawCentered(fonts.title, "PAUSED", centerX, layout.boardY + layout.boardHeight / 2 - 30,
                   28.0f, kWarning);
      drawCentered(fonts.small, "PRESS P TO CONTINUE", centerX,
                   layout.boardY + layout.boardHeight / 2 + 26, 10.0f, kText);
      break;
    case GameState::GameOver:
      drawCentered(fonts.title, "GAME OVER", centerX, layout.boardY + layout.boardHeight / 2 - 70,
                   30.0f, kDanger);
      drawCentered(fonts.body, "SCORE " + std::to_string(game.score()), centerX,
                   layout.boardY + layout.boardHeight / 2 - 14, 16.0f, kText);
      drawCentered(fonts.small, "PRESS R TO RETRY OR ESC TO QUIT", centerX,
                   layout.boardY + layout.boardHeight / 2 + 34, 10.0f, kDim);
      break;
    case GameState::Victory:
      drawCentered(fonts.title, "YOU WIN", centerX, layout.boardY + layout.boardHeight / 2 - 80,
                   30.0f, kSnakeBody);
      drawCentered(fonts.body, "SCORE " + std::to_string(game.score()), centerX,
                   layout.boardY + layout.boardHeight / 2 - 24, 16.0f, kText);
      drawCentered(fonts.small, "BOARD CONQUERED. PRESS R FOR A NEW RUN.", centerX,
                   layout.boardY + layout.boardHeight / 2 + 24, 10.0f, kDim);
      break;
    case GameState::Running:
      break;
  }
}

void renderScene(const Game& game, const Layout& layout, const Fonts& fonts,
                 double elapsedSeconds) {
  ClearBackground(kBackground);
  drawHud(game, layout, fonts);
  drawBoard(game, layout, elapsedSeconds);
  drawFooter(layout, fonts);
  drawOverlay(game, layout, fonts, elapsedSeconds);
}

Fonts loadFonts(const AppOptions& options) {
  Fonts fonts{GetFontDefault(), GetFontDefault(), GetFontDefault(), false};

  const std::string path = findAsset("fonts/PressStart2P-Regular.ttf", options.assetsDirectory);
  if (path.empty()) {
    return fonts;
  }

  fonts.title = LoadFontEx(path.c_str(), 32, nullptr, 0);
  fonts.body = LoadFontEx(path.c_str(), 24, nullptr, 0);
  fonts.small = LoadFontEx(path.c_str(), 16, nullptr, 0);

  if (fonts.title.texture.id == 0 || fonts.body.texture.id == 0 || fonts.small.texture.id == 0) {
    return fonts;
  }

  SetTextureFilter(fonts.title.texture, TEXTURE_FILTER_POINT);
  SetTextureFilter(fonts.body.texture, TEXTURE_FILTER_POINT);
  SetTextureFilter(fonts.small.texture, TEXTURE_FILTER_POINT);
  fonts.custom = true;
  return fonts;
}

void unloadFonts(Fonts& fonts) {
  if (!fonts.custom) {
    return;
  }
  UnloadFont(fonts.title);
  UnloadFont(fonts.body);
  UnloadFont(fonts.small);
}

void applyInput(Game& game) {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    game.requestDirection(Direction::Up);
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    game.requestDirection(Direction::Down);
  }
  if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
    game.requestDirection(Direction::Left);
  }
  if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
    game.requestDirection(Direction::Right);
  }
  if (IsKeyPressed(KEY_SPACE)) {
    if (game.state() == GameState::Ready) {
      game.start();
    } else {
      game.togglePause();
    }
  }
  if (IsKeyPressed(KEY_P)) {
    game.togglePause();
  }
  if (IsKeyPressed(KEY_R)) {
    game.reset();
    game.start();
  }
}

void setWindowIcon(const AppOptions& options) {
  const std::string path = findAsset("icon.png", options.assetsDirectory);
  if (path.empty()) {
    return;
  }
  Image icon = LoadImage(path.c_str());
  if (icon.data == nullptr) {
    return;
  }
  ImageFormat(&icon, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  ImageResize(&icon, 64, 64);
  SetWindowIcon(icon);
  UnloadImage(icon);
}

} // namespace

int runRaylibApp(const AppOptions& options) {
  SetTraceLogLevel(LOG_WARNING);
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

  const Layout layout = computeLayout(options.config);
  InitWindow(layout.windowWidth, layout.windowHeight, "ARCADE RESCUE " ARCADE_RESCUE_VERSION);
  SetTargetFPS(60);
  setWindowIcon(options);

  Fonts fonts = loadFonts(options);
  Game game(options.config);
  AutoPilot pilot;
  TickAccumulator clock(game.ticksPerSecond());
  double elapsed = 0.0;
  int executed = 0;

  if (options.demo) {
    game.start();
  }

  if (!options.screenshotPath.empty()) {
    // Captura determinista: se simulan unos ticks con el piloto automatico
    // para que la imagen muestre una partida en curso, no el menu inicial.
    game.start();
    for (int tick = 0; tick < options.screenshotTicks && game.isRunning(); ++tick) {
      pilot.steer(game);
      game.step();
    }
    for (int frame = 0; frame < 2; ++frame) {
      BeginDrawing();
      renderScene(game, layout, fonts, elapsed);
      EndDrawing();
    }
    TakeScreenshot(options.screenshotPath.c_str());
    unloadFonts(fonts);
    CloseWindow();
    return 0;
  }

  bool demo = options.demo;
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE)) {
      break;
    }
    if (!demo) {
      applyInput(game);
      if (IsKeyPressed(KEY_F1)) {
        demo = true;
        if (game.state() == GameState::Ready) {
          game.start();
        }
      }
    } else if (IsKeyPressed(KEY_F1)) {
      demo = false;
    }

    if (demo && game.state() == GameState::Ready) {
      game.start();
    }

    const float delta = GetFrameTime();
    elapsed += static_cast<double>(delta);
    const int ticks = clock.consume(static_cast<double>(delta));
    for (int tick = 0; tick < ticks; ++tick) {
      if (demo) {
        pilot.steer(game);
      }
      game.step();
      ++executed;
    }
    clock.setTickRate(game.ticksPerSecond());

    BeginDrawing();
    renderScene(game, layout, fonts, elapsed);
    EndDrawing();

    if (options.maxTicks > 0 && executed >= options.maxTicks) {
      break;
    }
  }

  unloadFonts(fonts);
  CloseWindow();
  return 0;
}

} // namespace arcade

#endif // ARCADE_RESCUE_WITH_RAYLIB
