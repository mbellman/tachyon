#include <math.h>
#include <random>

#include "engine/tachyon_random.h"

static std::random_device randomDevice;
static std::default_random_engine randomEngine(randomDevice());
static std::uniform_real_distribution<float> randomRange(0.f, 1.f);

float Tachyon_GetRandom() {
  return randomRange(randomEngine);
}

float Tachyon_GetRandom(float low, float high) {
  return low + Tachyon_GetRandom() * (high - low);
}

int Tachyon_GetRandom(int low, int high) {
  return low + int(Tachyon_GetRandom() * (high - low + 1));
}