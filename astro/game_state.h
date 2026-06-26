#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"
#include "astro/entities.h"

#define MAX_ANIMATED_PEOPLE 10

// @todo pass entity variable name
#define for_entities(array) for (uint16 i = 0; i < (uint16)array.size(); i++)

#define for_entities_of_type(__type)\
  auto& entities = EntityDispatcher::GetEntityContainer(state, __type);\
  for_entities(entities)\

#define for_all_entity_types() for (auto type : entity_types)

#define for_range(__low, __high) for (int i = __low; i <= __high; i++)

#define clamp_to_0(__value) if (__value < 0.f) __value = 0.f
#define clamp_to_1(__value) if (__value > 1.f) __value = 1.f

#define time_since(t) (tachyon->scene.scene_time - t)
#define get_scene_time() tachyon->scene.scene_time
#define is_moving_left_stick() (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)

#define get_speed() state.player_velocity.magnitude()
#define get_speed_ratio() (get_speed() / PlayerCharacter::MAX_RUN_SPEED)

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
      player_satchel,
      player_blanket,
      player_flask,
      player_lantern,

      // Animation bone debugging
      debug_skeleton_bone,

      // Collision debugging
      debug_collision_point,

      // Static environment meshes
      water_plane,
      snow_particle,
      stray_leaf,
      river_leaf,
      dust_mote,

      // Dynamic fauna meshes
      butterfly_left_wing,
      butterfly_right_wing,
      tiny_bird_head,
      tiny_bird_body,
      tiny_bird_feet,
      tiny_bird_wings,
      tiny_bird_left_wing,
      tiny_bird_right_wing,
      duck_body,
      duck_neck,
      duck_wings,
      duck_head,
      duck_beak,
      swan_body,
      swan_beak,
      swan_beak_skin,

      // Clothing + Armor meshes
      lesser_helmet,
      lesser_vambrace,
      low_helmet,
      shoulder_plate,

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
      ladder_rung,

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
      rock_step,
      rock_stair,
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
      player_vambraces,
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
    Plane base_plane;
    Plane visible_plane;
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
   * Camera controller states
   * ----------------------------
   */
  struct CameraState {
    float height_adjustment = 0.f;
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

  enum FlowerParticleType {
    PARTICLE_RISING,
    PARTICLE_FLOATING,
    PARTICLE_FADING_OUT
  };

  struct FlowerParticle {
    float spawn_time = 0.f;
    float lifetime = 1.f;
    float radius = 500.f;
    tVec3f spawn_position;
    tVec3f color = tVec3f(1.f);
    int32 light_id = -1;
    int32 spawning_entity_id = -1;
    FlowerParticleType type = PARTICLE_RISING;
  };

  /**
   * ----------------------------
   * Water flows
   * ----------------------------
   */
  struct WaterFlow {
    std::vector<tVec3f> flow_positions;
  };

  struct WaterFlowLeaf {
    WaterFlow* source_flow = nullptr;
    tObject object;
    float progress = 0.f;
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

  struct TinyBird {
    tVec3f position;
    tVec3f jump_start_position;
    tVec3f target_position;
    Quaternion rotation;

    float timer = 0.f;

    // Parameters for free-flying tiny birds
    float wing_speed = 0.f;
    float wing_value = 0.f;
    float wing_angle = 0.f;
    float steady_flight_duration = 0.5f;

    float last_head_turn_time = 0.f;
    float last_jump_time = 0.f;
    float last_fly_away_time = 0.f;
    float last_wing_flapping_time = 0.f;
    bool flapping_wings = false;
    bool did_land = false;

    enum State {
      IDLING,
      TURN_AROUND,
      JUMP_FORWARD,
      FLY_DOWN_AND_LAND,
      FLY_UP,
      FLY_FORWARD
    } state;
  };

  struct Duck {
    tVec3f position;
    tVec3f target_position;
    Quaternion rotation;
    Quaternion head_rotation;
    float last_target_change_time = 0.f;
    float last_random_head_turn_time = 0.f;
    float current_speed = 200.f;
    float random_head_turn_angle = 0.f;

    EntityRecord spawn_entity_record;
  };

  struct Swan {
    tVec3f position;
    tVec3f target_position;
    Quaternion rotation;
    Quaternion head_rotation;
    float last_target_time = 0.f;

    EntityRecord spawn_entity_record;
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

  struct tAnimationRig {
    tSkeleton rest_pose;
    tSkeleton active_pose;
    tSkeletonAnimation* current_animation = nullptr;
    tSkeletonAnimation* next_animation = nullptr;
    tSkeletonAnimation* upper_body_animation = nullptr;
    tSkeletonAnimation* arm_animation = nullptr;
    float next_animation_blend_alpha = 0.f;
    float head_turn_angle = 0.f;
    float torso_turn_angle = 0.f;
    float torso_tilt_angle = 0.f;
    float torso_compression = 0.f;

    float current_animation_time = 0.f;
    float current_animation_speed = 0.f;
    float next_animation_time = 0.f;
    float next_animation_speed = 0.f;
    float upper_body_animation_time = 0.f;
    float upper_body_animation_speed = 0.f;
  };

  /**
   * ----------------------------
   * NPC/enemy skinned meshes
   * ----------------------------
   */
  struct SkinnedPerson {
    int32 body_mesh_index = -1;
    int32 shirt_mesh_index = -1;
    int32 pants_mesh_index = -1;
    tAnimationRig rig;
  };

  /**
   * ----------------------------
   * Locations
   * ----------------------------
   */
  enum Location {
    LOCATION_UNSPECIFIED = -1,
    TUTORIAL,
    DIVINATION_WOODREALM,
    DIVINATION_RIVERWAY,
    DIVINATION_LAKE_PROMENADE,
    DIVINATION_LAKEFRONT_SOUTH,
    PROVENANCE_WOODREALM_WEST,
    PROVENANCE_WOODREALM_EAST,
    VILLAGE_1,
    FOREST_1,
    FAERIE_FOREST,
    STARGAZING_HILL,
    OLD_WOOD,
    LONELY_FARMHOUSE
  };

  /**
   * ----------------------------
   * Locations
   * ----------------------------
   */
  enum SubStory {
    SUBSTORY_UNSPECIFIED = -1,
    SUBSTORY_SEEKER_STARGAZER
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
    struct Player {
      Quaternion rotation;
      tMat4f rotation_matrix;

      Quaternion visual_rotation;
      tVec3f visual_position;

      float running_charge = 0.f;
      float climb_speed = 0.f;

      // Attachments
      float satchel_freefall = 0.f;
      float blanket_freefall = 0.f;
      float blanket_run_swing = 0.f;

      float last_stopped_moving_time = 0.f;
      float last_wand_start_time = 0.f;
      float last_climbing_time = 0.f;
      float last_climbing_start_time = 0.f;
      float last_climbing_stop_time = 0.f;
      tVec3f climb_up_start_position;

      bool is_looking_at_something = false;
      bool is_airborne_in_run_cycle = false;

      tVec3f planted_left_foot_position;
      bool is_left_foot_planted = false;

      tVec3f planted_right_foot_position;
      bool is_right_foot_planted = false;

      tAnimationRig rig;
    } player;

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
    float last_ledge_jump_time = 0.f;
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
    bool is_moving_down_slope = false;
    bool is_on_wood_surface = false;
    bool is_on_stone_surface = false;
    bool is_on_ladder = false;
    bool is_starting_climb_down = false;
    bool did_resolve_radius_collision = false;
    bool did_resolve_plane_collision = false;
    bool did_jump_off_ledge = false;
    bool did_climb_down = false;
    int32 last_used_wind_chimes_id = -1;

    float previous_move_delta = 0.f;
    float movement_distance = 0.f;
    float last_walk_sound_time = 0.f;
    uint8 walk_cycle = 0;

    float fall_velocity = 0.f;
    float current_ground_y = 0.f;
    float run_oscillation = 0.f;
    float tilt_angle = 0.f;

    // Location
    Location current_location = Location::TUTORIAL;
    Location last_wind_chimes_location = Location::TUTORIAL;
    float last_area_change_time = 0.f;

    // Sub-stories
    SubStory current_substory = SUBSTORY_UNSPECIFIED;
    float last_substory_title_time = 0.f;

    // Animations
    struct SkeletonAnimations {
      tSkeletonAnimation player_idle;
      tSkeletonAnimation player_idle_2;
      tSkeletonAnimation player_idle_wand;
      tSkeletonAnimation player_idle_wand_2;
      tSkeletonAnimation player_idle_quickturn;
      tSkeletonAnimation player_walk;
      tSkeletonAnimation player_walk_wand;
      tSkeletonAnimation player_run;
      tSkeletonAnimation player_run_wand;
      tSkeletonAnimation player_climb;
      tSkeletonAnimation player_climb_up;
      tSkeletonAnimation player_climb_down;
      tSkeletonAnimation player_swing_wand;

      tSkeletonAnimation person_idle;
      tSkeletonAnimation person_talking;
      tSkeletonAnimation person_hit_front;
    } animations;

    uint8 player_idle_stance = 1;
    SkinnedPerson skinned_people[MAX_ANIMATED_PEOPLE];
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
    tVec3f camera_tracking_position;
    tVec3f camera_offset_position;
    bool use_zoomed_out_camera = false;
    float camera_angle = 0.9f;
    float camera_blend_speed = 0.f;
    float camera_height_adjustment = 0.f;
    float target_camera_height_adjustment = 0.f;
    std::unordered_map<std::string, CameraState> camera_state_map;

    // Vantage camera mode
    bool use_vantage_camera = false;
    float recorded_camera_angle = 0.9f;
    // Ensure that the time since the vantage camera change time
    // is 1.0 at game start, treating the camera as fully blended-out
    // from a vantage camera view (the transition-out duration is
    // 1 second). As soon as we trigger the vantage camera again,
    // this will be updated accordingly.
    float vantage_camera_change_time = -1.f;

    float water_level = -1800.f; // @deprecated

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
    float last_wand_hint_time = 0.f;
    float wand_sense_factor = 0.f;
    bool is_holding_up_wand = false;
    int32 wand_hint_light_id = -1;

    // Dynamic fauna
    std::vector<Butterfly> butterflies;
    std::vector<TinyBird> tiny_birds;
    std::vector<Duck> ducks;
    std::vector<Swan> swans;
    float last_tiny_bird_spawn_time = 0.f;
    float tiny_bird_cooldown_time = 0.f;

    // Particles
    std::vector<FlowerParticle> flower_particles;
    std::vector<int32> sculpture_particles;
    std::vector<int32> glow_particle_light_ids;
    float glow_particles_alpha = 0.f;

    // Water flows
    std::vector<WaterFlow> water_flows;
    std::vector<WaterFlowLeaf> water_flow_leaves;

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
    std::unordered_map<std::string, DialogueSet> dialogue_map;
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
      tUIElement* divination_riverway_title = nullptr;
      tUIElement* lake_promenade_title = nullptr;
      tUIElement* lakefront_south_title = nullptr;

      tUIElement* seeker_stargazer_title = nullptr;

      float titles_alpha = 1.f;
    } ui;

    // @todo dev mode only
    tUIText* debug_text = nullptr;
    tUIText* debug_text_large = nullptr;
    bool is_level_editor_open = false;
    bool show_game_stats = false;
    bool use_slow_motion = false;
    bool enemies_disabled = false;
    bool is_rebuilding_procedural_objects = false;
  };
}