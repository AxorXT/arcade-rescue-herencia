#pragma once

#include "arcade/game.hpp"

namespace arcade {

// Piloto automatico muy simple basado en BFS.
//
// Se usa en tres sitios:
//  - modo --demo del frontend grafico (modo atraccion para capturas),
//  - simulacion --headless (smoke test jugando de verdad),
//  - pruebas de integracion que necesitan partidas largas y validas.
//
// No forma parte de las reglas del juego: solo elige direcciones legales.
class AutoPilot {
 public:
  // Pide un giro hacia la comida si hay camino; si no, elige la direccion
  // segura con mas espacio libre. Puede no pedir nada (la serpiente morira).
  void steer(Game& game);

  // Numero de decisiones tomadas (utilidad para pruebas y metricas).
  int decisions() const noexcept { return decisions_; }

 private:
  int decisions_{0};
};

} // namespace arcade
