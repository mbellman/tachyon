#pragma once

#include "metro/game_state.h"

namespace metro {
  struct DebugConeConfig {
    tVec3f position;
    tVec3f scale;
    tVec3f direction;
    tVec3f color;
  };

  struct DebugLineConfig {
    tVec3f position;
    tVec3f vector;
    tVec3f color;
    float thickness;
  };

  namespace Debug {
    void Reset(Tachyon* tachyon, State& state);
    void ShowDebugLine(Tachyon* tachyon, State& state, const DebugLineConfig& config);
    void ShowDebugCone(Tachyon* tachyon, State& state, const DebugConeConfig& config);
    void ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius);
    void ShowDebugVector(Tachyon* tachyon, State& state, const tVec3f& position, const tVec3f& vector, const tVec3f& color);
    void ShowDebugPlane(Tachyon* tachyon, State& state, const CollisionPlane& plane, const tVec3f& color);
  }
}