#include <map>
#include <string>
#include <vector>

#include "astro/data_loader.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"

using namespace astro;

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

uint16 DataLoader::MeshIndexToId(State& state, uint16 mesh_index) {
  auto& meshes = state.meshes;

  const static std::map<uint16, uint16> mesh_map = {
    { meshes.rock_1, 1 },
    { meshes.ground_1, 2 },
    { meshes.flat_ground, 3 },
    { meshes.lookout_tower, 4 },
    { meshes.river_edge, 5 },
    { meshes.rock_2, 6 }
  };

  return mesh_map.at(mesh_index);
}

uint16 DataLoader::MeshIdToIndex(State& state, uint16 mesh_id) {
  auto& meshes = state.meshes;

  const static std::map<uint16, uint16> mesh_map = {
    { 1, meshes.rock_1 },
    { 2, meshes.ground_1 },
    { 3, meshes.flat_ground },
    { 4, meshes.lookout_tower },
    { 5, meshes.river_edge },
    { 6, meshes.rock_2 }
  };

  return mesh_map.at(mesh_id);
}

// @todo This is very slow as it allocates strings and vectors for every single line
// in the data file. It should be considered an interim solution for reading level data
// until later in development, when we have some kind of binary data or intermediate
// representation which can be rapidly parsed and converted into entities/objects.
void DataLoader::LoadLevelData(Tachyon* tachyon, State& state) {
  log_time("LoadLevelData()");

  auto level_data = Tachyon_GetFileContents("./astro/level_data/level.txt");
  auto lines = SplitString(level_data, "\n");  // @allocation

  #define parsef(i) stof(parts[i])
  #define parse_bool(i) (i == "1")
  #define parse_vec3f(i1, i2, i3) tVec3f(parsef(i1), parsef(i2), parsef(i3))
  #define parse_quaternion(i1, i2, i3, i4) Quaternion(parsef(i1), parsef(i2), parsef(i3), parsef(i4))

  for (auto& line : lines) {
    if (line[0] == '@') {
      // Entity
      auto parts = SplitString(line, ",");  // @allocation
      // @todo use constant IDs which map to entity types
      EntityType entity_type = (EntityType)stoi(parts[0].substr(1));
      GameEntity entity = EntityManager::CreateNewEntity(state, entity_type);

      entity.position = parse_vec3f(1, 2, 3);
      entity.scale = parse_vec3f(4, 5, 6);
      entity.orientation = parse_quaternion(7, 8, 9, 10);
      entity.tint = parse_vec3f(11, 12, 13);
      entity.astro_start_time = parsef(14);
      entity.astro_end_time = parsef(15);

      entity.item_pickup_name = parts[16];
      entity.unique_name = parts[17];
      entity.associated_entity_name = parts[18];
      entity.requires_astro_sync = parse_bool(parts[19]);

      // @temporary
      if (entity_type == LAMPPOST) {
        entity.did_activate = true;
      }

      // Set base visible position + rotation
      entity.visible_position = entity.position;
      entity.visible_rotation = entity.orientation;

      EntityManager::SaveNewEntity(state, entity);

      auto& mesh_ids = EntityDispatcher::GetMeshes(state, entity.type);

      for (auto mesh_id : mesh_ids) {
        create(mesh_id);
      }
    }
    else if (line[0] == '$') {
      // Object
      auto parts = SplitString(line, ",");  // @allocation
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
        object.material = tVec4f(1.f, 0, 0, 0.1f);
      }

      // @temporary
      // @todo set mesh material properties
      if (mesh_index == state.meshes.rock_1 || mesh_index == state.meshes.rock_2) {
        object.material = tVec4f(1., 0, 0, 0);
      }

      // @temporary
      // @todo set mesh material properties
      if (mesh_index == state.meshes.river_edge) {
        object.color = tVec3f(0.27f, 0.135f, 0.135f);
        object.material = tVec4f(1.f, 0, 0, 1.f);
      }

      // @temporary
      // @todo set mesh material properties
      if (mesh_index == state.meshes.flat_ground) {
        object.material = tVec4f(1., 0, 0, 0);
      }

      commit(object);
    }
  }

  #undef parsef
  #undef parse_bool
  #undef parse_vec3f
  #undef parse_quaternion
}

void DataLoader::LoadNpcDialogue(Tachyon* tachyon, State& state) {
  auto level_data = Tachyon_GetFileContents("./astro/level_data/npc_dialogue.txt");
  auto lines = SplitString(level_data, "\n");  // @allocation

  std::string current_npc_name = "";

  for (auto& line : lines) {
    if (line.size() == 0) continue;
    if (line.starts_with("//")) continue;

    if (line[0] == '@') {
      // NPC or other dialogue trigger
      DialogueSet dialogue;

      if (line[1] == '@') {
        // Dialogue set with lines selected at random
        dialogue.random = true;

        current_npc_name = line.substr(2);
      } else {
        current_npc_name = line.substr(1);
      }

      state.npc_dialogue[current_npc_name] = dialogue;
    } else {
      // Dialogue lines
      auto& dialogue_set = state.npc_dialogue[current_npc_name];

      if (line.starts_with("+")) {
        // Lines starting with "+" should only be read the first time
        // the dialogue is invoked. Upon returning to the dialogue set,
        // we should continue from subsequent lines.
        dialogue_set.returning_first_line_index++;

        dialogue_set.lines.push_back(line.substr(1));
      } else {
        dialogue_set.lines.push_back(line);
      }
    }
  }
}