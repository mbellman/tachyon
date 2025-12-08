#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

// @todo pass entity variable name
#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

#define time_since(t) (tachyon->scene.scene_time - t)
#define get_scene_time() tachyon->scene.scene_time

namespace astro {
  /**
   * ----------------------------
   * Meshes
   * ----------------------------
   */
  struct MeshIds : EntityMeshIds {
    uint16
      // Character meshes
      player,
      wand,

      // Static environment meshes
      water_plane,
      snow_particle,

      // Procedural meshes
      grass,
      small_grass,
      ground_flower,
      tiny_ground_flower,
      bush_flower,
      ground_1_leaves,
      p_dirt_path, // @todo rename

      // HUD meshes
      astrolabe_rear,
      astrolabe_base,
      astrolabe_plate,
      astrolabe_fragment_ul,
      astrolabe_fragment_ll,
      astrolabe_ring,
      astrolabe_hand,
      target_reticle,

      // Item meshes
      item_astro_part,
      item_gate_key,

      // Decorative meshes
      flat_ground,
      rock_1,
      river_edge,
      ground_1,
      lookout_tower,

      // Editor meshes
      // @todo dev mode only
      gizmo_arrow,
      gizmo_resizer,
      gizmo_rotator,
      editor_placer;
  };

  // @todo move to engine?
  struct Plane {
    tVec3f p1, p2, p3, p4;
  };

  /**
   * ----------------------------
   * Spellcasting
   * ----------------------------
   */
  struct HomingOrb {
    int32 light_id = -1;
    float targeting_start_time = 0.f;
    float targeting_start_speed = 0.f;
    tVec3f targeting_start_direction;
    bool is_targeting = false;
  };

  struct Spells {
    // Stun
    float stun_start_time = 0.f;
    int32 stun_light_id = -1;
    bool did_cast_stun_this_frame = false;

    // Homing
    float homing_start_time = 0.f;
    tVec3f homing_start_direction;
    EntityRecord homing_target_entity;
    HomingOrb homing_orbs[3];
  };

  /**
   * ----------------------------
   * Items and inventory management
   * ----------------------------
   */
  enum ItemType {
    ITEM_UNSPECIFIED = -1,
    ASTROLABE_LOWER_LEFT,
    ASTROLABE_LOWER_RIGHT,
    ASTROLABE_UPPER_RIGHT,
    GATE_KEY,

    ITEM_STUN_SPELL,
    ITEM_HOMING_SPELL
  };

  struct Item {
    ItemType type = ITEM_UNSPECIFIED;
  };

  /**
   * ----------------------------
   * Path segments
   * ----------------------------
   */
  // @todo put color and astro time stuff on here
  struct PathSegment {
    int32 index;
    tVec3f base_position;
    tVec3f base_scale;
    uint16 entity_index_a = 0;
    uint16 entity_index_b = 0;
    tObject object;
    Plane plane;
  };

  /**
   * ----------------------------
   * Grass
   * ----------------------------
   */
  struct GrassBlade {
    tVec3f position;
    tVec3f scale;
    int32 path_segment_index = -1;
    uint16 active_object_id = 0xFFFF;
  };

  struct GrassChunk {
    tVec3f center_position;
    std::vector<GrassBlade> grass_blades;
    bool is_currently_in_view = false;
  };

  /**
   * ----------------------------
   * Game state
   * ----------------------------
   */
  struct State : EntityContainers {
    MeshIds meshes;

    float dt = 0.f;

    // Player attributes
    tVec3f player_position;
    tVec3f last_player_position;
    tVec3f last_solid_ground_position;
    tVec3f player_velocity;
    tVec3f player_facing_direction;
    int32 player_light_id = -1;
    Plane last_plane_walked_on;
    float player_hp = 100.f;
    float last_damage_time = 0.f;
    float last_wand_swing_time = 0.f;
    float death_time = 0.f;
    bool is_on_solid_ground = false;
    bool did_resolve_radius_collision = false;

    float movement_distance = 0.f;
    float last_walk_sound_movement_distance = 0.f;
    uint8 walk_cycle = 0;

    // Large-scale generated elements
    std::vector<PathSegment> dirt_path_segments;
    std::vector<GrassChunk> grass_chunks;

    // Targeted entities
    EntityRecord target_entity; // @todo rename target_entity_record
    EntityRecord speaking_entity_record;
    float target_start_time = 0.f;
    float last_run_input_time = 0.f;
    float last_dodge_time = 0.f;
    bool has_target = false;
    std::vector<EntityRecord> targetable_entities;

    tVec3f camera_shift;
    bool use_zoomed_out_camera = false;

    float water_level = -1800.f;

    // Astro properties
    float astro_time = 0.f;
    float astro_turn_speed = 0.f;
    float astro_time_at_start_of_turn = 0.f;
    float game_time_at_start_of_turn = 0.f;
    float time_warp_start_radius = 30000.f;
    float time_warp_end_radius = 30000.f;
    // @todo use a float from 0.0 -> 1.0 representing night -> day
    bool is_nighttime = false;
    bool is_astrolabe_stopped = true;
    bool is_astro_traveling = false;
    float astrolabe_visibility = 1.f; // @todo update effect or remove
    int32 astrolabe_light_id = -1;

    Spells spells;

    std::vector<Item> inventory;

    float last_frame_left_trigger = 0.f;
    float last_frame_right_trigger = 0.f;

    // Dialogue
    std::string dialogue_message = "";
    float dialogue_start_time = 0.f;
    float last_dialogue_sound_time = 0.f;
    bool has_blocking_dialogue = false;
    bool dismissed_blocking_dialogue = false;
    std::unordered_map<std::string, std::vector<std::string>> npc_dialogue;
    std::string current_dialogue_sequence = "";
    int32 current_dialogue_step = 0;

    // Music
    float bgm_start_time = -1.f;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
    bool show_game_stats = false;
  };
}