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
    LILAC_BUSH
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
    LILAC_BUSH
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
  };

  /**
   * ----------------------------
   * The primary game entity data structure.
   * ----------------------------
   */
  struct GameEntity : EntityRecord {
    // Customizable properties
    tVec3f position;
    tVec3f scale;
    tVec3f tint;
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

    // Only used for enemy entities
    EnemyState enemy_state;

    // Only used for item pickup entities. We could use an internal ID,
    // but those IDs might not remain constant as item types are introduced
    // or eliminated, and using a readable item name is more ergonomic,
    // particularly for editing. We have to look up the actual item details
    // in a hash map when we create the object for it and pick it up, but
    // we can eat the cost for that.
    std::string item_pickup_name = "";

    // For entities which spawn light sources
    int32 light_id = -1;

    // For gates, doors etc.
    float game_open_time = -1.f;
    float astro_open_time = 0.f;
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
      shrub_branches,

      // OAK_TREE
      oak_tree_placeholder,
      oak_tree_roots,
      oak_tree_trunk,
      oak_tree_branches,

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

      // BANDIT
      bandit_placeholder,
      bandit,

      // STONE_WALL
      stone_wall_placeholder,
      stone_wall,

      // DIRT_PATH
      // @unused
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
      lilac_flower

      ;
  };

  /**
   * ----------------------------
   * Supplies a series of containers for different entity types.
   * ----------------------------
   */
  struct EntityContainers {
    std::vector<GameEntity> item_pickups;
    std::vector<GameEntity> dirt_paths; // @unused
    std::vector<GameEntity> dirt_path_nodes;
    std::vector<GameEntity> shrubs;
    std::vector<GameEntity> flower_bushes;
    std::vector<GameEntity> lilac_bushes;
    std::vector<GameEntity> glow_flowers;
    std::vector<GameEntity> oak_trees;
    std::vector<GameEntity> willow_trees;
    std::vector<GameEntity> small_stone_bridges;
    std::vector<GameEntity> stone_walls;
    std::vector<GameEntity> wooden_gate_doors;
    std::vector<GameEntity> river_logs;
    std::vector<GameEntity> low_guards;
    std::vector<GameEntity> bandits;
    std::vector<GameEntity> houses;
    std::vector<GameEntity> gates;
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
    { ITEM_PICKUP, {
      .name = "Item Pickup",
      .scale = tVec3f(1000.f),
      .tint = tVec3f(0.3f, 0.6f, 1.f)
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
      .scale = tVec3f(500.f),
      .tint = tVec3f(0.2f, 0.8f, 0.5f)
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

    { GLOW_FLOWER, {
      .name = "Glow Flower",
      .scale = tVec3f(600.f),
      .tint = tVec3f(0.2f, 0.4f, 1.f)
    } },

    { OAK_TREE, {
      .name = "Oak Tree",
      .scale = tVec3f(2000.f),
      .tint = tVec3f(1.f, 0.4f, 0.2f)
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

    { WOODEN_GATE_DOOR, {
      .name = "Wooden Gate Door",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(1.f, 0.6f, 0.2f)
    } },

    { HOUSE, {
      .name = "House",
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
      .scale = tVec3f(1500.f),
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