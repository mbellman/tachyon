#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entity_types.h"

#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

namespace astro {
  struct MeshIds {
    uint16
      // Character meshes
      player,

      // Static environment meshes
      water_plane,

      // HUD meshes
      astrolabe_base,
      astrolabe_ring,
      astrolabe_hand,

      // Decorative meshes
      flat_ground,
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

      // SMALL_STONE_BRIDGE
      small_stone_bridge_placeholder,
      small_stone_bridge_columns,
      small_stone_bridge_base,

      // Editor meshes
      // @todo dev mode only
      gizmo_arrow,
      gizmo_resizer,
      gizmo_rotator;
  };

  struct State {
    MeshIds meshes;

    // @todo default this in game.cpp or elsewhere
    tVec3f player_position = tVec3f(0.f, 0.f, 3500.f);
    tVec3f player_velocity;

    tVec3f camera_shift;

    float astro_time = 0.f;
    float astro_turn_speed = 0.f;

    std::vector<GameEntity> shrubs;
    std::vector<GameEntity> oak_trees;
    std::vector<GameEntity> willow_trees;
    std::vector<GameEntity> small_stone_bridges;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
  };
}