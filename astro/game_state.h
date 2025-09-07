#pragma once

#include "engine/tachyon_aliases.h"

namespace astro {
  struct MeshIds {
    uint16
      cube,
      plane;
  };

  struct State {
    MeshIds meshes;

    tVec3f player_position = tVec3f(0.f);
  };
}