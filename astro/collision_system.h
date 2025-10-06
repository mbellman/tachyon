#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace CollisionSystem {
    void HandleCollisions(Tachyon* Tachyon, State& state);
    Plane GetObjectPlane(const tObject& object);
    bool IsPointOnPlane(const tVec3f& point, const Plane& plane);
  }
}