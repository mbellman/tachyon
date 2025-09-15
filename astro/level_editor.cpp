#include <format>
#include <string>
#include <vector>

#include "astro/level_editor.h"
#include "astro/entity_manager.h"

using namespace astro;

struct Selectable {
  bool is_entity;
  EntityRecord entity_record;
  tObject placeholder;
};

struct LevelEditorState {
  std::vector<Selectable> selectables;
  // @todo allow multiple?
  Selectable current_selectable;
  bool is_object_selected = false;
} editor;

/**
 * ----------------------------
 * Serialization helpers for different data types.
 * ----------------------------
 */
static inline std::string Serialize(float f) {
  return std::format("{:.3f}", f);
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

/**
 * ----------------------------
 * Converts an entity into a condensed string representation
 * for storage in a level data file.
 * ----------------------------
 */
std::string SerializeEntity(BaseEntity* entity) {
  switch (entity->type) {
    case SHRUB: {
      auto& plant = *(PlantEntity*)entity;

      return (
        std::to_string(plant.type) + "," +
        Serialize(plant.position) + "," +
        Serialize(plant.scale) + "," +
        Serialize(plant.orientation) + "," +
        Serialize(plant.tint) + "," +
        std::to_string(plant.astro_time_when_born)
      );
    }

    case OAK_TREE:
    case WILLOW_TREE: {
      auto& tree = *(TreeEntity*)entity;

      return (
        std::to_string(tree.type) + "," +
        Serialize(tree.position) + "," +
        Serialize(tree.scale) + "," +
        Serialize(tree.orientation) + "," +
        Serialize(tree.tint) + "," +
        std::to_string(tree.astro_time_when_born)
      );
    }

    default:
      // @todo log error
      return "";
  }
}

/**
 * ----------------------------
 * Writes all entities/objects in the level to a file.
 * ----------------------------
 */
void SaveWorldData(State& state) {
  std::string world_data = "";

  for (auto& selectable : editor.selectables) {
    if (selectable.is_entity) {
      auto* entity = EntityManager::FindEntity(state, selectable.entity_record);

      if (entity != nullptr) {
        world_data += SerializeEntity(entity) + "\n";
      } else {
        // @todo log error?
      }
    } else {
      // @todo serialize plain objects
    }
  }

  // @todo write to file
  printf("Saving!\n");
  printf("%s\n", world_data.c_str());
  printf("\n");
}

/**
 * ----------------------------
 * Sets the camera upon opening the editor.
 * ----------------------------
 */
static void InitEditorCamera(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f forward = camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f);

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));
}

/**
 * ----------------------------
 * Handles camera panning/movement in the editor.
 * ----------------------------
 */
static void HandleFreeCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  const float camera_panning_speed = 0.3f;
  const float camera_movement_speed = is_key_held(tKey::SPACE) ? 10000.f : 5000.f;

  // Mouse panning
  {
    camera.orientation.yaw += tachyon->mouse_delta_x * camera_panning_speed * dt;
    camera.orientation.pitch += tachyon->mouse_delta_y * camera_panning_speed * dt;
  }

  // WASD controls
  {
    if (is_key_held(tKey::W)) {
      camera.position += camera.orientation.getDirection() * camera_movement_speed * dt;
    }

    if (is_key_held(tKey::A)) {
      camera.position += camera.orientation.getLeftDirection() * camera_movement_speed * dt;
    }

    if (is_key_held(tKey::D)) {
      camera.position += camera.orientation.getLeftDirection().invert() * camera_movement_speed * dt;
    }

    if (is_key_held(tKey::S)) {
      camera.position += camera.orientation.getDirection().invert() * camera_movement_speed * dt;
    }
  }

  camera.rotation = camera.orientation.toQuaternion();
}

/**
 * ----------------------------
 * Selects a given Selectable, either an entity or plain object.
 * ----------------------------
 */
static void MakeSelection(Tachyon* tachyon, State& state, Selectable& selectable) {
  editor.is_object_selected = true;
  editor.current_selectable = selectable;

  auto& placeholder = selectable.placeholder;

  placeholder.color = tVec3f(1.f, 0, 1.f);

  commit(placeholder);
}

/**
 * ----------------------------
 * Deselects whatever Selectable is currently selected, if applicable.
 * ----------------------------
 */
static void DeselectCurrent(Tachyon* tachyon, State& state) {
  if (!editor.is_object_selected) {
    return;
  }

  editor.is_object_selected = false;

  auto& live_placeholder = *get_live_object(editor.current_selectable.placeholder);

  // Restore the placeholder color
  commit(live_placeholder);

  SaveWorldData(state);
}

/**
 * ----------------------------
 * Checks for Selectables in front of and near the camera,
 * and selects one if it is found.
 * ----------------------------
 */
