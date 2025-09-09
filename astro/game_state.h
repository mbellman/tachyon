#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

namespace astro {
  struct MeshIds {
    uint16
      player,

      ground_plane,
      water_plane;
  };

  struct State {
    MeshIds meshes;

    tVec3f player_position = tVec3f(0.f);

    // @todo debug mode only
    tUIText* debug_text = nullptr;
  };
}