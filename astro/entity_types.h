#pragma once

#include <map>

#include "engine/tachyon.h"

namespace astro {
  enum EntityType {
    UNSPECIFIED = -1,
    SHRUB,
    OAK_TREE,
    WILLOW_TREE,
    SMALL_STONE_BRIDGE,
    WOODEN_GATE_DOOR
  };

  static std::vector<EntityType> entity_types = {
    SHRUB,
    OAK_TREE,
    WILLOW_TREE,
    SMALL_STONE_BRIDGE,
    WOODEN_GATE_DOOR
  };

  struct EntityRecord {
    int32 id = -1;
    EntityType type = UNSPECIFIED;
  };

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
  };

  struct EntityDefaults {
    std::string name;
    tVec3f scale;
    tVec3f orientation;
    tVec3f tint;
  };

  static std::map<EntityType, EntityDefaults> entity_defaults_map = {
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

    { WOODEN_GATE_DOOR, {
      .name = "Wooden Gate Door",
      .scale = tVec3f(3500.f),
      .tint = tVec3f(1.f, 0.6f, 0.2f)
    } }
  };
}