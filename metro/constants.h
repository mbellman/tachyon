#pragma once

#include "engine/tachyon_input.h"
#include "engine/tachyon_types.h"

namespace metro {
  const static tVec3f AXIS_X = tVec3f(1.f, 0, 0);
  const static tVec3f AXIS_Y = tVec3f(0, 1.f, 0);
  const static tVec3f AXIS_Z = tVec3f(0, 0, 1.f);

  const static tVec3f Y_UP = tVec3f(0, 1.f, 0);
  const static tVec3f Z_BACKWARD = tVec3f(0, 0, -1.f);

  const static auto GAMEPAD_X = tKey::CONTROLLER_A;
  const static auto GAMEPAD_O = tKey::CONTROLLER_B;
  const static auto GAMEPAD_SQUARE = tKey::CONTROLLER_X;
  const static auto GAMEPAD_TRIANGLE = tKey::CONTROLLER_Y;
}