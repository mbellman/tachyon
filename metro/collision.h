#pragma once

#include "metro/game_state.h"

namespace metro {
  struct CollisionTest {
    tVec3f collision_point;
    bool has_collision = false;
  };

  struct Transform {
    tVec3f position;
    Quaternion rotation;
    tVec3f scale;

    Transform(const tObject& object):
      position(object.position),
      rotation(object.rotation),
      scale(object.scale) {};
  };

  namespace Collision {
    CollisionPlane CreateFloorCollisionPlane(const Transform& transform);
    CollisionPlane CreateSlopeCollisionPlane(const Transform& transform);
    CollisionTest TestRayHit(tVec3f& ray_start, tVec3f& ray, CollisionPlane& plane);
  }
}