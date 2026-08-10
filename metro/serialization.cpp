#include <format>

#include "metro/serialization.h"

using namespace metro;

// @todo move to engine?
static std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
  // @allocation
  std::vector<std::string> values;
  uint32 offset = 0;
  uint32 found = 0;

  // Add each delimited string segment to the list
  while ((found = str.find(delimiter, offset)) != std::string::npos) {
    values.push_back(str.substr(offset, found - offset));

    offset = found + delimiter.size();
  }

  // Include the remaining string segment after the final delimiter
  values.push_back(str.substr(offset, str.size() - offset));

  // @allocation
  return values;
}

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

static Bicycle DeserializeBike(EntityType type, const std::string& bike_data) {
  auto parts = SplitString(bike_data, ",");
}

// @todo combine this and below into a map or tuple array
static EntityType StringToEntityType(const std::string& entity_name) {
  if (entity_name == "Common Bike")     return COMMON_BIKE;
  if (entity_name == "Platform")        return PLATFORM;
  if (entity_name == "Ramp")            return RAMP;
  if (entity_name == "Walkway Segment") return WALKWAY_SEGMENT;

  return UNSPECIFIED;
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

void Serialization::LoadWorldData(State& state, const std::string& world_level_name) {
  state.world_level_name = world_level_name;

  auto level_data_file = "./metro/levels/" + world_level_name;
  auto data = Tachyon_GetFileContents(level_data_file.c_str());
  auto lines = SplitString(data, "\n");

  EntityType current_entity_type = UNSPECIFIED;

  for (auto& line : lines) {
    if (line.starts_with("@")) {
      // @temporary
      console_log(line.substr(1));

      current_entity_type = StringToEntityType(line.substr(1));
    } else {
      // @todo
    }
  }
}