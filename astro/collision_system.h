#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace CollisionSystem {
    Plane CreatePlane(const tVec3f& position, const tVec3f& scale, const Quaternion& rotation);
    bool IsPointOnPlane(const tVec3f& point, const Plane& plane);
    void RebuildFlatGroundPlanes(Tachyon* tachyon, State& state);
    float QueryGroundHeight(State& state, const float x, const float z);
    void HandleCollisions(Tachyon* Tachyon, State& state);
  }
}