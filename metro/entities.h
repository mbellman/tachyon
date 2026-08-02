#pragma once

#include "engine/tachyon.h"

namespace metro {
  struct StaticEntity {
    tVec3f position;
    Quaternion rotation = Quaternion(1.f, 0, 0, 0);
    tVec3f scale;
    tVec3f color;

    bool active = true;
    bool modified = false;
  };

  struct InteractiveEntity {
    // @todo
  };
}