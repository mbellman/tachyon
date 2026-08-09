#pragma once

#include "engine/tachyon_types.h"

namespace metro {
  struct CollisionPlane {
    // Plane corners
    tVec3f p1, p2, p3, p4;
    // Plane tangents
    tVec3f t1, t2, t3, t4;
    tVec3f normal;
  };

  // @todo
  struct CollisionTriangle {
    // Triangle points
    tVec3f p1, p2, p3;
    // Triangle tangents
    tVec3f t1, t2, t3;
    tVec3f normal;
  };

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
    void PadCollisionPlane(CollisionPlane& plane, const float padding);
    void PrepareCollisionPlane(CollisionPlane& plane);
    CollisionPlane CreateFloorCollisionPlane(const Transform& transform);
    CollisionPlane CreateSlopeCollisionPlane(const Transform& transform);
    CollisionTest TestRayHit(tVec3f& ray_start, tVec3f& ray, CollisionPlane& plane);
  }
}