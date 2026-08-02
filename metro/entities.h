#pragma once

#include "engine/tachyon.h"

#define for_static_entity_containers()\
  for (auto* entities : {\
      &state.platforms,\
      &state.ramps,\
      &state.walkway_segments\
  })

#define for_entities()\
  for (auto& entity : *entities)

namespace metro {
  struct CollisionPlane {
    // Plane corners
    tVec3f p1, p2, p3, p4;
    // Plane tangents
    tVec3f t1, t2, t3, t4;
    tVec3f normal;
  };

  struct StaticEntity {
    int32 id = -1;

    tVec3f position;
    Quaternion rotation = Quaternion(1.f, 0, 0, 0);
    tVec3f scale;
    tVec3f color;

    bool active = true;
    bool needs_update = true;

    // @todo @optimize come up with a non-heap-allocated solution for collision planes,
    // given that we don't know how many we'll need per entity type
    std::vector<CollisionPlane> collision_planes;
  };

  struct InteractiveEntity {
    int32 id = -1;

    // @todo
  };
}