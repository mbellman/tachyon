#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

#define MAX_ANIMATED_PEOPLE 5

// @todo pass entity variable name
#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

#define for_range(__low, __high) for (int i = __low; i <= __high; i++)

#define time_since(t) (tachyon->scene.scene_time - t)
#define get_scene_time() tachyon->scene.scene_time

namespace astro {
  /**
   * ----------------------------
   * Meshes
   * ----------------------------
   */
  struct MeshIds : EntityMeshIds {
    // Static meshes
    uint16
      // Player meshes
      player_head,
      player_wand,
      player_lantern,

      // Animation bone debugging
      debug_skeleton_bone,

      // Static environment meshes
      water_plane,
      snow_particle,
      stray_leaf,
      dust_mote,

      // Dynamic fauna meshes
      butterfly_left_wing,
      butterfly_right_wing,

      // Clothing + Armor meshes
      lesser_helmet,

      // Procedural meshes
      grass, // @todo rename ground_1_grass
      small_grass,
      ground_flower,
      tiny_ground_flower,
      ground_1_flower,
      bush_flower,
      bush_flower_2,
      flower_middle,
      tree_mushroom,
      vine_leaf,
      vine_flower,
      tree_flower,

      // Procedural path meshes
      dirt_path,
      rock_dirt,
      stone_path,
      path_stone,

      // HUD meshes
      astrolabe_rear,
      astrolabe_base,
      astrolabe_plate,
      astrolabe_plate2,
      astrolabe_plate3,
      astrolabe_plate4,
      astrolabe_ring,
      astrolabe_hand,
      target_reticle,

      // Item meshes
      item_astro_part,
      item_gate_key,

      // Decorative meshes
      flat_ground,
      rock_1,
      rock_2,
      river_edge,
      ground_1,
      lookout_tower,
      stairs_floor,

      // Facade meshes
      facade_sg_castle,

      // Editor meshes
      // @todo dev mode only
      gizmo_arrow,
      gizmo_resizer,
      gizmo_rotator,
      editor_placer;

    // Skinned meshes
    int32
      player_hood,
      player_robes,
      player_trim,
      player_shirt,
      player_pants,
      player_boots,
      player_belt;
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

    MAGIC_WAND,
    CHIME_PARTS,
    GATE_KEY,

    ITEM_STUN_SPELL,
    ITEM_MAGIC_WEAPON,
    ITEM_HOMING_SPELL
  };

  struct Item {
    ItemType type = ITEM_UNSPECIFIED;
  };

  /**
   * ----------------------------
   * Path generation
   * ----------------------------
   */
  struct PathNode {
    uint16 entity_index = 0;
    tVec3f position;
    tVec3f scale;

    uint16 connections[4] = { 0, 0, 0, 0 };
    uint16 total_connections = 0;

    uint16 connections_walked[4] = { 0, 0, 0, 0 };
    uint16 total_connections_walked = 0;
  };

  struct PathNetwork {
    PathNode* nodes = nullptr;
    uint16 total_nodes = 0;
  };

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
   * Small grass
   * ----------------------------
   */
  struct GrassBlade {
    tVec3f position;
    tVec3f scale;
    tColor color;
    int32 dirt_path_segment_index = -1;
    int32 stone_path_segment_index = -1;
    uint16 active_object_id = 0xFFFF;
  };

  struct GrassChunk {
    tVec3f center_position;
    std::vector<GrassBlade> grass_blades;
    bool is_currently_in_view = false;
  };

  /**
   * ----------------------------
   * Larger grass
   * ----------------------------
   */
  struct GroundPlantCluster {
    tVec3f position;
    std::vector<tVec3f> grass_positions;
    std::vector<tVec3f> flower_positions;
    std::vector<uint16> grass_object_ids;
    std::vector<uint16> flower_object_ids;
    bool is_currently_in_view = false;
  };

  /**
   * ----------------------------
   * Dialogue
   * ----------------------------
   */
  struct DialogueSet {
    bool random = false;
    bool invoked = false;
    std::vector<std::string> lines;
    int32 returning_first_line_index = 0;
  };

