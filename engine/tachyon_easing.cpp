#include "engine/tachyon_easing.h"

float Tachyon_Lerpf(const float a, const float b, const float alpha) {
  return a + (b - a) * alpha;
}