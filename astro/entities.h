#pragma once

#include <map>
#include <string>

#include "engine/tachyon.h"

namespace astro {
  /**
   * ----------------------------
   * Entity type constants.
   * ----------------------------
   */
  enum EntityType {
    UNSPECIFIED = -1,
    ALTAR,
    AREA_CHANGE,
    BANDIT,
    BELLFLOWER,
    BIRCH_TREE,
    BIRD_GATE,
    BIRD_SPAWN,
    CAMERA_CONTROLLER,
    CASTLE_RAMPART,
    CASTLE_STEEPLE,
    CASTLE_TOWER,
    CHESTNUT_TREE,
    DIRT_PATH_NODE,
    DUCK_SPAWN,
    EVENT_TRIGGER,
    FAERIE,
    FLAG,
    FLOWER_BUSH,
    FOG_SPAWN,
    GATE,
    GLOW_FLOWER,
    GROUND_FLOWER_PATCH,
    HOUSE,
    IRON_GATE,
    ITEM_PICKUP,
    LADDER,
    LAMP,
    LAMPPOST,
    LEAF_SHRUB,
    LESSER_GUARD,
    LIGHT_POST,
    LILAC_BUSH,
    LILY_PAD,
    LILY_PAD_CLUSTER,
    LOW_GUARD,
    MAGIC_GATE,
    MOOD_LIGHT,
    MUSHROOM,
    NPC,
    OAK_TREE,
    RIVER_LOG,
    ROSE_BUSH,
    SCULPTURE_1,
    SHRUB,
    SIGNPOST,
    SMALL_BIRD,
    SMALL_STONE_BRIDGE,
    STARFLOWER,
    STONE_PATH_NODE,
    STONE_WALL,
    SUNBEAM,
    TALL_GRASS,
    TALL_WEEDS,
    TULIP_PLANT,
    VANTAGE_SPOT,
    WATER_FLOW_NODE,
    WATER_WHEEL,
    WILLOW_TREE,
    WIND_CHIMES,
    WOODEN_BRIDGE,
    WOODEN_FENCE,
    WOODEN_GATE_DOOR
  };

  /**
   * ----------------------------
   * An iterable list of valid entity types.
   * ----------------------------
   */
  static std::vector<EntityType> entity_types = {
    ALTAR,
    AREA_CHANGE,
    BANDIT,
    BELLFLOWER,
    BIRCH_TREE,
    BIRD_GATE,
    BIRD_SPAWN,
    CAMERA_CONTROLLER,
    CASTLE_RAMPART,
    CASTLE_STEEPLE,
    CASTLE_TOWER,
    CHESTNUT_TREE,
    DIRT_PATH_NODE,
    DUCK_SPAWN,
    EVENT_TRIGGER,
    FAERIE,
    FLAG,
    FLOWER_BUSH,
    FOG_SPAWN,
    GATE,
    GLOW_FLOWER,
    GROUND_FLOWER_PATCH,
    HOUSE,
    IRON_GATE,
    ITEM_PICKUP,
    LADDER,
    LAMP,
    LAMPPOST,
    LEAF_SHRUB,
    LESSER_GUARD,
    LIGHT_POST,
    LILAC_BUSH,
    LILY_PAD,
    LILY_PAD_CLUSTER,
    LOW_GUARD,
    MAGIC_GATE,
    MOOD_LIGHT,
    MUSHROOM,
    NPC,
    OAK_TREE,
    RIVER_LOG,
    ROSE_BUSH,
    SCULPTURE_1,
    SHRUB,
    SIGNPOST,
    SMALL_BIRD,
    SMALL_STONE_BRIDGE,
    STARFLOWER,
    STONE_PATH_NODE,
    STONE_WALL,
    SUNBEAM,
    TALL_GRASS,
    TALL_WEEDS,
    TULIP_PLANT,
    VANTAGE_SPOT,
    WATER_FLOW_NODE,
    WATER_WHEEL,
    WILLOW_TREE,
    WIND_CHIMES,
    WOODEN_BRIDGE,
    WOODEN_FENCE,
    WOODEN_GATE_DOOR
  };

