#include <math.h>

#include "engine/tachyon_linear_algebra.h"

tMat4f tMat4f::perspective(float fov, float near, float far) {
  constexpr float FOV_DIVISOR = 2.f * 3.141592f / 180.f;
  constexpr float aspectRatio = 1920.f / 1080.f;

  // float aspectRatio = (float)area.width / (float)area.height;
  float f = 1.0f / tanf(fov / FOV_DIVISOR);
  float nf = near - far;

  return {
    f / aspectRatio, 0.0f, 0.0f, 0.0f,
    0.0f, f, 0.0f, 0.0f,
    0.0f, 0.0f, (far + near) / nf, (2 * far * near) / nf,
    0.0f, 0.0f, -1.0f, 0.0f
  };
}