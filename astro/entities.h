#pragma once

#include "engine/tachyon.h"

namespace astro {
  enum EntityType {
    UNSPECIFIED = -1,
    OAK_TREE,
    WILLOW_TREE
  };

  struct EntityRecord {
    int32 id;
    EntityType type = UNSPECIFIED;
  };

  struct BaseEntity : EntityRecord {
    tVec3f position;
    tVec3f scale;
    tVec3f tint;
    Quaternion orientation = Quaternion(1.f, 0, 0, 0);
  };

  struct Living {
    float astro_time_when_born = 0.f;
  };

  struct TreeEntity : BaseEntity, Living {};
}