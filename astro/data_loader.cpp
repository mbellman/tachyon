#include <map>
#include <string>
#include <vector>

#include "astro/data_loader.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"

using namespace astro;

static std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
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

  return values;
}

uint16 DataLoader::MeshIndexToId(State& state, uint16 mesh_index) {
  auto& meshes = state.meshes;

  const static std::map<uint16, uint16> mesh_map = {
    { meshes.rock_1, 1 },
    { meshes.ground_1, 2 },
    { meshes.flat_ground, 3 }
  };

  return mesh_map.at(mesh_index);
}

uint16 DataLoader::MeshIdToIndex(State& state, uint16 mesh_id) {
  auto& meshes = state.meshes;

  const static std::map<uint16, uint16> mesh_map = {
    { 1, meshes.rock_1 },
    { 2, meshes.ground_1 },
    { 3, meshes.flat_ground }
  };

  return mesh_map.at(mesh_id);
}

// @todo This is very slow as it allocates strings and vectors for every single line
// in the data file. It should be considered an interim solution for reading level data
// until later in development, when we have some kind of binary data or intermediate
// representation which can be rapidly parsed and converted into entities/objects.
void DataLoader::LoadLevelData(Tachyon* tachyon, State& state) {
  auto start = Tachyon_GetMicroseconds();
  auto level_data = Tachyon_GetFileContents("./astro/level_data/level.txt");
  auto lines = SplitString(level_data, "\n");

  #define parsef(i) stof(parts[i])
  #define parse_vec3f(i1, i2, i3) tVec3f(parsef(i1), parsef(i2), parsef(i3))
  #define parse_quaternion(i1, i2, i3, i4) Quaternion(parsef(i1), parsef(i2), parsef(i3), parsef(i4))

  for (auto& line : lines) {
    if (line[0] == '@') {
      // Entity
      auto parts = SplitString(line, ",");
      // @todo use constant IDs which map to entity types
      EntityType entity_type = (EntityType)stoi(parts[0].substr(1));
      GameEntity entity = EntityManager::CreateNewEntity(state, entity_type);

      entity.position = parse_vec3f(1, 2, 3);
      entity.scale = parse_vec3f(4, 5, 6);
      entity.orientation = parse_quaternion(7, 8, 9, 10);
      entity.tint = parse_vec3f(11, 12, 13);
      entity.astro_start_time = parsef(14);
      entity.astro_end_time = parsef(15);

      // Set base visible position
      entity.visible_position = entity.position;

      EntityManager::SaveNewEntity(state, entity);

      auto& mesh_ids = EntityDispatcher::GetMeshes(state, entity.type);

      for (auto mesh_id : mesh_ids) {
        create(mesh_id);
      }
    }
    else if (line[0] == '$') {
      // Object
      auto parts = SplitString(line, ",");
      uint16 mesh_id = stoi(parts[0].substr(1));
      auto mesh_index = MeshIdToIndex(state, mesh_id);
      auto& object = create(mesh_index);

      object.position = parse_vec3f(1, 2, 3);
      object.scale = parse_vec3f(4, 5, 6);
      object.rotation = parse_quaternion(7, 8, 9, 10);
      object.color.rgba = stoi(parts[11]);

      // @temporary
      // @todo set mesh material properties
      if (mesh_index == state.meshes.ground_1) {
        object.material = tVec4f(0.9f, 0, 0, 0.1f);
      }

      commit(object);
    }
  }

  #undef parsef
  #undef parse_vec3f
  #undef parse_quaternion

  auto total_time = Tachyon_GetMicroseconds() - start;

  add_console_message("Loaded level data in " + std::to_string(total_time) + "us", tVec3f(1.f));
}