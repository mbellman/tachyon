#pragma once

#include "metro/game_state.h"

namespace metro {
  struct DebugLineConfig {
    tVec3f position;
    tVec3f vector;
    tVec3f color;
    float thickness;
  };

  struct DebugShapeConfig {
    tVec3f position;
    tVec3f scale;
    tVec3f direction;
    tVec3f color;
  };

  struct DebugCubeConfig {
    tVec3f position;
    tVec3f scale;
    Quaternion rotation;
    tVec3f color;
  };

  namespace Debug {
    void Reset(Tachyon* tachyon, State& state);
    void ShowDebugLine(Tachyon* tachyon, State& state, const DebugLineConfig& config);
    void ShowDebugPlane(Tachyon* tachyon, State& state, const CollisionPlane& plane, const tVec3f& color);
    void ShowDebugCube(Tachyon* tachyon, State& state, const DebugCubeConfig& config);
    void ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius);
    void ShowDebugRing(Tachyon* tachyon, State& state, const DebugShapeConfig& config);
    void ShowDebugCone(Tachyon* tachyon, State& state, const DebugShapeConfig& config);
    void ShowDebugVector(Tachyon* tachyon, State& state, const tVec3f& position, const tVec3f& vector, const tVec3f& color);
  }
}