  /**
   * ----------------------------
   * Entity identifier; used for lookup.
   * ----------------------------
   */
  struct EntityRecord {
    EntityType type = UNSPECIFIED;
    int32 id = -1;
  };

  /**
   * ----------------------------
   * Enemy-related structures.
   * ----------------------------
   */
  enum EnemyMood {
    ENEMY_IDLE,
    ENEMY_ENGAGED,
    ENEMY_AGITATED,
    ENEMY_FEARFUL
  };

  struct EnemyState {
    EnemyMood mood = ENEMY_IDLE;
    float health = 100.f;
    float speed = 0.f;
    float last_mood_change_time = 0.f;
    float last_attack_start_time = 0.f;
    float last_attack_action_time = 0.f;
    float last_damage_time = 0.f;
    float last_block_time = 0.f;
    float last_break_time = 0.f;
    float last_death_time = 0.f;
    bool is_counterattacking = false;
  };

  /**
   * ----------------------------
   * The primary game entity data structure.
   * @todo use a bitfield for flags
   * @todo check size in bytes and optimize for space where possible
   * ----------------------------
   */
  struct GameEntity : EntityRecord {
    // Customizable properties
    tVec3f position;
    tVec3f scale;
    tVec3f tint; // @todo make editable
    Quaternion orientation = Quaternion(1.f, 0, 0, 0); // @todo rename rotation
    float astro_start_time = 0.f;
    float astro_end_time = 0.f;

    // Visible properties of the entity, e.g. for collisions
    //
    // @todo entities may have more complex structures with
    // multiple mesh elements comprising them, all of which
    // can change with time. It may be worthwhile to have
    // another method in entity behaviors which updates or
    // saves hitboxes with respect to entity time evolution.
    //
    // Especially if we check entity proximity to the player
    // and only generate hitboxes for entities nearby, the
    // number of actual collision bounds generated + checked
    // per frame can be very low.
    tVec3f visible_scale;
    tVec3f visible_position;
    Quaternion visible_rotation;

    // For enemy entities
    EnemyState enemy_state;

    // For reversing movement in time
    std::vector<tVec3f> recent_positions;
    float last_recent_position_record_time = 0.f;
    float last_recent_position_reverse_time = 0.f;

    // For entities which accumulate values over time (rotation for wheels, etc.)
    float accumulation_value = 0.f;

    // Used for item pickup entities, or entities which can
    // impart items to the player (e.g. guards carrying things).
    //
    // @todo rename this
    std::string item_pickup_name = "";

    // For entities with associated or dependent behaviors
    std::string unique_name = "";
    std::string associated_entity_name = "";
    // Cached at start time using the associated entity name
    EntityRecord associated_entity_record;

    // For entities which spawn light sources
    int32 light_id = -1;

    // For interactible entities (gates, doors, light posts etc.)
    float game_activation_time = -1.f;
    float astro_activation_time = 0.f;
    bool did_activate = false;
    bool can_activate = true;

    // A special flag for entities which are inert or reduced in capability by default.
    // The particular meaning of this varies between entities:
    //
    // Lampposts: requires a wand action to turn on
    //
    // Light pillars: requires that their associated light pillar is astro-synced
    // before responding to them
    //
    // Sculpture_1s: requires activating to form a chain for activating distant entities
    bool requires_action = false;

    // For responder light pillars which have become "synced" with their associated entity
    // @todo rename did_take_action
    bool is_astro_synced = false;
  };

  /**
   * ----------------------------
   * Mesh IDs to use for different entity objects + placeholders.
   * ----------------------------
   */
  struct EntityMeshIds {
    uint16

      // ALTAR
      altar_placeholder,
      altar_base,
      altar_statue,

      // AREA_CHANGE
      area_change_placeholder,

      // BANDIT
      bandit_placeholder,
      bandit,

      // BELLFLOWER
      bellflower_placeholder,
      bellflower_stems,
      bellflower_petals,

      // BIRCH_TREE
      birch_tree_placeholder,
      birch_tree_trunk,
      birch_tree_roots,
      birch_tree_leaves,

      // BIRD_GATE
      bird_gate_placeholder,
      bird_gate,

      // BIRD_SPAWN
      bird_spawn_placeholder,

