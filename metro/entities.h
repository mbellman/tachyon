#pragma once

#include "engine/tachyon.h"

#define for_static_entity_containers()\
  for (auto* entities : {\
      &state.entities.platforms,\
      &state.entities.ramps,\
      &state.entities.walkway_segments\
  })

#define for_entities()\
  for (auto& entity : *entities)

namespace metro {
  // @todo move to engine
  struct CollisionPlane {
    // Plane corners
    tVec3f p1, p2, p3, p4;
    // Plane tangents
    tVec3f t1, t2, t3, t4;
    tVec3f normal;
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
    tVec3f color;

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

  static StaticEntity& CreateStaticEntity(Entities& entities, EntityType type) {
    StaticEntity entity;
    entity.type = type;

    switch (type) {
      case PLATFORM:
        entities.platforms.push_back(entity);

        return entities.platforms.back();
      case RAMP:
        entities.ramps.push_back(entity);

        return entities.ramps.back();
      case WALKWAY_SEGMENT:
        entities.walkway_segments.push_back(entity);

        return entities.walkway_segments.back();
      default:
        console_error("CreateStaticEntity(): Invalid entity type");
        exit(0);
    }
  }

  static InteractiveEntity& CreateInteractiveEntity(Entities& entities, EntityType type) {
    // @todo
  }
}