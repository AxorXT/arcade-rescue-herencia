#pragma once

#include <cstdint>

namespace arcade {

// Generador xorshift32: rapido, deterministico y sin dependencias externas.
// La semilla permite reproducir exactamente una partida (clave para las
// pruebas automatizadas y para los bugs reportados por QA).
class RandomGenerator {
 public:
  explicit RandomGenerator(std::uint32_t seed = 0x9E3779B9U) noexcept;

  void reseed(std::uint32_t seed) noexcept;

  // Siguiente valor de 32 bits.
  std::uint32_t nextUInt() noexcept;

  // Entero uniforme en [0, upperBound). Devuelve 0 si upperBound <= 1.
  int nextInt(int upperBound) noexcept;

 private:
  std::uint32_t state_;
};

} // namespace arcade