      // CAMERA_CONTROLLER
      camera_controller_placeholder,

      // CASTLE_RAMPART
      castle_rampart_placeholder,
      castle_rampart,

      // CASTLE_STEEPLE
      castle_steeple_placeholder,
      castle_steeple,

      // CASTLE_TOWER
      castle_tower_placeholder,
      castle_tower,

      // CHESTNUT_TREE
      chestnut_tree_placeholder,
      chestnut_tree_trunk,
      chestnut_tree_leaves,

      // DIRT_PATH_NODE
      dirt_path_node_placeholder,

      // DUCK_SPAWN
      duck_spawn_placeholder,

      // EVENT_TRIGGER
      event_trigger_placeholder,

      // FAERIE
      faerie_placeholder,
      faerie_left_wing,
      faerie_right_wing,

      // FLAG
      flag_placeholder,
      flag_pole,
      flag_banner,

      // FLOWER_BUSH
      flower_bush_placeholder,
      flower_bush_leaves,

      // FOG_SPAWN
      fog_spawn_placeholder,

      // GATE
      gate_placeholder,
      gate_body,
      gate_left_door,
      gate_right_door,
      gate_lock,

      // GLOW_FLOWER
      glow_flower_placeholder,
      glow_flower_petals,

      // GROUND_FLOWER_PATCH
      ground_flower_patch_placeholder,
      ground_flower_patch,

      // HOUSE
      house_placeholder,
      house_body,
      house_frame,
      house_door,
      house_window_panes,
      house_roof,
      house_tiles,
      house_chimney,

      // IRON_GATE
      iron_gate_placeholder,
      iron_gate_side,
      iron_gate_wall,

      // ITEM_PICKUP
      item_pickup_placeholder,
      item_pickup,

      // LADDER
      ladder_placeholder,
      ladder,

      // LAMP
      lamp_placeholder,
      lamp_frame,
      lamp_light,

      // LAMPPOST
      lamppost_placeholder,
      lamppost_stand,
      lamppost_frame,
      lamppost_lamp,

      // LEAF_SHRUB
      leaf_shrub_placeholder,
      leaf_shrub_plant,

      // LESSER_GUARD
      lesser_guard_placeholder,
      lesser_guard_shield,
      lesser_guard_sword,

      // LIGHT_POST
      light_post_placeholder,
      light_post_pillar,
      light_post_lamp,

      // LILAC_BUSH
      lilac_placeholder,
      lilac_leaves,
      lilac_flower,

      // LILY_PAD
      lily_pad_placeholder,
      lily_pad,

      // LILY_PAD_CLUSTER
      lily_pad_cluster_placeholder,
      lily_pad_cluster,
      lily_pad_flower,

      // LOW_GUARD
      low_guard_placeholder,
      low_guard_body,
      low_guard_shield,
      low_guard_spear,

      // MAGIC_GATE
      magic_gate_placeholder,
      magic_gate,
      magic_gate_barrier,

      // MOOD_LIGHT
      mood_light_placeholder,

      // MUSHROOM
      mushroom_placeholder,
      mushroom_body,
      mushroom_spots,

      // NPC
      npc_placeholder,
      npc,

      // OAK_TREE
      oak_tree_placeholder,
      oak_tree_roots,
      oak_tree_trunk,
      oak_tree_branches,
      oak_tree_leaves,
      oak_tree_leaves_facade,

      // RIVER_LOG
      river_log_placeholder,
      river_log,

      // ROSE_BUSH
      rose_bush_placeholder,
      rose_bush_leaves,
      rose_bush_flower,

      // SCULPTURE_1
      sculpture_1_placeholder,
      sculpture_1_stand,
      sculpture_1_wheel,

      // SHRUB
      shrub_placeholder,
      shrub_bottom,
      shrub_middle,
      shrub_top,

      // SIGNPOST
      signpost_placeholder,
      signpost,

      // SMALL_BIRD
      small_bird_placeholder,
      small_bird_body,
      small_bird_head,
      small_bird_wings,
      small_bird_left_wing,
      small_bird_right_wing,

      // SMALL_STONE_BRIDGE
      small_stone_bridge_placeholder,
      small_stone_bridge_columns,
      small_stone_bridge_base,

