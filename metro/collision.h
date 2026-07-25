#pragma once

#include "metro/game_state.h"

namespace metro {
  struct CollisionTest {
    tVec3f point;
    bool hit = false;
  };

  namespace Collision {
    void AddFloorCollision(State& state, const tObject& object);
    CollisionTest TestRayHit(tVec3f& ray_start, tVec3f& ray, CollisionPlane& plane);
  }
}