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
    case ELECTRIC_SCOOTER:
      return SCOOTER;
    case PLATFORM:
    case RAMP:
    case ROAD_SEGMENT:
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

// @todo combine this and below into a map or tuple array
EntityType metro::StringToEntityType(const std::string& entity_name) {
  if (entity_name == "Common Bike")     return COMMON_BIKE;
  if (entity_name == "Platform")        return PLATFORM;
  if (entity_name == "Ramp")            return RAMP;
  if (entity_name == "Road Segment")    return ROAD_SEGMENT;
  if (entity_name == "Walkway Segment") return WALKWAY_SEGMENT;

  return UNSPECIFIED;
}

std::string metro::EntityTypeToString(EntityType type) {
  switch (type) {
    case COMMON_BIKE    : return "Common Bike";
    case PLATFORM       : return "Platform";
    case RAMP           : return "Ramp";
    case ROAD_SEGMENT   : return "Road Segment";
    case WALKWAY_SEGMENT: return "Walkway Segment";
    default:
      return "Entity";
  }
}