      // STARFLOWER
      starflower_placeholder,
      starflower_leaves,
      starflower_petals,

      // STONE_PATH_NODE
      stone_path_node_placeholder,

      // STONE_WALL
      stone_wall_placeholder,
      stone_wall,

      // SUNBEAM
      sunbeam_placeholder,
      sunbeam,

      // TALL_GRASS
      tall_grass_placeholder,
      tall_grass,

      // TALL_WEEDS
      tall_weeds_placeholder,
      tall_weeds,

      // TULIP_PLANT
      tulip_plant_placeholder,
      tulip_plant_leaves,
      tulip_plant_stalk,
      tulip_plant_bulb,

      // VANTAGE_SPOT
      vantage_spot_placeholder,

      // WATER_FLOW_NODE
      water_flow_node_placeholder,

      // WATER_WHEEL
      water_wheel_placeholder,
      water_wheel,
      water_wheel_platform,

      // WILLOW_TREE
      willow_tree_placeholder,
      willow_tree_trunk,
      willow_tree_branches,
      willow_tree_leaves,

      // WIND_CHIMES
      wind_chimes_placeholder,
      wind_chimes_stand,
      wind_chimes_pivot,
      wind_chimes_hook,
      wind_chimes_hook_2,

      // WOODEN_BRIDGE
      wooden_bridge_placeholder,
      wooden_bridge_platform,

      // WOODEN_FENCE
      wooden_fence_placeholder,
      wooden_fence_post,
      wooden_fence_beam,

      // WOODEN_GATE_DOOR
      wooden_gate_door_placeholder,
      wooden_gate_door
    ;
  };

  /**
   * ----------------------------
   * Supplies a series of containers for different entity types.
   * ----------------------------
   */
  typedef std::vector<GameEntity> EntityList;

  struct EntityContainers {
    EntityList altars;
    EntityList area_changes;
    EntityList bandits;
    EntityList bellflowers;
    EntityList birch_trees;
    EntityList bird_gates;
    EntityList bird_spawns;
    EntityList camera_controllers;
    EntityList castle_ramparts;
    EntityList castle_steeples;
    EntityList castle_towers;
    EntityList chestnut_trees;
    EntityList dirt_path_nodes;
    EntityList duck_spawns;
    EntityList event_triggers;
    EntityList faeries;
    EntityList flags;
    EntityList flower_bushes;
    EntityList fog_spawns;
    EntityList gates;
    EntityList glow_flowers;
    EntityList ground_flower_patches;
    EntityList houses;
    EntityList iron_gates;
    EntityList item_pickups;
    EntityList ladders;
    EntityList lamps;
    EntityList lampposts;
    EntityList leaf_shrubs;
    EntityList lesser_guards;
    EntityList light_posts;
    EntityList lilac_bushes;
    EntityList lily_pads;
    EntityList lily_pad_clusters;
    EntityList low_guards;
    EntityList magic_gates;
    EntityList mood_lights;
    EntityList mushrooms;
    EntityList npcs;
    EntityList oak_trees;
    EntityList river_logs;
    EntityList rose_bushes;
    EntityList sculpture_1s;
    EntityList shrubs;
    EntityList signposts;
    EntityList small_birds;
    EntityList small_stone_bridges;
    EntityList starflowers;
    EntityList stone_path_nodes;
    EntityList stone_walls;
    EntityList sunbeams;
    EntityList tall_grasses;
    EntityList tall_weeds;
    EntityList tulip_plants;
    EntityList vantage_spots;
    EntityList water_flow_nodes;
    EntityList water_wheels;
    EntityList willow_trees;
    EntityList wind_chimes;
    EntityList wooden_bridges;
    EntityList wooden_fences;
    EntityList wooden_gate_doors;
  };

  /**
   * ----------------------------
   * Default properties for entities.
   * ----------------------------
   */
  struct EntityDefaults {
    std::string name;
    tVec3f scale;
    tVec3f orientation;
    tVec3f tint;
  };

