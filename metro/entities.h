#pragma once

#include "engine/tachyon.h"

namespace metro {
  struct StaticEntity {
    tVec3f position;
    Quaternion rotation;
    tVec3f scale;
    tVec3f color;

    bool active = true;
  };

  struct InteractiveEntity {
    // @todo
  };
}