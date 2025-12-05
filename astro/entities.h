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
    SHRUB,
    OAK_TREE,
    WILLOW_TREE,
    SMALL_STONE_BRIDGE,
    WOODEN_GATE_DOOR,
    RIVER_LOG,
    LOW_GUARD,
    BANDIT,
    STONE_WALL,
    DIRT_PATH, // @unused
    FLOWER_BUSH,
    ITEM_PICKUP,
    GLOW_FLOWER,
    DIRT_PATH_NODE,
    HOUSE,
    GATE,
    LILAC_BUSH,
    LIGHT_POST,
    WOODEN_FENCE,
    ALTAR,
    MUSHROOM,
    LAMPPOST,
    FOG_SPAWN,
    WIND_CHIMES,
    WATER_WHEEL,
    CHESTNUT_TREE
  };

  /**
   * ----------------------------
   * An iterable list of valid entity types.
   * ----------------------------
   */
  static std::vector<EntityType> entity_types = {
    SHRUB,
    OAK_TREE,
    WILLOW_TREE,
    SMALL_STONE_BRIDGE,
    WOODEN_GATE_DOOR,
    RIVER_LOG,
    LOW_GUARD,
    BANDIT,
    STONE_WALL,
    DIRT_PATH, // @unused
    FLOWER_BUSH,
    ITEM_PICKUP,
    GLOW_FLOWER,
    DIRT_PATH_NODE,
    HOUSE,
    GATE,
    LILAC_BUSH,
    LIGHT_POST,
    WOODEN_FENCE,
    ALTAR,
    MUSHROOM,
    LAMPPOST,
    FOG_SPAWN,
    WIND_CHIMES,
    WATER_WHEEL,
    CHESTNUT_TREE
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
    float last_attack_time = 0.f;
  };

  /**
   * ----------------------------
   * The primary game entity data structure.
   * @todo use a bitfield for flags
   * ----------------------------
   */
  struct GameEntity : EntityRecord {
    // Customizable properties
    tVec3f position;
    tVec3f scale;
    tVec3f tint; // @todo
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

    // For responder light pillars which have become "synced" with their associated entity
    bool is_astro_synced = false;

    // For responder light pillars which require that their associated entity become synced
    // before responding to them
    bool requires_astro_sync = false;
  };

  /**
   * ----------------------------
   * Mesh IDs to use for different entity objects + placeholders.
   * ----------------------------
   */
  struct EntityMeshIds {
    uint16
      // SHRUB
      shrub_placeholder,
      shrub_leaves,

      // OAK_TREE
      oak_tree_placeholder,
      oak_tree_roots,
      oak_tree_trunk,
      oak_tree_branches,
      oak_tree_leaves,

      // WILLOW_TREE
      willow_tree_placeholder,
      willow_tree_trunk,

      // RIVER_LOG
      river_log_placeholder,
      river_log,

      // SMALL_STONE_BRIDGE
      small_stone_bridge_placeholder,
      small_stone_bridge_columns,
      small_stone_bridge_base,

      // WOODEN_GATE_DOOR
      wooden_gate_door_placeholder,
      wooden_gate_door,

      // LOW_GUARD
      low_guard_placeholder,
      low_guard_body,
      low_guard_shield,
      low_guard_spear,

      // BANDIT
      bandit_placeholder,
      bandit,

      // STONE_WALL
      stone_wall_placeholder,
      stone_wall,

      // DIRT_PATH
      // @unused
      // @todo remove
      dirt_path_placeholder,
      dirt_path,

      // FLOWER_BUSH
      flower_bush_placeholder,
      flower_bush_leaves,

      // ITEM_PICKUP
      item_pickup_placeholder,

      // GLOW_FLOWER
      glow_flower_placeholder,
      glow_flower_petals,

      // DIRT_PATH_NODE,
      dirt_path_node_placeholder,

      // HOUSE
      house_placeholder,
      house_body,
      house_frame,
      house_roof,
      house_chimney,

      // GATE
      gate_placeholder,
      gate_body,
      gate_left_door,
      gate_right_door,
      gate_lock,

      // LILAC
      lilac_placeholder,
      lilac_leaves,
      lilac_flower,

      // LIGHT_POST
      light_post_placeholder,
      light_post_pillar,
      light_post_lamp,

      // WOODEN_FENCE
      wooden_fence_placeholder,
      wooden_fence_post,
      wooden_fence_beam,

      // ALTAR
      altar_placeholder,
      altar_base,
      altar_statue,

      // MUSHROOM
      mushroom_placeholder,
      mushroom_body,

      // LAMPPOST,
      lamppost_placeholder,
      lamppost_stand,
      lamppost_frame,
      lamppost_lamp,

      // FOG_SPAWN
      fog_spawn_placeholder,

      // WIND_CHIMES
      wind_chimes_placeholder,
      wind_chimes_stand,

      // WATER_WHEEL
      water_wheel_placeholder,
      water_wheel,
      water_wheel_platform,

      // CHESTNUT_TREE
      chestnut_tree_placeholder,
      chestnut_tree_trunk,
      chestnut_tree_leaves

      ;
  };

  /**
   * ----------------------------
   * Supplies a series of containers for different entity types.
   * ----------------------------
   */
  struct EntityContainers {
    std::vector<GameEntity> wind_chimes;
    std::vector<GameEntity> altars;
    std::vector<GameEntity> item_pickups;
    std::vector<GameEntity> fog_spawns;
    std::vector<GameEntity> dirt_paths; // @unused
    std::vector<GameEntity> dirt_path_nodes;

    std::vector<GameEntity> shrubs;
    std::vector<GameEntity> flower_bushes;
    std::vector<GameEntity> lilac_bushes;
    std::vector<GameEntity> mushrooms;
    std::vector<GameEntity> glow_flowers;
    std::vector<GameEntity> oak_trees;
    std::vector<GameEntity> chestnut_trees;
    std::vector<GameEntity> willow_trees;
    std::vector<GameEntity> river_logs;

    std::vector<GameEntity> small_stone_bridges;
    std::vector<GameEntity> stone_walls;
    std::vector<GameEntity> wooden_gate_doors;
    std::vector<GameEntity> gates;
    std::vector<GameEntity> lampposts;
    std::vector<GameEntity> houses;
    std::vector<GameEntity> light_posts;
    std::vector<GameEntity> wooden_fences;
    std::vector<GameEntity> water_wheels;

    std::vector<GameEntity> low_guards;
    std::vector<GameEntity> bandits;
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
    { WIND_CHIMES, {
      .name = "Wind Chimes",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(1.f)
    } },

    { ITEM_PICKUP, {
      .name = "Item Pickup",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.3f, 0.6f, 1.f)
    } },

    { FOG_SPAWN, {
      .name = "Fog Spawn",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.8f)
    } },

    // @unused
    { DIRT_PATH, {
      .name = "Dirt Path",
      .scale = tVec3f(2000.f, 1.f, 2000.f),
      .tint = tVec3f(0.7f, 0.3f, 0.1f)
    } },

    { DIRT_PATH_NODE, {
      .name = "Dirt Path Node",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(1.f, 0.4f, 0.1f)
    } },

    { SHRUB, {
      .name = "Shrub",
      .scale = tVec3f(800.f),
      .tint = tVec3f(0.1f, 0.3f, 0.1f)
    } },

    { FLOWER_BUSH, {
      .name = "Flower Bush",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.2f, 0.6f, 0.3f)
    } },

    { LILAC_BUSH, {
      .name = "Lilac Bush",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.3f, 0.8f, 0.2f)
    } },

    { MUSHROOM, {
      .name = "Mushroom",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.2f, 1.f, 0.4f)
    } },

    { GLOW_FLOWER, {
      .name = "Glow Flower",
      .scale = tVec3f(600.f),
      .tint = tVec3f(0.2f, 0.4f, 1.f)
    } },

    { OAK_TREE, {
      .name = "Oak Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.15f, 0.3f, 0.1f)
    } },

    { CHESTNUT_TREE, {
      .name = "Chestnut Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(0.3f, 0.15f, 0.1f)
    } },

    // @todo
    { WILLOW_TREE, {
      .name = "Willow Tree",
      .scale = tVec3f(500.f, 2000.f, 500.f ),
      .tint = tVec3f(1.f, 0.6f, 0.3f )
    } },

    { SMALL_STONE_BRIDGE, {
      .name = "Small Stone Bridge",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(0.6f)
    } },

    { STONE_WALL, {
      .name = "Stone Wall",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(0.5f)
    } },

    { GATE, {
      .name = "Gate",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(0.4f)
    } },

    { LIGHT_POST, {
      .name = "Light Pillar",
      .scale = tVec3f(1500.f),
      .tint = tVec3f(0.4f)
    } },

    { ALTAR, {
      .name = "Altar",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(0.6f)
    } },

    { WOODEN_GATE_DOOR, {
      .name = "Wooden Gate Door",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(1.f, 0.6f, 0.2f)
    } },

    { WOODEN_FENCE, {
      .name = "Wooden Fence",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.8f, 0.4f)
    } },

    { LAMPPOST, {
      .name = "Lamppost",
      .scale = tVec3f(2500.f),
      .tint = tVec3f(1.f, 0.6f, 0.4f)
    } },

    { HOUSE, {
      .name = "House",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(1.f)
    } },

    { WATER_WHEEL, {
      .name = "Water Wheel",
      .scale = tVec3f(4000.f),
      .tint = tVec3f(1.f)
    } },

    { RIVER_LOG, {
      .name = "River Log",
      .scale = tVec3f(3000.f),
      .tint = tVec3f(1.f, 0.5f, 0.1f)
    } },

    { LOW_GUARD, {
      .name = "Low Guard",
      .scale = tVec3f(600.f, 1500.f, 600.f ),
      .tint = tVec3f(0.7f)
    } },

    { BANDIT, {
      .name = "Bandit",
      .scale = tVec3f(600.f, 1500.f, 600.f),
      .tint = tVec3f(0.2f)
    } }
  };

  /**
   * ----------------------------
   * Entity utilities.
   * ----------------------------
   */
  // @todo do we need this?
  static inline bool IsSameEntity(GameEntity& entity, EntityRecord& record) {
    return entity.type == record.type && entity.id == record.id;
  }

  static inline bool IsSameEntity(EntityRecord& record_a, EntityRecord& record_b) {
    return record_a.type == record_b.type && record_a.id == record_b.id;
  }

  static inline EntityRecord GetRecord(GameEntity& entity) {
    return {
      .type = entity.type,
      .id = entity.id
    };
  }
}