static void MaybeMakeSelection(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f forward = camera.orientation.getDirection();
  float highest_candidate_score = 0.f;
  Selectable candidate;

  for (auto& selectable : editor.selectables) {
    auto& live_placeholder = *get_live_object(selectable.placeholder);
    float scale_limit = 20000.f; // @todo base on placeholder scale
    tVec3f camera_to_entity = live_placeholder.position - camera.position;
    float distance = camera_to_entity.magnitude();
    float selectable_dot = tVec3f::dot(forward, camera_to_entity.unit());

    if (distance > scale_limit || selectable_dot < 0.6f) {
      continue;
    }

    float score = (100.f * powf(selectable_dot, 20.f)) / distance;

    if (score > highest_candidate_score) {
      highest_candidate_score = score;
      candidate = selectable;
    }
  }

  // If we have a candidate selectable, select it
  if (highest_candidate_score > 0.f) {
    MakeSelection(tachyon, state, candidate);
  }
}

/**
 * ----------------------------
 * Handles inputs while the editor is open.
 * ----------------------------
 */
static void HandleEditorActions(Tachyon* tachyon, State& state) {
  if (did_left_click_down() && !editor.is_object_selected) {
    MaybeMakeSelection(tachyon, state);
  }

  if (did_right_click_down() && editor.is_object_selected) {
    DeselectCurrent(tachyon, state);
  }
}

/**
 * ----------------------------
 * Puts an entity and its placeholder object in the Selectables list
 * so it can be clicked on and manipulated in the editor.
 * ----------------------------
 */
static void TrackSelectableEntity(BaseEntity& entity, tObject& placeholder) {
  editor.selectables.push_back({
    .is_entity = true,
    .entity_record = {
      entity.id,
      entity.type
    },
    .placeholder = placeholder,
  });
}

/**
 * ----------------------------
 * Creates placeholder objects for all entities in the level.
 * ----------------------------
 */
static void SpawnEntityPlaceholders(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo refactor
  for_entities(state.shrubs) {
    auto& entity = state.shrubs[i];
    auto& placeholder = create(meshes.shrub_placeholder);

    placeholder.position = entity.position;
    placeholder.scale = tVec3f(800.f);
    placeholder.color = tVec3f(0.2f, 0.8f, 0.5f);

    commit(placeholder);

    TrackSelectableEntity(entity, placeholder);
  }

  // @todo refactor
  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];
    auto& placeholder = create(meshes.oak_tree_placeholder);

    placeholder.position = entity.position;
    placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
    placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

    commit(placeholder);

    TrackSelectableEntity(entity, placeholder);
  }

  // @todo refactor
  for_entities(state.willow_trees) {
    auto& entity = state.willow_trees[i];
    auto& placeholder = create(meshes.willow_tree_placeholder);

    placeholder.position = entity.position;
    placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
    placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

    commit(placeholder);

    TrackSelectableEntity(entity, placeholder);
  }
}

/**
 * ----------------------------
 * Resets the placeholder objects for all entities.
 * ----------------------------
 */
static void RemoveEntityPlaceholders(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.shrub_placeholder);
  remove_all(meshes.oak_tree_placeholder);
  remove_all(meshes.willow_tree_placeholder);
}

void LevelEditor::OpenLevelEditor(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.is_level_editor_open = true;

  objects(meshes.astrolabe_base).disabled = true;
  objects(meshes.astrolabe_ring).disabled = true;
  objects(meshes.astrolabe_hand).disabled = true;

  // @todo iterate over entity types + meshes for that entity
  objects(meshes.oak_tree_trunk).disabled = true;
  objects(meshes.willow_tree_trunk).disabled = true;
  objects(meshes.shrub_branches).disabled = true;

  editor.selectables.clear();

  SpawnEntityPlaceholders(tachyon, state);
  InitEditorCamera(tachyon, state);
}

void LevelEditor::CloseLevelEditor(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.is_level_editor_open = false;

  objects(meshes.astrolabe_base).disabled = false;
  objects(meshes.astrolabe_ring).disabled = false;
  objects(meshes.astrolabe_hand).disabled = false;

  // @todo iterate over entity types + meshes for that entity
  objects(meshes.oak_tree_trunk).disabled = false;
  objects(meshes.willow_tree_trunk).disabled = false;
  objects(meshes.shrub_branches).disabled = false;

  DeselectCurrent(tachyon, state);

  editor.selectables.clear();

  SaveWorldData(state);
  RemoveEntityPlaceholders(tachyon, state);
}

void LevelEditor::HandleLevelEditor(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    HandleFreeCamera(tachyon, state, dt);
    HandleEditorActions(tachyon, state);
  }
}