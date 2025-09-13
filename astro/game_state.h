#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

namespace astro {
  struct MeshIds {
    uint16
      player,

      ground_plane,
      water_plane,

      // OAK_TREE
      oak_tree_trunk,

      // WILLOW_TREE
      willow_tree_trunk;
  };

  struct State {
    MeshIds meshes;

    tVec3f player_position = tVec3f(0.f, 0.f, 3500.f);

    float astro_time = 0.f;
    float astro_turn_speed = 0.f;

    // @todo debug mode only
    tUIText* debug_text = nullptr;

    std::vector<TreeEntity> oak_trees;
    std::vector<TreeEntity> willow_trees;
  };
}