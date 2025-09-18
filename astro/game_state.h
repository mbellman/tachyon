#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

namespace astro {
  struct MeshIds {
    uint16
      // Character meshes
      player,

      // Static environment meshes
      ground_plane,
      water_plane,

      // HUD meshes
      astrolabe_base,
      astrolabe_ring,
      astrolabe_hand,

      // Decorative meshes
      rock_1,
      ground_1,

      // SHRUB
      shrub_placeholder,
      shrub_branches,

      // OAK_TREE
      oak_tree_placeholder,
      oak_tree_trunk,

      // WILLOW_TREE
      willow_tree_placeholder,
      willow_tree_trunk,
      
      // Editor meshes
      // @todo dev mode only
      gizmo_arrow,
      gizmo_resizer;
  };

  struct State {
    MeshIds meshes;

    // @todo default this in game.cpp or elsewhere
    tVec3f player_position = tVec3f(0.f, 0.f, 3500.f);

    float astro_time = 0.f;
    float astro_turn_speed = 0.f;

    std::vector<GameEntity> shrubs;
    std::vector<GameEntity> oak_trees;
    std::vector<GameEntity> willow_trees;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
  };
}