  /**
   * ----------------------------
   * Particles
   * ----------------------------
   */
  struct WandLight {
    tVec3f position;
    float spawn_time = 0.f;
    int32 light_id = -1;
  };

  enum AmbientParticleType {
    PARTICLE_RISING,
    PARTICLE_FLOATING,
    PARTICLE_FADING_OUT
  };

  struct AmbientParticle {
    float spawn_time = 0.f;
    float lifetime = 1.f;
    float radius = 500.f;
    tVec3f spawn_position;
    tVec3f color = tVec3f(1.f);
    int32 light_id = -1;
    int32 spawning_entity_id = -1;
    AmbientParticleType type = PARTICLE_RISING;
  };

  /**
   * ----------------------------
   * Events
   * ----------------------------
   */
  struct BaseEvent {
    float start_time = 0.f;
    float end_time = 0.f;
  };

  struct CameraEvent : BaseEvent {
    EntityRecord target_entity_record;
    float blend_factor = 1.f;
  };

  struct EntityMoveEvent : BaseEvent {
    EntityRecord entity_record;
    tVec3f start_position;
    tVec3f end_position;
  };

  /**
   * ----------------------------
   * Dynamic fauna
   * ----------------------------
   */
  struct Butterfly {
    tVec3f position;
    tVec3f direction;
    uint16 left_wing;
    uint16 right_wing;

    enum State {
      FLYING_STRAIGHT,
      TURNING_LEFT,
      TURNING_RIGHT
    } state;

    float last_state_change_time = 0.f;
  };

  /**
   * ----------------------------
   * Animation
   * @todo move to engine
   * ----------------------------
   */
  struct tSkeletonAnimation {
    std::vector<tSkeleton> frames;
    tSkeleton evaluated_pose;
    std::string name = "";
  };

  struct tSkinnedMeshAnimation {
    tSkeleton rest_pose;
    tSkeleton active_pose;
    tSkeletonAnimation* current_animation = nullptr;
    tSkeletonAnimation* next_animation = nullptr;
    tSkeletonAnimation* upper_body_animation = nullptr;
    tSkeletonAnimation* arm_animation = nullptr;
    float seek_time = 0.f;
    float next_animation_blend_alpha = 0.f;
    float head_turn_angle = 0.f;
    float torso_turn_angle = 0.f;
    float upper_body_animation_time = 0.f;
  };

  /**
   * ----------------------------
   * NPC/enemy skinned meshes
   * ----------------------------
   */
  struct ReservedSkinnedMesh {
    int32 mesh_index = -1;
    tSkinnedMeshAnimation animation;
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
    std::vector<tVec3f> previous_player_positions;
    tVec3f last_solid_ground_position;
    tVec3f player_velocity;
    tVec3f player_facing_direction;
    int32 player_light_id = -1;
    Plane last_plane_walked_on;
    float player_hp = 100.f;
    float last_damage_time = 0.f;
    float last_quick_turn_time = 0.f;
    float last_wand_swing_time = 0.f;
    float last_wand_action_time = 0.f;
    float last_wand_bounce_time = 0.f;
    float last_auto_hop_time = 0.f;
    float last_death_time = 0.f;
    float last_spawn_time = 0.f;
    float last_dodge_time = 0.f;
    float last_target_jump_time = 0.f;
    float last_break_attack_time = 0.f;
    float last_strong_attack_time = 0.f;
    float last_run_input_time = 0.f;
    float last_wind_chimes_action_time = 0.f;
    bool is_on_solid_ground = false;
    bool is_on_solid_platform = false;
    bool did_resolve_radius_collision = false;
    int32 last_used_wind_chimes_id = -1;

    float previous_move_delta = 0.f;
    float movement_distance = 0.f;
    float last_walk_sound_movement_distance = 0.f;
    uint8 walk_cycle = 0;

    float fall_velocity = 0.f;
    float current_ground_y = 0.f;
    float run_oscillation = 0.f;
    float tilt_angle = 0.f;

    // Animations
    struct SkeletonAnimations {
      tSkeletonAnimation player_idle;
      tSkeletonAnimation player_idle_wand;
      tSkeletonAnimation player_walk;
      tSkeletonAnimation player_walk_wand;
      tSkeletonAnimation player_run;
      tSkeletonAnimation player_run_wand;
      tSkeletonAnimation player_swing_wand;

