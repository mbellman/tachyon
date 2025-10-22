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

  // @todo @optimize
  return 1.f + c3 * pow(t - 1.f, 3.f) + c1 * powf(t - 1.f, 2.f);
}