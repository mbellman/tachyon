#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entity_types.h"

#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

namespace astro {
  struct MeshIds : EntityMeshIds {
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

      // Editor meshes
      // @todo dev mode only
      gizmo_arrow,
      gizmo_resizer,
      gizmo_rotator;
  };

  // @todo move to engine?
  struct Plane {
    tVec3f p1, p2, p3, p4;
  };

  struct Spells {
    // Stun
    float last_stun_time = 0.f;
    int32 stun_light_id = -1;

    // Homing
    float last_homing_time = 0.f;
    EntityRecord homing_target_entity;
    // @temporary
    // @todo allow multiple homing lights
    int32 homing_light_id = -1;
  };

  struct State : EntityContainers {
    MeshIds meshes;

    // @todo default this in game.cpp or elsewhere
    tVec3f player_position = tVec3f(0.f, 0.f, 3500.f);
    tVec3f last_player_position;
    tVec3f last_solid_ground_position;
    tVec3f player_velocity;
    bool is_on_solid_ground = false;
    Plane last_plane_walked_on;

    tVec3f camera_shift;

    float water_level = -1800.f;

    float astro_time = 0.f;
    float astro_turn_speed = 0.f;
    float astro_time_at_start_of_turn = 0.f;

    Spells spells;

    float last_frame_left_trigger = 0.f;
    float last_frame_right_trigger = 0.f;

    std::string dialogue_message = "";
    float dialogue_start_time = 0.f;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
  };
}