  /**
   * ----------------------------
   * Defines default properties for different entities.
   * @todo add serialization ID
   * ----------------------------
   */
  static std::map<EntityType, EntityDefaults> entity_defaults_map = {
    { ALTAR, {
      .name = "Altar",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(0.6f)
    } },

    { AREA_CHANGE, {
      .name = "Area Change",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.5f, 1.f)
    } },

    { BANDIT, {
      .name = "Bandit",
      .scale = tVec3f(600.f, 1500.f, 600.f),
      .tint = tVec3f(0.2f)
    } },

    { BELLFLOWER, {
      .name = "Bellflower",
      .scale = tVec3f(1200.f),
      .tint = tVec3f(1.f, 0.6f, 0.5f)
    } },

    { BIRCH_TREE, {
      .name = "Birch Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.7f, 0.6f, 0.6f)
    } },

    { BIRD_GATE, {
      .name = "Bird Gate",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(1.f)
    } },

    { BIRD_SPAWN, {
      .name = "Bird Spawn",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.5f)
    } },

    { CAMERA_CONTROLLER, {
      .name = "Camera Controller",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.2f)
    } },

    { CASTLE_RAMPART, {
      .name = "Castle Rampart",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.6f)
    } },

    { CASTLE_STEEPLE, {
      .name = "Castle Steeple",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.6f)
    } },

    { CASTLE_TOWER, {
      .name = "Castle Tower",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.6f)
    } },

    { CHESTNUT_TREE, {
      .name = "Chestnut Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.3f, 0.15f, 0.1f)
    } },

    { DIRT_PATH_NODE, {
      .name = "Dirt Path Node",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(1.f, 0.4f, 0.1f)
    } },

    { DUCK_SPAWN, {
      .name = "Duck Spawn",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.7f, 0.f)
    } },

    { EVENT_TRIGGER, {
      .name = "Event Trigger",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.2f, 0.3f, 1.f)
    } },

    { FAERIE, {
      .name = "Faerie",
      .scale = tVec3f(750.f),
      .tint = tVec3f(1.f)
    } },

    { FLAG, {
      .name = "Flag",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f)
    } },

    { FLOWER_BUSH, {
      .name = "Flower Bush",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.2f, 0.6f, 0.3f)
    } },

    { FOG_SPAWN, {
      .name = "Fog Spawn",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.8f)
    } },

    { GATE, {
      .name = "Gate",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.4f)
    } },

    { GLOW_FLOWER, {
      .name = "Glow Flower",
      .scale = tVec3f(600.f),
      .tint = tVec3f(0.2f, 0.4f, 1.f)
    } },

    { GROUND_FLOWER_PATCH, {
      .name = "Ground Flower Patch",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f)
    } },

    { HOUSE, {
      .name = "House",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(1.f)
    } },

    { IRON_GATE, {
      .name = "Iron Gate",
      .scale = tVec3f(5000.f),
      .tint = tVec3f(1.f)
    } },

    { ITEM_PICKUP, {
      .name = "Item Pickup",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.3f, 0.6f, 1.f)
    } },

    { LADDER, {
      .name = "Ladder",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.6f)
    } },

    { LAMP, {
      .name = "Lamp",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(1.f, 0.6f, 0.4f)
    } },

    { LAMPPOST, {
      .name = "Lamppost",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(1.f, 0.6f, 0.4f)
    } },

    { LEAF_SHRUB, {
      .name = "Leaf Shrub",
      .scale = tVec3f(800.f),
      .tint = tVec3f(0.07f, 0.14f, 0.07f)
    } },

    { LESSER_GUARD, {
      .name = "Lesser Guard",
      .scale = tVec3f(600.f, 1500.f, 600.f),
      .tint = tVec3f(1.f, 0.8f, 0.2f)
    } },

    { LIGHT_POST, {
      .name = "Light Post",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.4f)
    } },

    { LILAC_BUSH, {
      .name = "Lilac Bush",
      .scale = tVec3f(800.f),
      .tint = tVec3f(1.f, 0.4f, 1.f)
    } },

    { LILY_PAD, {
      .name = "Lily Pad",
      .scale = tVec3f(500.f),
      .tint = tVec3f(0.3f, 0.7f, 0.1f)
    } },

    { LILY_PAD_CLUSTER, {
      .name = "Lily Pad Cluster",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.3f, 0.7f, 0.1f)
    } },

    { LOW_GUARD, {
      .name = "Low Guard",
      .scale = tVec3f(600.f, 1500.f, 600.f),
      .tint = tVec3f(0.7f)
    } },

    { MAGIC_GATE, {
      .name = "Magic Gate",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.6f)
    } },

    { MOOD_LIGHT, {
      .name = "Mood Light",
      .scale = tVec3f(500.f),
      .tint = tVec3f(1.f)
    } },

    { MUSHROOM, {
      .name = "Mushroom",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.2f, 1.f, 0.4f)
    } },

    { NPC, {
      .name = "Npc",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(1.f)
    } },

    { OAK_TREE, {
      .name = "Oak Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.15f, 0.3f, 0.1f)
    } },

    { RIVER_LOG, {
      .name = "River Log",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(1.f, 0.5f, 0.1f)
    } },

    { ROSE_BUSH, {
      .name = "Rose Bush",
      .scale = tVec3f(800.f),
      .tint = tVec3f(1.f, 0.2f, 0.1f)
    } },

    { SCULPTURE_1, {
      .name = "Sculpture 1",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(1.f)
    } },

    { SHRUB, {
      .name = "Shrub",
      .scale = tVec3f(800.f),
      .tint = tVec3f(0.1f, 0.3f, 0.1f)
    } },

    { SIGNPOST, {
      .name = "Signpost",
      .scale = tVec3f(1250.f),
      .tint = tVec3f(1.f, 0.8f, 0.4f)
    } },

    { SMALL_BIRD, {
      .name = "Small Bird",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.6f)
    } },

    { SMALL_STONE_BRIDGE, {
      .name = "Small Stone Bridge",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.6f)
    } },

    { STARFLOWER, {
      .name = "Starflower",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(1.f, 0.6f, 0.5f)
    } },

    { STONE_PATH_NODE, {
      .name = "Stone Path Node",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.5f)
    } },

    { STONE_WALL, {
      .name = "Stone Wall",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(0.5f)
    } },

    { SUNBEAM, {
      .name = "Sunbeam",
      .scale = tVec3f(3000.f, 8000.f, 3000.f),
      .tint = tVec3f(1.f, 0.7f, 0.2f)
    } },

    { TALL_GRASS, {
      .name = "Tall Grass",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.1f, 0.3f, 0.1f)
    } },

    { TALL_WEEDS, {
      .name = "Tall Weeds",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(0.1f, 0.3f, 0.1f)
    } },

    { TULIP_PLANT, {
      .name = "Tulip Plant",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.2f, 0.6f, 0.3f)
    } },

    { VANTAGE_SPOT, {
      .name = "Vantage Spot",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(1.f, 0.1f, 0.2f)
    } },

    { WATER_FLOW_NODE, {
      .name = "Water Flow Node",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.1f, 0.2f, 1.f)
    } },

    { WATER_WHEEL, {
      .name = "Water Wheel",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(1.f)
    } },

    { WILLOW_TREE, {
      .name = "Willow Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.6f, 0.3f)
    } },

    { WIND_CHIMES, {
      .name = "Wind Chimes",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(1.f)
    } },

    { WOODEN_BRIDGE, {
      .name = "Wooden Bridge",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.6f)
    } },

    { WOODEN_FENCE, {
      .name = "Wooden Fence",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.8f, 0.4f)
    } },

    { WOODEN_GATE_DOOR, {
      .name = "Wooden Gate Door",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(1.f, 0.6f, 0.2f)
    } },
  };

  /**
   * ----------------------------
   * Entity utilities.
   * ----------------------------
   */
  // @todo do we need this?
  static inline bool IsSameEntity(const GameEntity& entity, const EntityRecord& record) {
    return entity.type == record.type && entity.id == record.id;
  }

  static inline bool IsSameEntity(const EntityRecord& record_a, const EntityRecord& record_b) {
    return record_a.type == record_b.type && record_a.id == record_b.id;
  }

  // @todo CreateRecord()
  static inline EntityRecord GetRecord(const GameEntity& entity) {
    return {
      .type = entity.type,
      .id = entity.id
    };
  }
}