#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

namespace astro {
  struct MeshIds {
    uint16
      player,

      ground_plane,
      water_plane,

      astrolabe_base,
      astrolabe_ring,
      astrolabe_hand,

      // SHRUB
      shrub_placeholder,
      shrub_branches,

      // OAK_TREE
      oak_tree_placeholder,
      oak_tree_trunk,

      // WILLOW_TREE
      willow_tree_placeholder,
      willow_tree_trunk;
  };

  struct State {
    MeshIds meshes;

    // @todo default this in game.cpp or elsewhere
    tVec3f player_position = tVec3f(0.f, 0.f, 3500.f);

    float astro_time = 0.f;
    float astro_turn_speed = 0.f;

    std::vector<PlantEntity> shrubs;
    std::vector<TreeEntity> oak_trees;
    std::vector<TreeEntity> willow_trees;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    bool is_level_editor_open = false;
  };
}