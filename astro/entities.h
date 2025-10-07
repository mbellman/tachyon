#pragma once

#include <map>

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
    DIRT_PATH
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
    DIRT_PATH
  };

  /**
   * ----------------------------
   * Entity identifier; used for lookup.
   * ----------------------------
   */
  struct EntityRecord {
    int32 id = -1;
    EntityType type = UNSPECIFIED;
  };

  /**
   * ----------------------------
   * Enemy-related structures.
   * ----------------------------
   */
  enum EnemyMood {
    IDLE,
    NOTICED,
    AGITATED,
    FEARFUL
  };

  struct EnemyState {
    EnemyMood mood = IDLE;
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
    Quaternion orientation = Quaternion(1.f, 0, 0, 0);
    float astro_start_time = 0.f;
    float astro_end_time = 0.f;

    // Apparent size of the entity, e.g. for collisions
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

    // Only used for certain game entities
    EnemyState enemy_state;
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
      oak_tree_trunk,

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
      low_guard,
      
      // BANDIT
      bandit_placeholder,
      bandit,

      // STONE_WALL,
      stone_wall_placeholder,
      stone_wall,

      // DIRT_PATH,
      dirt_path_placeholder,
      dirt_path

      ;
  };

  /**
   * ----------------------------
   * Supplies a series of containers for different entity types.
   * ----------------------------
   */
  struct EntityContainers {
    std::vector<GameEntity> dirt_paths;
    std::vector<GameEntity> shrubs;
    std::vector<GameEntity> oak_trees;
    std::vector<GameEntity> willow_trees;
    std::vector<GameEntity> small_stone_bridges;
    std::vector<GameEntity> stone_walls;
    std::vector<GameEntity> wooden_gate_doors;
    std::vector<GameEntity> river_logs;
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
    { DIRT_PATH, {
      .name = "Dirt Path",
      .scale = tVec3f(2000.f, 1.f, 2000.f),
      .tint = tVec3f(0.7f, 0.3f, 0.1f)
    } },

    { SHRUB, {
      .name = "Shrub",
      .scale = tVec3f(500.f),
      .tint = tVec3f(0.2f, 0.8f, 0.5f)
    } },

    { OAK_TREE, {
      .name = "Oak Tree",
      .scale = tVec3f(500.f, 2000.f, 500.f ),
      .tint = tVec3f(1.f, 0.6f, 0.3f)
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

    { WOODEN_GATE_DOOR, {
      .name = "Wooden Gate Door",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(1.f, 0.6f, 0.2f)
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
      .scale = tVec3f(600.f, 1500.f, 600.f ),
      .tint = tVec3f(0.2f)
    } }
  };
}