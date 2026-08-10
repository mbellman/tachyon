#include <format>

#include "metro/serialization.h"

using namespace metro;

static inline std::string Serialize(float f) {
  return std::format("{:.3f}", f);
}

static inline std::string Serialize(bool value) {
  return value ? "1" : "";
}

static inline std::string Serialize(const tVec3f& vector) {
  return (
    Serialize(vector.x) + "," +
    Serialize(vector.y) + "," +
    Serialize(vector.z)
  );
}

static inline std::string Serialize(const Quaternion& quaternion) {
  return (
    Serialize(quaternion.w) + "," +
    Serialize(quaternion.x) + "," +
    Serialize(quaternion.y) + "," +
    Serialize(quaternion.z)
  );
}

static inline std::string Serialize(const tColor& color) {
  return std::to_string(color.rgba);
}

static std::string Serialize(const StaticEntity& entity) {
  return (
    Serialize(entity.position) + "," +
    Serialize(entity.rotation) + "," +
    Serialize(entity.scale)
  );
}

static void SerializeEntities(std::string& data, const std::string& name, const std::vector<StaticEntity>& entities) {
  data += "@" + name;
  data += "\n";

  for (auto& entity : entities) {
    data += Serialize(entity);
    data += "\n";
  }
}

static void SerializeBike(std::string& data, const Bicycle& bike) {
  data += "@" + Serialization::EntityTypeToString(bike.type);
  data += "\n";

  data += Serialize(bike.position) + ",";
  data += Serialize(bike.facing_direction) + ",";
  data += Serialize(bike.frame_color) + ",";
  data += Serialize(bike.grips_color) + ",";
  data += Serialize(bike.saddle_color) + ",";
  data += Serialize(bike.wheel_color);
  data += "\n";
}

std::string Serialization::EntityTypeToString(EntityType type) {
  switch (type) {
    case COMMON_BIKE    : return "Common Bike";
    case PLATFORM       : return "Platform";
    case RAMP           : return "Ramp";
    case WALKWAY_SEGMENT: return "Walkway Segment";
    default:
      return "Entity";
  }
}

void Serialization::SaveWorldData(const State& state) {
  std::string data = "";

  // Static entities
  {
    SerializeEntities(data, "Platform", state.entities.platforms);
    SerializeEntities(data, "Ramp", state.entities.ramps);
    SerializeEntities(data, "Walkway Segment", state.entities.walkway_segments);
  }

  // Interactive entities
  {
    // @todo
  }

  // Bikes
  {
    for (auto& bike : state.bicycles) {
      SerializeBike(data, bike);
    }
  }

  Tachyon_WriteFileContents("./metro/levels/" + state.world_level_name, data);

  console_info("Saved world data to " + state.world_level_name);
}