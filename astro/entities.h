#pragma once

#include "engine/tachyon.h"

namespace astro {
  enum EntityType {
    UNSPECIFIED = -1,
    SHRUB,
    OAK_TREE,
    WILLOW_TREE
  };

  static std::vector<EntityType> entity_types = {
    SHRUB,
    OAK_TREE,
    WILLOW_TREE
  };

  struct EntityRecord {
    int32 id = -1;
    EntityType type = UNSPECIFIED;
  };

  struct GameEntity : EntityRecord {
    tVec3f position;
    tVec3f scale;
    tVec3f tint;
    Quaternion orientation = Quaternion(1.f, 0, 0, 0);

    float astro_start_time = 0.f;
    float astro_end_time = 0.f;
  };
}