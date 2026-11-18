#include "arcade/rng.hpp"

#include <limits>

namespace arcade {

namespace {
constexpr std::uint32_t kDefaultSeed = 0x9E3779B9U;
constexpr int kSeedWarmupRounds = 8;
} // namespace

RandomGenerator::RandomGenerator(std::uint32_t seed) noexcept {
  reseed(seed);
}

void RandomGenerator::reseed(std::uint32_t seed) noexcept {
  // xorshift32 no puede tener estado 0: se queda atascado en 0 para siempre.
  state_ = (seed == 0U) ? kDefaultSeed : seed;
  for (int round = 0; round < kSeedWarmupRounds; ++round) {
    nextUInt();
  }
}

std::uint32_t RandomGenerator::nextUInt() noexcept {
  state_ ^= state_ << 13;
  state_ ^= state_ >> 17;
  state_ ^= state_ << 5;
  return state_;
}

int RandomGenerator::nextInt(int upperBound) noexcept {
  if (upperBound <= 1) {
    return 0;
  }

  // Muestreo por rechazo: el modulo puro estaria sesgado hacia los valores
  // bajos. Se descarta la cola que no completa un bloque entero.
  const auto bound = static_cast<std::uint32_t>(upperBound);
  const std::uint32_t maximum = std::numeric_limits<std::uint32_t>::max();
  const std::uint32_t limit = maximum - (maximum % bound);

  std::uint32_t value = nextUInt();
  while (value >= limit) {
    value = nextUInt();
  }
  return static_cast<int>(value % bound);
}

} // namespace arcade
