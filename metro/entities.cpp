#include "metro/entities.h"

using namespace metro;

static uint32 running_unique_id = 0;

int32 metro::CreateUniqueId() {
  running_unique_id++;

  uint32 id = running_unique_id;

  id *= 1103515245u;
  id &= 0x7fffffffu;

  return (int32) id;
}

EntityCategory metro::GetEntityCategory(EntityType entity_type) {
  switch (entity_type) {
    case COMMON_BIKE:
      return BICYCLE;
    case PLATFORM:
    case RAMP:
    case WALKWAY_SEGMENT:
      return STATIC_ENTITY;
    default:
      return NOT_AN_ENTITY;
  }
}

StaticEntity& metro::CreateStaticEntity(Entities& entities, EntityType type) {
  StaticEntity entity;
  entity.type = type;
  entity.id = CreateUniqueId();

  // @todo refactor this
  switch (type) {
    case PLATFORM:
      entities.platforms.push_back(entity);

      return entities.platforms.back();
    case RAMP:
      entities.ramps.push_back(entity);

      return entities.ramps.back();
    case ROAD_SEGMENT:
      entities.road_segments.push_back(entity);

      return entities.road_segments.back();
    case WALKWAY_SEGMENT:
      entities.walkway_segments.push_back(entity);

      return entities.walkway_segments.back();
    default:
      console_error("CreateStaticEntity(): Invalid entity type");
      exit(0);
  }
}

// @incomplete
InteractiveEntity& metro::CreateInteractiveEntity(Entities& entities, EntityType type) {
  InteractiveEntity entity;
  entity.type = type;
  entity.id = CreateUniqueId();

  // @todo add the entity to the appropriate array! right now this is undefined behavior
  return entity;
}

bool metro::IsSameEntity(const BaseEntity& a, const BaseEntity& b) {
  return a.type == b.type && a.id == b.id;
}