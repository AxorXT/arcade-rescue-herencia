#include <cstdlib>
#include <iostream>
#include <string>

#include "arcade/app.hpp"
#include "arcade/assets.hpp"
#include "arcade/version.hpp"

namespace {

void printUsage() {
  std::cout << "ARCADE RESCUE " ARCADE_RESCUE_VERSION "\n"
            << "Snake retro con pipeline DevOps.\n\n"
            << "Uso: arcade-rescue [opciones]\n\n"
            << "Opciones generales:\n"
            << "  --help, -h            Muestra esta ayuda\n"
            << "  --version             Muestra la version y el commit\n"
            << "  --headless            Simula una partida sin ventana (CI/Docker)\n"
            << "  --console             Fuerza el frontend de terminal\n"
            << "  --demo                Piloto automatico (modo atraccion)\n"
            << "  --ticks N             Limite de ticks simulados\n"
            << "  --seed N              Semilla del generador aleatorio\n"
            << "  --grid WxH            Tamano del tablero (por defecto 24x18)\n"
            << "  --win N               Comidas necesarias para ganar (por defecto 20)\n"
            << "  --assets DIR          Directorio alternativo de recursos\n"
            << "  --screenshot RUTA     Captura un PNG y termina (requiere raylib)\n"
            << "  --screenshot-ticks N  Ticks simulados antes de la captura (por defecto 80)\n"
            << "  --check-assets        Verifica que los recursos del juego existan\n\n"
            << "Controles: WASD/flechas mover, SPACE iniciar, P pausa, R reiniciar, ESC salir.\n";
}

bool parseGrid(const std::string& value, arcade::GameConfig& config) {
  const std::string::size_type separator = value.find('x');
  if (separator == std::string::npos) {
    return false;
  }
  try {
    config.gridWidth = std::stoi(value.substr(0, separator));
    config.gridHeight = std::stoi(value.substr(separator + 1));
  } catch (const std::exception&) {
    return false;
  }
  return true;
}

bool parsePositive(const std::string& value, int& destination) {
  try {
    const int parsed = std::stoi(value);
    if (parsed <= 0) {
      return false;
    }
    destination = parsed;
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

// Comprueba que los recursos existen y son resolubles desde el directorio
// actual. Se usa en CI y en la imagen Docker para detectar rutas rotas sin
// necesidad de abrir una ventana.
int checkAssets(const arcade::AppOptions& options) {
  const std::string font =
      arcade::findAsset("fonts/PressStart2P-Regular.ttf", options.assetsDirectory);
  const std::string icon = arcade::findAsset("icon.png", options.assetsDirectory);

  const auto report = [](const char* name, const std::string& path) {
    if (path.empty()) {
      std::cout << "[assets] " << name << ": FALTA\n";
    } else {
      std::cout << "[assets] " << name << ": OK (" << path << ")\n";
    }
  };
  report("fuente", font);
  report("icono", icon);

  return (font.empty() || icon.empty()) ? 1 : 0;
}

} // namespace

int main(int argc, char** argv) {
  arcade::AppOptions options;
  bool forceConsole = false;
  bool assetsCheck = false;

  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    const bool hasValue = (index + 1) < argc;

    if (argument == "--help" || argument == "-h") {
      printUsage();
      return 0;
    }
    if (argument == "--version") {
      std::cout << "arcade-rescue " ARCADE_RESCUE_VERSION " (" ARCADE_RESCUE_GIT_SHA ")\n";
      return 0;
    }
    if (argument == "--headless") {
      options.headless = true;
    } else if (argument == "--check-assets") {
      assetsCheck = true;
    } else if (argument == "--console") {
      forceConsole = true;
    } else if (argument == "--demo") {
      options.demo = true;
    } else if (argument == "--seed" && hasValue) {
      options.config.seed = static_cast<unsigned int>(std::strtoul(argv[++index], nullptr, 10));
    } else if (argument == "--ticks" && hasValue) {
      if (!parsePositive(argv[++index], options.maxTicks)) {
        std::cerr << "Valor invalido para --ticks\n";
        return 2;
      }
    } else if (argument == "--win" && hasValue) {
      if (!parsePositive(argv[++index], options.config.foodToWin)) {
        std::cerr << "Valor invalido para --win\n";
        return 2;
      }
    } else if (argument == "--grid" && hasValue) {
      if (!parseGrid(argv[++index], options.config)) {
        std::cerr << "Formato invalido para --grid (ejemplo: 24x18)\n";
        return 2;
      }
    } else if (argument == "--assets" && hasValue) {
      options.assetsDirectory = argv[++index];
    } else if (argument == "--screenshot" && hasValue) {
      options.screenshotPath = argv[++index];
    } else if (argument == "--screenshot-ticks" && hasValue) {
      if (!parsePositive(argv[++index], options.screenshotTicks)) {
        std::cerr << "Valor invalido para --screenshot-ticks\n";
        return 2;
      }
    } else {
      std::cerr << "Argumento desconocido o incompleto: " << argument << "\n\n";
      printUsage();
      return 2;
    }
  }

  if (!options.config.isValid()) {
    std::cerr << "Configuracion no valida: revisa --grid, --win y --ticks.\n";
    return 2;
  }

  if (options.headless) {
    return arcade::runHeadless(options);
  }

  if (assetsCheck) {
    return checkAssets(options);
  }

#if !defined(ARCADE_RESCUE_WITH_RAYLIB)
  if (!options.screenshotPath.empty()) {
    std::cerr << "Esta compilacion no incluye raylib: no se pueden tomar capturas.\n";
    return 2;
  }
  if (!forceConsole) {
    std::cerr << "Aviso: compilado sin raylib, se usara el frontend de consola.\n";
  }
  return arcade::runConsoleApp(options);
#else
  if (forceConsole) {
    return arcade::runConsoleApp(options);
  }
  return arcade::runRaylibApp(options);
#endif
}
