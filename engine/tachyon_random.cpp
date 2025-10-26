#include "engine/tachyon_random.h"

static std::random_device random_device;
static std::default_random_engine non_deterministic_random_engine(random_device());
static std::uniform_real_distribution<float> unit_distribution(0.f, 1.f);

tRNG::tRNG() {
  engine = std::default_random_engine(random_device());
}

tRNG::tRNG(float seed) {
  engine = std::default_random_engine(seed);
}

float tRNG::Random() {
  return unit_distribution(engine);
}

float tRNG::Random(float low, float high) {
  return low + Random() * (high - low);
}

int tRNG::RandomInt(int low, int high) {
  return low + int(Random() * (high - low + 1));
}

float Tachyon_GetRandom() {
  return unit_distribution(non_deterministic_random_engine);
}

float Tachyon_GetRandom(float low, float high) {
  return low + Tachyon_GetRandom() * (high - low);
}

int Tachyon_GetRandom(int low, int high) {
  return low + int(Tachyon_GetRandom() * (high - low + 1));
}