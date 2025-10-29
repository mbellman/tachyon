#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace CollisionSystem {
    void HandleCollisions(Tachyon* Tachyon, State& state, const float dt);
    Plane CreatePlane(const tVec3f& position, const tVec3f& scale, const Quaternion& rotation);
    bool IsPointOnPlane(const tVec3f& point, const Plane& plane);
  }
}