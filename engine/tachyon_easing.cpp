#include <math.h>

#include "engine/tachyon_easing.h"

float Tachyon_Lerpf(const float a, const float b, const float alpha) {
  return a + (b - a) * alpha;
}

float Tachyon_EaseOutQuad(float t) {
  return 1.f - (1.f - t) * (1.f - t);
}

float Tachyon_EaseInOutf(float t) {
  if (t < 0.5f) {
    return 2.f * t * t;
  }

  t -= 0.5f;

  return 2.f * t * (1.f - t) + 0.5f;
}

/**
 * Adapted from https://easings.net/#easeOutBack
 */
float Tachyon_EaseOutBackf(float t) {
  float c1 = 2.f;
  float c3 = c1 + 1.f;
  float t_1 = t - 1.f;

  return 1.f + c3 * (t_1 * t_1 * t_1) + c1 * (t_1 * t_1);
}