#include "engine/tachyon_easing.h"

float Tachyon_Lerpf(const float a, const float b, const float alpha) {
  return a + (b - a) * alpha;
}

float Tachyon_EaseInOutf(float t) {
  if (t < 0.5f) {
    return 2.f * t * t;
  }

  t -= 0.5f;

  return 2.f * t * (1.f - t) + 0.5f;
}