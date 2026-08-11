#pragma once

#include "engine/tachyon.h"

#include "metro/collision.h"

#define for_static_entity_containers()\
  for (auto* entities : {\
      &state.entities.platforms,\
      &state.entities.ramps,\
      &state.entities.walkway_segments\
  })

#define for_entities()\
  for (auto& entity : *entities)

namespace metro {
  enum EntityCategory {
    NOT_AN_ENTITY = -1,
    BICYCLE,
    STATIC_ENTITY,
    INTERACTIVE_ENTITY
  };

  enum EntityType {
    UNSPECIFIED = -1,
    COMMON_BIKE,
    PLATFORM,
    RAMP,
    WALKWAY_SEGMENT
  };

  struct BaseEntity {
    EntityType type = UNSPECIFIED;
    int32 id = -1;

    bool needs_init = true;
    bool needs_update = true;
    bool needs_deletion = false;
  };

  struct StaticEntity : BaseEntity {
    tVec3f position;
    Quaternion rotation = Quaternion(1.f, 0, 0, 0);
    tVec3f scale;
    tColor color;

    bool active = true;

    // @todo @optimize come up with a non-heap-allocated solution for collision planes,
    // given that we don't know how many we'll need per entity type
    std::vector<CollisionPlane> collision_planes;
  };

  struct InteractiveEntity : BaseEntity {
    // @todo
  };

  struct Entities {
    // Statics
    std::vector<StaticEntity> platforms;
    std::vector<StaticEntity> ramps;
    std::vector<StaticEntity> walkway_segments;

    // Interactives
    std::vector<InteractiveEntity> vending_machines;
  };

  int32 CreateUniqueId();
  EntityCategory GetEntityCategory(EntityType entity_type);
  StaticEntity& CreateStaticEntity(Entities& entities, EntityType type);
  InteractiveEntity& CreateInteractiveEntity(Entities& entities, EntityType type);
  bool IsSameEntity(const BaseEntity& a, const BaseEntity& b);
}