      tSkeletonAnimation person_idle;
      tSkeletonAnimation person_talking;
      tSkeletonAnimation person_hit_front;
    } animations;

    tSkinnedMeshAnimation player_mesh_animation;
    ReservedSkinnedMesh person_skinned_meshes[MAX_ANIMATED_PEOPLE];
    int32 total_animated_people = 0;

    // Large-scale generated elements
    std::vector<PathSegment> dirt_path_segments;
    std::vector<PathSegment> stone_path_segments;
    std::vector<GrassChunk> grass_chunks;
    std::vector<GroundPlantCluster> ground_plant_clusters;

    // Flat ground planes, sorted by descending y for world xz height queries
    std::vector<Plane> flat_ground_planes;

    // Targeted entities
    EntityRecord target_entity; // @todo rename target_entity_record
    EntityRecord preview_target_entity_record;
    EntityRecord speaking_entity_record;
    float target_start_time = 0.f;
    bool has_target = false;
    std::vector<EntityRecord> targetable_entities;

    // Camera attributes
    tVec3f camera_shift;
    bool use_zoomed_out_camera = false;
    float camera_angle = 0.9f;
    float camera_blend_speed = 0.f;

    // Vantage camera mode
    bool use_vantage_camera = false;
    float recorded_camera_angle = 0.9f;
    // Ensure that the time since the vantage camera change time
    // is 1.0 at game start, treating the camera as fully blended-out
    // from a vantage camera view (the transition-out duration is
    // 1 second). As soon as we trigger the vantage camera again,
    // this will be updated accordingly.
    float vantage_camera_change_time = -1.f;

    float water_level = -1800.f;

    // Astro properties
    float astro_time = 76.f;
    float target_astro_time = 76.f;
    float astro_turn_speed = 0.f;
    float astro_time_at_start_of_turn = 0.f;
    float game_time_at_start_of_turn = 0.f;
    tVec3f astro_particle_spawn_position;
    float last_astro_turn_direction = 0.f;
    float time_warp_start_radius = 30000.f;
    float time_warp_end_radius = 30000.f;
    // @todo use a float from 0.0 -> 1.0 representing night -> day
    bool is_nighttime = false;
    bool is_astrolabe_stopped = true;
    bool is_astro_traveling = false;
    int32 astrolabe_light_id = -1;
    std::vector<int32> astro_light_ids;

    // Wand effects
    std::vector<WandLight> wand_lights;
    float last_wand_light_time = 0.f;
    float last_wand_strike_time = 0.f;
    float last_wand_light_pulse_time = 0.f;
    float wand_sense_factor = 0.f;
    float wand_hold_factor = 0.f;

    // Dynamic fauna
    std::vector<Butterfly> butterflies;

    // Particles
    std::vector<AmbientParticle> ambient_particles;
    std::vector<int32> sculpture_particles;

    // Magic
    Spells spells;

    // Items
    std::vector<Item> inventory;

    // @deprecated @todo remove
    float last_frame_left_trigger = 0.f;
    float last_frame_right_trigger = 0.f;

    // Dialogue
    std::string dialogue_message = "";
    float dialogue_start_time = 0.f;
    float dialogue_duration = 5.f;
    float last_dialogue_sound_time = 0.f;
    bool has_blocking_dialogue = false;
    bool dismissed_blocking_dialogue = false;
    std::unordered_map<std::string, DialogueSet> npc_dialogue;
    std::string current_dialogue_set = "";
    int32 current_dialogue_step = 0;

    // Events
    std::vector<CameraEvent> camera_events;
    std::vector<EntityMoveEvent> move_events;

    // Music
    bool music_enabled = true;
    float bgm_start_time = -1.f;

    // UI
    struct {
      tUIElement* future_age_title = nullptr;
      tUIElement* present_age_title = nullptr;
      tUIElement* past_age_title = nullptr;

      tUIElement* divination_woodrealm_title = nullptr;
      tUIElement* lake_promenade_title = nullptr;

      float titles_alpha = 1.f;
    } ui;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
    bool show_game_stats = false;
    bool use_slow_motion = false;
  };
}