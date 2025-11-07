#include <format>
#include <map>
#include <string>
#include <vector>

#include "astro/level_editor.h"
#include "astro/data_loader.h"
#include "astro/entities_and_objects.h"
#include "astro/entity_manager.h"
#include "astro/entity_dispatcher.h"
#include "astro/items.h"
#include "astro/object_manager.h"
#include "astro/procedural_generation.h"

#define get_entity_defaults(__type) entity_defaults_map.at(__type)

using namespace astro;

enum GizmoAction {
  POSITION,
  SCALE,
  ROTATE
};

struct Selectable {
  bool is_entity;
  EntityRecord entity_record;
  tObject placeholder;
};

struct LevelEditorState {
  std::vector<Selectable> selectables;
  // @todo allow multiple?
  Selectable current_selectable;
  GizmoAction current_gizmo_action = POSITION;
  int32 current_decorative_mesh_index = 0;
  int32 current_entity_index = 0;

  bool is_anything_selected = false;
  bool is_in_placement_mode = false;
  bool is_placing_entity = false;

  bool is_editing_entity_properties = false;
  int editing_entity_step = 0;
  std::string edited_entity_property_value = "";
} editor;

/**
 * ----------------------------
 * Exception-safe string to type conversion helpers.
 * ----------------------------
 */
static inline float ToFloat(const std::string& string, const float fallback) {
  try {
    auto f = stof(string);

    return f;
  } catch (const std::invalid_argument& e) {
    return fallback;
  }
}

static inline float ToBool(const std::string& string) {
  return string == "true" || string == "1";
}

/**
 * ----------------------------
 * Serialization helpers for different data types.
 * ----------------------------
 */
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

static inline std::string Serialize(tColor& color) {
  return std::to_string(color.rgba);
}

/**
 * ----------------------------
 * Converts an entity into a condensed string representation
 * for storage in a level data file.
 * ----------------------------
 */
std::string SerializeEntity(const GameEntity& entity) {
  return (
    "@" +
    std::to_string(entity.type) + "," +
    Serialize(entity.position) + "," +
    Serialize(entity.scale) + "," +
    Serialize(entity.orientation) + "," +
    Serialize(entity.tint) + "," +
    std::to_string(entity.astro_start_time) + "," +
    std::to_string(entity.astro_end_time) + "," +
    entity.item_pickup_name + "," +
    entity.unique_name + "," +
    entity.associated_entity_name + "," +
    Serialize(entity.requires_astro_sync)
  );
}

/**
 * ----------------------------
 * Converts an object into a condensed string representation
 * for storage in a level data file.
 * ----------------------------
 */
std::string SerializeObject(State& state, tObject& object) {
  return (
    "$" +
    std::to_string(DataLoader::MeshIndexToId(state, object.mesh_index)) + "," +
    Serialize(object.position) + "," +
    Serialize(object.scale) + "," +
    Serialize(object.rotation) + "," +
    Serialize(object.color)
  );
}

/**
 * ----------------------------
 * Writes all entities/objects in the level to a file.
 * ----------------------------
 */
void SaveLevelData(Tachyon* tachyon, State& state) {
  log_time("SaveLevelData()");

  std::string level_data = "";

  for_all_entity_types() {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      level_data += SerializeEntity(entity) + "\n";
    }
  }

  for (auto& decorative : GetDecorativeMeshes(state)) {
    auto& objects = objects(decorative.mesh_index);

    // Perform a sequence-preserving loop over the objects
    // to ensure they always serialize in the same order
    // (e.g. if objects are removed/shuffled during runtime)
    for (uint16 id = 0; id <= objects.highest_used_id; id++) {
      tObject* object = objects.getById(id);

      if (object != nullptr) {
        level_data += SerializeObject(state, *object) + "\n";
      }
    }
  }

  Tachyon_WriteFileContents("./astro/level_data/level.txt", level_data);
}

/**
 * ----------------------------
 * Finds the global axis most similar to a given vector.
 * ----------------------------
 */
static tVec3f GetClosestWorldAxis(const tVec3f& vector) {
  float abs_x = abs(vector.x);
  float abs_y = abs(vector.y);
  float abs_z = abs(vector.z);

  if (abs_x > abs_y && abs_x > abs_z) {
    return tVec3f(vector.x, 0, 0).unit();
  } else if (abs_y > abs_x && abs_y > abs_z) {
    return tVec3f(0, vector.y, 0).unit();
  } else {
    return tVec3f(0, 0, vector.z).unit();
  }
}

/**
 * ----------------------------
 * Finds the object axis most similar to a given vector.
 * ----------------------------
 */
static tVec3f GetClosestObjectAxis(const tObject& object, const tVec3f& vector) {
  tVec3f object_up = object.rotation.getUpDirection();
  tVec3f object_right = object.rotation.getLeftDirection().invert();
  tVec3f object_forward = object.rotation.getDirection();

  float dot_up = tVec3f::dot(vector, object_up);
  float dot_right = tVec3f::dot(vector, object_right);
  float dot_forward = tVec3f::dot(vector, object_forward);

  float up_factor = abs(dot_up);
  float right_factor = abs(dot_right);
  float forward_factor = abs(dot_forward);

  if (up_factor > right_factor && up_factor > forward_factor) {
    return dot_up < 0.f ? object_up.invert() : object_up;
  } else if (right_factor > up_factor && right_factor > forward_factor) {
    return dot_right < 0.f ? object_right.invert() : object_right;
  } else {
    return dot_forward < 0.f ? object_forward.invert() : object_forward;
  }
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
 * Handles swiveling around the current selected object.
 * ----------------------------
 */
static void HandleSelectedObjectCameraSwiveling(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f origin = editor.current_selectable.placeholder.position;
  tVec3f offset = camera.position - origin;
  tVec3f unit_offset = offset.unit();

  tCamera3p camera3p;
  camera3p.radius = offset.magnitude();
  camera3p.azimuth = atan2f(unit_offset.z, unit_offset.x);
  camera3p.altitude = atan2f(unit_offset.y, unit_offset.xz().magnitude());

  if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) {
    camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
    camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
    camera3p.limitAltitude(0.99f);

    camera.position = origin + camera3p.calculatePosition();
  }

  camera.orientation.face(origin - camera.position, tVec3f(0, 1.f, 0));
}

/**
 * ----------------------------
 * Handles camera behavior in the editor.
 * ----------------------------
 */
static void HandleCameraActions(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  const float camera_panning_speed = 0.2f;
  const float camera_movement_speed = is_key_held(tKey::SPACE) ? 30000.f : 5000.f;

  // Object swiveling or mouse panning
  {
    if (!is_left_mouse_held_down() && !editor.is_editing_entity_properties) {
      if (is_key_held(tKey::SHIFT) && editor.is_anything_selected) {
        HandleSelectedObjectCameraSwiveling(tachyon, state);
      } else {
        camera.orientation.yaw += tachyon->mouse_delta_x * camera_panning_speed * dt;
        camera.orientation.pitch += tachyon->mouse_delta_y * camera_panning_speed * dt;
      }
    }
  }

  // WASD controls
  {
    if (!editor.is_editing_entity_properties) {
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
  }

  // Update camera rotation
  {
    float blend_alpha = 50.f * dt;

    if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0 || blend_alpha > 1.f) {
      blend_alpha = 1.f;
    }

    camera.rotation = Quaternion::slerp(camera.rotation, camera.orientation.toQuaternion(), blend_alpha);
  }
}

/**
 * ----------------------------
 * Puts an ordinary decorative object in the selectables list
 * so it can be clicked on and manipulated in the editor.
 * ----------------------------
 */
static void TrackDecorativeObject(tObject& object) {
  editor.selectables.push_back({
    .is_entity = false,
    .placeholder = object
  });
}

/**
 * ----------------------------
 * Puts an entity and its placeholder object in the selectables list
 * so it can be clicked on and manipulated in the editor.
 * ----------------------------
 */
static void TrackSelectableEntity(GameEntity& entity, tObject& placeholder) {
  editor.selectables.push_back({
    .is_entity = true,
    .entity_record = GetRecord(entity),
    .placeholder = placeholder,
  });
}

/**
 * ----------------------------
 * Ensures all placeholder objects stored in the selectables list
 * are updated to reflect their live counterparts.
 * ----------------------------
 */
static void SyncSelectables(Tachyon* tachyon) {
  for (auto& selectable : editor.selectables) {
    auto& live_placeholder = *get_live_object(selectable.placeholder);

    selectable.placeholder = live_placeholder;
  }
}

/**
 * ----------------------------
 * Removes an element from the selectables list by entity ID.
 * ----------------------------
 */
static void ForgetSelectableEntity(int32 entity_id) {
  for (size_t i = 0; i < editor.selectables.size(); i++) {
    if (entity_id == editor.selectables[i].entity_record.id) {
      editor.selectables.erase(editor.selectables.begin() + i);

      break;
    }
  }
}

/**
 * ----------------------------
 * Removes an element from the selectables list by object.
 * ----------------------------
 */
static void ForgetSelectableObject(tObject& object) {
  for (size_t i = 0; i < editor.selectables.size(); i++) {
    if (object == editor.selectables[i].placeholder) {
      editor.selectables.erase(editor.selectables.begin() + i);

      break;
    }
  }
}

/**
 * ----------------------------
 * Creates a gizmo using three instances of a provided mesh.
 * ----------------------------
 */
static void CreateGizmoFromMesh(Tachyon* tachyon, State& state, uint16 mesh_index) {
  auto& meshes = state.meshes;

  auto& up = create(mesh_index);
  auto& left = create(mesh_index);
  auto& forward = create(mesh_index);

  up.scale = tVec3f(40.f);
  left.scale = tVec3f(40.f);
  forward.scale = tVec3f(40.f);

  left.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
  forward.rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI);

  up.color = tVec4f(0, 1.f, 0, 0.4f);
  left.color = tVec4f(0, 0, 1.f, 0.4f);
  forward.color = tVec4f(1.f, 0, 0, 0.4f);

  commit(up);
  commit(left);
  commit(forward);
}

/**
 * ----------------------------
 * Creates a positioning gizmo.
 * ----------------------------
 */
static void CreatePositionGizmo(Tachyon* tachyon, State& state) {
  CreateGizmoFromMesh(tachyon, state, state.meshes.gizmo_arrow);
}

/**
 * ----------------------------
 * Creates a scale gizmo.
 * ----------------------------
 */
static void CreateScaleGizmo(Tachyon* tachyon, State& state) {
  CreateGizmoFromMesh(tachyon, state, state.meshes.gizmo_resizer);
}

/**
 * ----------------------------
 * Creates a rotation gizmo.
 * ----------------------------
 */
static void CreateRotationGizmo(Tachyon* tachyon, State& state) {
  CreateGizmoFromMesh(tachyon, state, state.meshes.gizmo_rotator);
}

/**
 * ----------------------------
 * Updates the gizmo pieces created from a given mesh.
 * ----------------------------
 */
static void UpdateGizmoFromMesh(Tachyon* tachyon, State& state, uint16 mesh_index) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;
  auto& up = objects(mesh_index)[0];
  auto& left = objects(mesh_index)[1];
  auto& forward = objects(mesh_index)[2];

  auto& placeholder = editor.current_selectable.placeholder;
  tVec3f camera_to_selected = placeholder.position - camera.position;
  tVec3f position = camera.position + camera_to_selected.unit() * 650.f;

  up.position = position;
  left.position = position;
  forward.position = position;

  up.rotation = placeholder.rotation;
  left.rotation = placeholder.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
  forward.rotation = placeholder.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI);

  commit(up);
  commit(left);
  commit(forward);
}

/**
 * ----------------------------
 * Updates the positioning gizmo.
 * ----------------------------
 */
static void UpdatePositionGizmo(Tachyon* tachyon, State& state) {
  UpdateGizmoFromMesh(tachyon, state, state.meshes.gizmo_arrow);
}

/**
 * ----------------------------
 * Updates the scale gizmo.
 * ----------------------------
 */
static void UpdateScaleGizmo(Tachyon* tachyon, State& state) {
  UpdateGizmoFromMesh(tachyon, state, state.meshes.gizmo_resizer);
}

/**
 * ----------------------------
 * Updates the rotation gizmo.
 * ----------------------------
 */
static void UpdateRotationGizmo(Tachyon* tachyon, State& state) {
  UpdateGizmoFromMesh(tachyon, state, state.meshes.gizmo_rotator);

  auto& placeholder = editor.current_selectable.placeholder;
  auto& forward = objects(state.meshes.gizmo_rotator)[2];

  forward.rotation = placeholder.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI);

  commit(forward);
}

/**
 * ----------------------------
 * Creates the objects for a given gizmo type.
 * ----------------------------
 */
static void CreateGizmo(Tachyon* tachyon, State& state, GizmoAction action) {
  if (action == POSITION) {
    CreatePositionGizmo(tachyon, state);
  }
  else if (action == SCALE) {
    CreateScaleGizmo(tachyon, state);
  }
  else if (action == ROTATE) {
    CreateRotationGizmo(tachyon, state);
  }
}

/**
 * ----------------------------
 * Updates the objects for the current gizmo.
 * ----------------------------
 */
static void UpdateCurrentGizmo(Tachyon* tachyon, State& state) {
  if (editor.current_gizmo_action == POSITION) {
    UpdatePositionGizmo(tachyon, state);
  }
  else if (editor.current_gizmo_action == SCALE) {
    UpdateScaleGizmo(tachyon, state);
  }
  else if (editor.current_gizmo_action == ROTATE) {
    UpdateRotationGizmo(tachyon, state);
  }
}

/**
 * ----------------------------
 * Destroys gizmo-related objects.
 * ----------------------------
 */
static void DestroyGizmo(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.gizmo_arrow);
  remove_all(meshes.gizmo_resizer);
  remove_all(meshes.gizmo_rotator);
}

/**
 * ----------------------------
 * Creates the entity/object placer.
 * ----------------------------
 */
static void CreatePlacer(Tachyon* tachyon, State& state) {
  create(state.meshes.editor_placer);
}

/**
 * ----------------------------
 * Enters object/entity placement mode.
 * ----------------------------
 */
static void EnterPlacementMode(Tachyon* tachyon, State& state) {
  if (editor.is_in_placement_mode) {
    return;
  }

  editor.is_in_placement_mode = true;

  CreatePlacer(tachyon, state);
}

/**
 * ----------------------------
 * Handles updates for the entity/object placer.
 * ----------------------------
 */
static void UpdatePlacer(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_direction = camera.orientation.getDirection();
  float camera_height = camera.position.y - -1500.f;
  float distance = -camera_height / camera_direction.y;
  if (distance > 50000.f) distance = 50000.f;

  auto& placer = objects(state.meshes.editor_placer)[0];

  placer.position = camera.position + camera_direction * distance;
  placer.position.y = -1500.f;

  placer.scale = tVec3f(2000.f);
  placer.color = tVec4f(0.7f, 0.1f, 0.1f, 0.6f);
  placer.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI);

  commit(placer);
}

/**
 * ----------------------------
 * Destroys the entity/object placer.
 * ----------------------------
 */
static void DestroyPlacer(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.editor_placer);
}

/**
 * ----------------------------
 * Exits object/entity placement mode, removing the placer preview.
 * ----------------------------
 */
static void ExitPlacementMode(Tachyon* tachyon, State& state) {
  editor.is_in_placement_mode = false;

  DestroyPlacer(tachyon, state);
}

/**
 * ----------------------------
 * Cycles between available gizmo actions.
 * ----------------------------
 */
static void CycleGizmoAction(Tachyon* tachyon, State& state, int8 direction) {
  DestroyGizmo(tachyon, state);

  if (direction == 1) {
    // Cycle down
    if (editor.current_gizmo_action == POSITION) {
      editor.current_gizmo_action = SCALE;
    }
    else if (editor.current_gizmo_action == SCALE) {
      editor.current_gizmo_action = ROTATE;
    }
    else if (editor.current_gizmo_action == ROTATE) {
      editor.current_gizmo_action = POSITION;
    }
  }
  else if (direction == -1) {
    // Cycle up
    if (editor.current_gizmo_action == POSITION) {
      editor.current_gizmo_action = ROTATE;
    }
    else if (editor.current_gizmo_action == ROTATE) {
      editor.current_gizmo_action = SCALE;
    }
    else if (editor.current_gizmo_action == SCALE) {
      editor.current_gizmo_action = POSITION;
    }
  }

  CreateGizmo(tachyon, state, editor.current_gizmo_action);
}

/**
 * ----------------------------
 * Cycles between decorative meshes.
 * ----------------------------
 */
static void CycleDecorativeMeshes(Tachyon* tachyon, State& state, int8 direction) {
  editor.is_placing_entity = false;

  auto& decorative_meshes = GetDecorativeMeshes(state);

  if (direction == 1) {
    editor.current_decorative_mesh_index++;

    if (editor.current_decorative_mesh_index > (int32)decorative_meshes.size() - 1) {
      editor.current_decorative_mesh_index = 0;
    }
  }
  else if (direction == -1) {
    editor.current_decorative_mesh_index--;

    if (editor.current_decorative_mesh_index < 0) {
      editor.current_decorative_mesh_index = decorative_meshes.size() - 1;
    }
  }

  show_overlay_message("Active mesh: " + decorative_meshes[editor.current_decorative_mesh_index].mesh_name);
}

/**
 * ----------------------------
 * Cycles between entities.
 * ----------------------------
 */
static void CycleEntities(Tachyon* tachyon, State& state, int8 direction) {
  editor.is_placing_entity = true;

  if (direction == 1) {
    editor.current_entity_index++;

    if (editor.current_entity_index > (int32)entity_types.size() - 1) {
      editor.current_entity_index = 0;
    }
  }
  else if (direction == -1) {
    editor.current_entity_index--;

    if (editor.current_entity_index < 0) {
      editor.current_entity_index = entity_types.size() - 1;
    }
  }

  auto entity_type = entity_types[editor.current_entity_index];
  std::string& entity_name = get_entity_defaults(entity_type).name;

  show_overlay_message("Active entity: " + entity_name);
}

/**
 * ----------------------------
 * Initiates the entity property editor flow.
 * ----------------------------
 */
static void StartEditingEntityProperties(Tachyon* tachyon) {
  editor.is_editing_entity_properties = true;
  editor.editing_entity_step = 0;
  editor.edited_entity_property_value = "";

  // Ensure engine hotkeys don't accidentally trigger while typing
  tachyon->hotkeys_enabled = false;
}

/**
 * ----------------------------
 * Exits the entity property editor flow.
 * ----------------------------
 */
static void StopEditingEntityProperties(Tachyon* tachyon) {
  editor.is_editing_entity_properties = false;
  editor.editing_entity_step = 0;
  editor.edited_entity_property_value = "";
}

/**
 * ----------------------------
 * Flow for editing selected entity properties.
 * ----------------------------
 */
static void HandleEntityPropertiesEditor(Tachyon* tachyon, State& state) {
  auto& entity_record = editor.current_selectable.entity_record;
  auto* entity = EntityManager::FindEntity(state, entity_record);
  auto& property_value = editor.edited_entity_property_value;

  if (get_text_input()) {
    property_value += get_text_input();
  }

  if (did_press_key(tKey::BACKSPACE) && property_value.size() > 0) {
    property_value.pop_back();
  }

  if (did_press_key(tKey::ENTER)) {
    // 1. astro_start_time
    if (editor.editing_entity_step == 0) {
      entity->astro_start_time = ToFloat(property_value, entity->astro_start_time);
    }
    // 2. astro_end_time
    else if (editor.editing_entity_step == 1) {
      entity->astro_end_time = ToFloat(property_value, entity->astro_end_time);
    }
    // 3. item_pickup_name
    else if (editor.editing_entity_step == 2) {
      if (property_value != "") {
        entity->item_pickup_name = property_value;
      }
    }
    // 4. unique_name
    else if (editor.editing_entity_step == 3) {
      if (property_value != "") {
        entity->unique_name = property_value;
      }
    }
    // 5. associated_entity_name
    else if (editor.editing_entity_step == 4) {
      if (property_value != "") {
        entity->associated_entity_name = property_value;
      }
    }
    // 6. requires_astro_sync
    else if (editor.editing_entity_step == 5) {
      entity->requires_astro_sync = ToBool(property_value);

      StopEditingEntityProperties(tachyon);

      return;
    }

    editor.editing_entity_step++;
    editor.edited_entity_property_value = "";
  }
}

/**
 * ----------------------------
 * Selects a given Selectable, either an entity or plain object.
 * ----------------------------
 */
static void MakeSelection(Tachyon* tachyon, State& state, Selectable& selectable) {
  editor.is_anything_selected = true;
  editor.current_selectable = selectable;
  editor.current_gizmo_action = POSITION;

  auto& placeholder = editor.current_selectable.placeholder;

  placeholder.color = tVec4f(1.f, 0, 1.f, 0.2f);

  commit(placeholder);

  CreateGizmo(tachyon, state, editor.current_gizmo_action);
}

/**
 * ----------------------------
 * Deselects whatever Selectable is currently selected, if applicable.
 * ----------------------------
 */
static void DeselectCurrent(Tachyon* tachyon, State& state) {
  auto& placeholder = editor.current_selectable.placeholder;
  auto& live_placeholder = *get_live_object(placeholder);

  // Save transformations
  live_placeholder.position = placeholder.position;
  live_placeholder.scale = placeholder.scale;
  live_placeholder.rotation = placeholder.rotation;

  commit(live_placeholder);

  editor.is_anything_selected = false;

  StopEditingEntityProperties(tachyon);
  DestroyGizmo(tachyon, state);
  SyncSelectables(tachyon);
  SaveLevelData(tachyon, state);

  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  if (!tachyon->hotkeys_enabled) {
    // We disable engine hotkeys when starting the entity editor flow.
    // On deselection, restore hotkey behavior.
    tachyon->hotkeys_enabled = true;
  }
}

/**
 * ----------------------------
 * Checks for selectables in front of and near the camera,
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
    float distance_limit = live_placeholder.scale.magnitude() * 10.f;
    tVec3f camera_to_entity = live_placeholder.position - camera.position;
    float distance = camera_to_entity.magnitude();
    float dot = tVec3f::dot(forward, camera_to_entity.unit());

    if (distance > distance_limit || dot < 0.6f) {
      continue;
    }

    float score = (100.f * powf(dot, 20.f)) / distance;

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
 * Copies entity properties to its placeholder object.
 * ----------------------------
 */
static void SyncEntityPlaceholder(Tachyon* tachyon, tObject& placeholder, const GameEntity& entity) {
  placeholder.position = entity.position;
  placeholder.scale = entity.scale;
  placeholder.rotation = entity.orientation;
  placeholder.color = entity.tint;

  // @temporary
  // @todo allow entity placeholder default material
  {
    if (entity.type == ITEM_PICKUP) {
      placeholder.material = tVec4f(0.2f, 0, 0.5f, 0.5f);
    }

    if (entity.type == DIRT_PATH_NODE) {
      placeholder.material = tVec4f(1.f, 0, 0, 0.2f);
    }
  }

  commit(placeholder);
}

/**
 * ----------------------------
 * Creates and tracks a placeholder object for an entity.
 * ----------------------------
 */
static void SpawnEntityPlaceholder(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto placeholder_mesh_id = EntityDispatcher::GetPlaceholderMesh(state, entity.type);
  auto& placeholder = create(placeholder_mesh_id);

  SyncEntityPlaceholder(tachyon, placeholder, entity);
  TrackSelectableEntity(entity, placeholder);
}

/**
 * ----------------------------
 * Creates objects and a placeholder for an entity.
 * ----------------------------
 */
static void SpawnEntityObjects(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& mesh_ids = EntityDispatcher::GetMeshes(state, entity.type);

  for (auto mesh_id : mesh_ids) {
    create(mesh_id);
  }

  SpawnEntityPlaceholder(tachyon, state, entity);
}

/**
 * ----------------------------
 * Returns the position a new object or entity should spawn at when placed.
 * @todo remove the legacy non-placement-mode conditions
 * ----------------------------
 */
static tVec3f GetSpawnPosition(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  if (editor.is_in_placement_mode) {
    auto& placer = objects(state.meshes.editor_placer)[0];

    if (editor.is_placing_entity) {
      // @temporary
      // @todo define entity placement defaults
      EntityType entity_type = entity_types[editor.current_entity_index];

      if (entity_type == DIRT_PATH_NODE) {
        return placer.position + tVec3f(0, 1500.f, 0);
      }
    }

    return placer.position;
  }
  else if (editor.is_placing_entity) {
    EntityType entity_type = entity_types[editor.current_entity_index];
    auto& defaults = get_entity_defaults(entity_type);

    // @temporary
    // @todo define various restrictions/defaults on how certain
    // decorative mesh objects are spawned or can be manipulated
    if (entity_type == DIRT_PATH) {
      tVec3f position = camera.position + camera.orientation.getDirection() * abs(camera.position.y) * 1.5f;
      position.y = -1450.f;

      return position;
    } else {
      return camera.position + camera.orientation.getDirection() * 7500.f;
    }
  }
  else {
    auto& decorative_meshes = GetDecorativeMeshes(state);
    auto& current_decorative_mesh = decorative_meshes[editor.current_decorative_mesh_index];

    // @temporary
    // @todo define various restrictions/defaults on how certain
    // decorative mesh objects are spawned or can be manipulated
    if (current_decorative_mesh.mesh_index == state.meshes.flat_ground) {
      tVec3f position = camera.position + camera.orientation.getDirection() * abs(camera.position.y) * 1.5f;
      position.y = -1500.f;

      return position;
    } else {
      return camera.position + camera.orientation.getDirection() * 7500.f;
    }
  }
}

/**
 * ----------------------------
 * Creates a normal object and adds it to the scene.
 * @todo rename to reflect this is for the current-selected type
 * ----------------------------
 */
static void CreateNewDecorativeObject(Tachyon* tachyon, State& state) {
  auto& decorative_meshes = GetDecorativeMeshes(state);
  auto& current_decorative_mesh = decorative_meshes[editor.current_decorative_mesh_index];
  auto& object = create(current_decorative_mesh.mesh_index);

  object.position = GetSpawnPosition(tachyon, state);
  object.scale = current_decorative_mesh.default_scale;
  object.color = current_decorative_mesh.default_color;

  commit(object);

  TrackDecorativeObject(object);

  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  // @todo remove
  if (!editor.is_in_placement_mode) {
    MakeSelection(tachyon, state, editor.selectables.back());
  }
}

/**
 * ----------------------------
 * Creates an entity and adds it to the scene.
 * @todo rename to reflect this is for the current-selected type
 * ----------------------------
 */
static void CreateNewEntity(Tachyon* tachyon, State& state) {
  EntityType entity_type = entity_types[editor.current_entity_index];
  auto& defaults = get_entity_defaults(entity_type);
  GameEntity entity = EntityManager::CreateNewEntity(state, entity_type);

  entity.position = GetSpawnPosition(tachyon, state);
  entity.scale = defaults.scale;
  entity.orientation = Quaternion(1.f, 0, 0, 0);
  entity.tint = defaults.tint;
  // @temporary
  entity.astro_start_time = -50.f;

  EntityManager::SaveNewEntity(state, entity);

  SpawnEntityObjects(tachyon, state, entity);

  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);
}

/**
 * ----------------------------
 * Handler for moving selected objects around.
 * ----------------------------
 */
static void HandleSelectedObjectPositionActions(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& placeholder = editor.current_selectable.placeholder;
  float move_speed = (placeholder.position - camera.position).magnitude() / 4000.f;

  if (abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y)) {
    tVec3f camera_left = camera.orientation.getLeftDirection();
    tVec3f move_axis = GetClosestObjectAxis(placeholder, camera_left);

    placeholder.position -= move_axis * move_speed * (float)tachyon->mouse_delta_x;
  } else {
    tVec3f move_axis = tVec3f(0, 1.f, 0);

    placeholder.position -= move_axis * move_speed * (float)tachyon->mouse_delta_y;
  }

  // @temporary
  if (placeholder.mesh_index == state.meshes.flat_ground) {
    placeholder.position.y = -1500.f;
  }
  else if (placeholder.mesh_index == state.meshes.dirt_path_placeholder) {
    placeholder.position.y = -1450.f;
  }

  // @optimize We don't need to do this every time the object is moved!
  // It would be perfectly acceptable to do this on deselection.
  if (editor.current_selectable.is_entity) {
    // Find and sync the position of the original entity
    auto* entity = EntityManager::FindEntity(state, editor.current_selectable.entity_record);

    entity->position = placeholder.position;
  }

  commit(placeholder);
}

/**
 * ----------------------------
 * Handler for scaling selected objects.
 * ----------------------------
 */
static void HandleSelectedObjectScaleActions(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& placeholder = editor.current_selectable.placeholder;
  float scale_speed = (placeholder.position - camera.position).magnitude() / 4000.f;

  if (abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y)) {
    tVec3f camera_left = camera.orientation.getLeftDirection();

    // The scale axis must be ascertained over several steps.
    // First, we get the object axis most similar to camera-left.
    tVec3f scale_axis = GetClosestObjectAxis(placeholder, camera_left);

    // Next, we inverse transform the object axis into a world axis.
    // This doesn't mean we'll be scaling the object along the world axis,
    // but scaling the object along x or z is the same operation regardless
    // of its rotation, so we have to treat that as a globally stable thing.
    scale_axis = placeholder.rotation.toMatrix4f().inverse() * scale_axis;
    scale_axis = GetClosestWorldAxis(scale_axis);

    // Ensure dragging the mouse right or left always scales in the correct direction,
    // regardless of which direction we're looking along X or Z
    if (scale_axis.x == 1.f) scale_axis.x = -1.f;
    if (scale_axis.z == 1.f) scale_axis.z = -1.f;

    placeholder.scale -= scale_axis * scale_speed * (float)tachyon->mouse_delta_x;
  } else {
    tVec3f scale_axis = tVec3f(0, 1.f, 0);

    placeholder.scale -= scale_axis * scale_speed * (float)tachyon->mouse_delta_y;
  }

  // @temporary
  if (
    placeholder.mesh_index == state.meshes.flat_ground ||
    placeholder.mesh_index == state.meshes.dirt_path_placeholder
  ) {
    placeholder.scale.y = 1.f;
  }

  // @optimize We don't need to do this every time the object is moved!
  // It would be perfectly acceptable to do this on deselection.
  if (editor.current_selectable.is_entity) {
    // Find and sync the scale of the original entity
    auto* entity = EntityManager::FindEntity(state, editor.current_selectable.entity_record);

    entity->scale = placeholder.scale;
  }

  commit(placeholder);
}

/**
 * ----------------------------
 * Handler for rotating selected objects.
 * ----------------------------
 */
static void HandleSelectedObjectRotateActions(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& placeholder = editor.current_selectable.placeholder;
  float scale_speed = (placeholder.position - camera.position).magnitude() / 4000.f;

  if (abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y)) {
    placeholder.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), (float)tachyon->mouse_delta_x * 0.001f);
  } else {
    // @todo
  }

  // @optimize We don't need to do this every time the object is moved!
  // It would be perfectly acceptable to do this on deselection.
  if (editor.current_selectable.is_entity) {
    // Find and sync the rotation of the original entity
    auto* entity = EntityManager::FindEntity(state, editor.current_selectable.entity_record);

    entity->orientation = placeholder.rotation;
    entity->visible_rotation = placeholder.rotation;
  }

  commit(placeholder);
}

/**
 * ----------------------------
 * Handler for manipulating selected objects with the mouse.
 * ----------------------------
 */
static void HandleSelectedObjectActions(Tachyon* tachyon, State& state) {
  if (tachyon->mouse_delta_x == 0 && tachyon->mouse_delta_y == 0) {
    return;
  }

  if (editor.current_gizmo_action == POSITION) {
    HandleSelectedObjectPositionActions(tachyon, state);
  }
  else if (editor.current_gizmo_action == SCALE) {
    HandleSelectedObjectScaleActions(tachyon, state);
  }
  else if (editor.current_gizmo_action == ROTATE) {
    HandleSelectedObjectRotateActions(tachyon, state);
  }
}

/**
 * ----------------------------
 * Formatters for displaying different types of values in the editor HUD.
 * ----------------------------
 */
static std::string FormatForDisplay(const tVec3f& vector) {
  return std::format("{{ {:.1f}, {:.1f}, {:.1f} }}", vector.x, vector.y, vector.z);
}

static std::string FormatForDisplay(const Quaternion& q) {
  return std::format("{{ {:.3f}, {:.3f}, {:.3f}, {:.3f} }}", q.x, q.y, q.z, q.w);
}

/**
 * ----------------------------
 * Renders a series of info labels in the upper right of the screen.
 * ----------------------------
 */
static void RenderInfoLabels(Tachyon* tachyon, State& state, const std::vector<std::string>& labels) {
  for (int32 i = 0; (int32)i < labels.size(); i++) {
    auto& label = labels[i];

    Tachyon_DrawUIText(tachyon, state.debug_text, {
      .screen_x = tachyon->window_width - 480,
      .screen_y = 20 + (i * 25),
      .centered = false,
      .color = tVec3f(1.f),
      .string = label
    });
  }
}

/**
 * ----------------------------
 * Displays information for the current-selected entity.
 * ----------------------------
 */
static void DisplaySelectedEntityProperties(Tachyon* tachyon, State& state) {
  auto& selected = editor.current_selectable;
  auto& entity_name = get_entity_defaults(selected.entity_record.type).name;
  auto& entity = *EntityManager::FindEntity(state, selected.entity_record);
  int32 total_active = EntityDispatcher::GetEntityContainer(state, entity.type).size();
  auto& mesh = mesh(selected.placeholder.mesh_index);
  int32 total_verts = mesh.lod_1.vertex_end - mesh.lod_1.vertex_start;
  int32 summed_verts = total_active * total_verts;

  static std::vector<std::string> labels;

  labels.clear();
  labels.push_back("Entity: " + entity_name + " (" + std::to_string(total_active) + " active)");
  labels.push_back("Vertices: " + std::to_string(total_verts) + " [" + std::to_string(summed_verts) + "]");
  labels.push_back("Entity ID: " + std::to_string(entity.id));
  labels.push_back("position: " + FormatForDisplay(entity.position));
  labels.push_back("scale: " + FormatForDisplay(entity.scale));
  labels.push_back("orientation: " + FormatForDisplay(entity.orientation));

  bool blink_text_cursor = int(roundf(tachyon->running_time * 2.f)) % 2 == 0;
  std::string text_cursor = blink_text_cursor ? "|" : "";

  #define is_on_editing_step(step) editor.is_editing_entity_properties && editor.editing_entity_step == step

  if (is_on_editing_step(0)) {
    labels.push_back(".astro_start_time: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".astro_start_time: " + Serialize(entity.astro_start_time));
  }

  if (is_on_editing_step(1)) {
    labels.push_back(".astro_end_time: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".astro_end_time: " + Serialize(entity.astro_end_time));
  }

  if (is_on_editing_step(2)) {
    labels.push_back(".item_pickup_name: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".item_pickup_name: " + entity.item_pickup_name);
  }

  if (is_on_editing_step(3)) {
    labels.push_back(".unique_name: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".unique_name: " + entity.unique_name);
  }

  if (is_on_editing_step(4)) {
    labels.push_back(".associated_entity_name: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".associated_entity_name: " + entity.associated_entity_name);
  }

  if (is_on_editing_step(5)) {
    labels.push_back(".requires_astro_sync: " + editor.edited_entity_property_value + text_cursor);
  } else {
    labels.push_back(".requires_astro_sync: " + std::string(entity.requires_astro_sync ? "true" : "false"));
  }

  RenderInfoLabels(tachyon, state, labels);
}

/**
 * ----------------------------
 * Displays information for the current-selected object.
 * ----------------------------
 */
static void DisplaySelectedObjectProperties(Tachyon* tachyon, State& state) {
  auto& object = editor.current_selectable.placeholder;
  auto& mesh = mesh(object.mesh_index);
  auto& mesh_name = GetDecorativeMeshName(state, object.mesh_index);
  uint16 total_active = objects(object.mesh_index).total_active;
  int32 total_verts = mesh.lod_1.vertex_end - mesh.lod_1.vertex_start;
  int32 summed_verts = total_active * total_verts;

  static std::vector<std::string> labels;

  labels.clear();
  labels.push_back("Mesh: " + mesh_name + " (" + std::to_string(total_active) + " active)");
  labels.push_back("Vertices: " + std::to_string(total_verts) + " [" + std::to_string(summed_verts) + "]");
  labels.push_back("Object ID: " + std::to_string(object.object_id));
  labels.push_back("position: " + FormatForDisplay(object.position));
  labels.push_back("scale: " + FormatForDisplay(object.scale));
  labels.push_back("rotation: " + FormatForDisplay(object.rotation));

  RenderInfoLabels(tachyon, state, labels);
}

/**
 * ----------------------------
 * Removes the last object of a given mesh.
 * ----------------------------
 */
static void RemoveLastObject(Tachyon* tachyon, uint16 mesh_index) {
  auto& objects = objects(mesh_index);
  uint16 total_active = objects.total_active;

  if (total_active > 0) {
    auto& last_object = objects[total_active - 1];

    remove_object(last_object);
  }
}

/**
 * ----------------------------
 * Deletes the currently-selected entity or object.
 * ----------------------------
 */
static void DeleteSelected(Tachyon* tachyon, State& state) {
  auto& selected = editor.current_selectable;

  remove_object(selected.placeholder);

  if (selected.is_entity) {
    ForgetSelectableEntity(selected.entity_record.id);

    auto& entity = *EntityManager::FindEntity(state, selected.entity_record);

    if (entity.light_id > -1) {
      remove_point_light(entity.light_id);
    }

    EntityManager::DeleteEntity(state, selected.entity_record);

    // Remove objects associated with the entity
    auto& mesh_ids = EntityDispatcher::GetMeshes(state, selected.entity_record.type);

    for (auto mesh_id : mesh_ids) {
      RemoveLastObject(tachyon, mesh_id);
    }
  } else {
    ForgetSelectableObject(selected.placeholder);
  }

  DestroyGizmo(tachyon, state);

  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  editor.is_anything_selected = false;
}

/**
 * ----------------------------
 * Respawns the player near the camera.
 * ----------------------------
 */
static void RespawnPlayer(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_direction = camera.orientation.getDirection();
  float camera_height = camera.position.y - -1500.f;
  float distance = -camera_height / camera_direction.y;

  state.player_position = camera.position + camera_direction * distance;
  state.player_position.y = 0.f;

  auto& player = objects(state.meshes.player)[0];

  player.position = state.player_position;

  commit(player);
}

/**
 * ----------------------------
 * Handles inputs while the editor is open.
 * ----------------------------
 */
static void HandleEditorActions(Tachyon* tachyon, State& state) {
  if (did_left_click_down() && !editor.is_anything_selected) {
    if (editor.is_in_placement_mode) {
      if (editor.is_placing_entity) {
        CreateNewEntity(tachyon, state);
      } else {
        CreateNewDecorativeObject(tachyon, state);
      }
    } else {
      MaybeMakeSelection(tachyon, state);
    }
  }

  if (did_press_key(tKey::CONTROL)) {
    tachyon->use_high_visibility_mode = !tachyon->use_high_visibility_mode;

    if (tachyon->use_high_visibility_mode) {
      show_overlay_message("Lighting disabled");
    } else {
      show_overlay_message("Lighting enabled");
    }
  }

  if (editor.is_anything_selected) {
    // Selected object/entity actions
    if (is_left_mouse_held_down()) {
      HandleSelectedObjectActions(tachyon, state);
    }

    if (
      did_press_key(tKey::BACKSPACE) &&
      !editor.is_editing_entity_properties
    ) {
      DeleteSelected(tachyon, state);
    }

    if (did_right_click_down()) {
      DeselectCurrent(tachyon, state);
    }

    if (did_wheel_down()) {
      CycleGizmoAction(tachyon, state, 1);
    }

    if (did_wheel_up()) {
      CycleGizmoAction(tachyon, state, -1);
    }

    if (
      did_press_key(tKey::ENTER) &&
      editor.current_selectable.is_entity &&
      !editor.is_editing_entity_properties
    ) {
      StartEditingEntityProperties(tachyon);
    }
    else if (editor.is_editing_entity_properties) {
      HandleEntityPropertiesEditor(tachyon, state);
    }
  } else {
    // Free actions
    if (did_press_key(tKey::ARROW_LEFT)) {
      CycleDecorativeMeshes(tachyon, state, -1);
      EnterPlacementMode(tachyon, state);
    }

    if (did_press_key(tKey::ARROW_RIGHT)) {
      CycleDecorativeMeshes(tachyon, state, 1);
      EnterPlacementMode(tachyon, state);
    }

    if (did_press_key(tKey::ARROW_UP)) {
      CycleEntities(tachyon, state, -1);
      EnterPlacementMode(tachyon, state);
    }

    if (did_press_key(tKey::ARROW_DOWN)) {
      CycleEntities(tachyon, state, 1);
      EnterPlacementMode(tachyon, state);
    }

    if (did_right_click_down() && editor.is_in_placement_mode) {
      ExitPlacementMode(tachyon, state);
    }

    if (did_press_key(tKey::R)) {
      RespawnPlayer(tachyon, state);
    }
  }
}

/**
 * ----------------------------
 * Creates placeholder objects for all entities in the level.
 * ----------------------------
 */
static void SpawnEntityPlaceholders(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for_all_entity_types() {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      SpawnEntityPlaceholder(tachyon, state, entity);
    }
  }
}

/**
 * ----------------------------
 * Puts all decorative objects in the selectables list.
 * ----------------------------
 */
static void TrackDecorativeObjects(Tachyon* tachyon, State& state) {
  for (auto& decorative : GetDecorativeMeshes(state)) {
    for (auto& object : objects(decorative.mesh_index)) {
      TrackDecorativeObject(object);
    }
  }
}

/**
 * ----------------------------
 * Destroys the placeholder objects for all entities.
 * ----------------------------
 */
static void RemoveEntityPlaceholders(Tachyon* tachyon, State& state) {
  for_all_entity_types() {
    auto placeholder_mesh_id = EntityDispatcher::GetPlaceholderMesh(state, type);

    remove_all(placeholder_mesh_id);
  }
}

/**
 * ----------------------------
 * Displays decorative object placement details.
 * ----------------------------
 */
static void DisplayObjectPlacementLabels(Tachyon* tachyon, State& state) {
  auto& decorative_mesh = GetDecorativeMeshes(state)[editor.current_decorative_mesh_index];
  auto& mesh = mesh(decorative_mesh.mesh_index);
  auto& mesh_name = GetDecorativeMeshName(state, decorative_mesh.mesh_index);
  uint16 total_active = objects(decorative_mesh.mesh_index).total_active;
  int32 total_verts = mesh.lod_1.vertex_end - mesh.lod_1.vertex_start;
  int32 summed_verts = total_active * total_verts;

  static std::vector<std::string> labels;

  labels.clear();
  labels.push_back("Mesh: " + mesh_name + " (" + std::to_string(total_active) + " active)");
  labels.push_back("Vertices: " + std::to_string(total_verts) + " [" + std::to_string(summed_verts) + "]");

  RenderInfoLabels(tachyon, state, labels);
}

/**
 * ----------------------------
 * Displays entity placement details.
 * ----------------------------
 */
static void DisplayEntityPlacementLabels(Tachyon* tachyon, State& state) {
  EntityType entity_type = entity_types[editor.current_entity_index];
  auto& defaults = get_entity_defaults(entity_type);
  auto& entity_name = defaults.name;
  uint16 mesh_index = EntityDispatcher::GetPlaceholderMesh(state, entity_type);
  int32 total_active = EntityDispatcher::GetEntityContainer(state, entity_type).size();
  auto& mesh = mesh(mesh_index);
  int32 total_verts = mesh.lod_1.vertex_end - mesh.lod_1.vertex_start;
  int32 summed_verts = total_active * total_verts;

  static std::vector<std::string> labels;

  labels.clear();
  labels.push_back("Entity: " + entity_name + " (" + std::to_string(total_active) + " active)");
  labels.push_back("Vertices: " + std::to_string(total_verts) + " [" + std::to_string(summed_verts) + "]");

  RenderInfoLabels(tachyon, state, labels);
}

/**
 * ----------------------------
 * Displays UI and HUD elements.
 * ----------------------------
 */
static void HandleUI(Tachyon* tachyon, State& state) {
  Tachyon_DrawUIText(tachyon, state.debug_text_large, {
    .screen_x = tachyon->window_width / 2,
    .screen_y = 30,
    .centered = true,
    .color = tVec3f(1.f),
    .string = "Level Editor"
  });

  if (editor.is_in_placement_mode) {
    if (editor.is_placing_entity) {
      DisplayEntityPlacementLabels(tachyon, state);
    } else {
      DisplayObjectPlacementLabels(tachyon, state);
    }
  }

  if (editor.is_anything_selected) {
    std::string message = editor.current_selectable.is_entity
      ? "Entity selected"
      : "Object selected";

    Tachyon_DrawUIText(tachyon, state.debug_text, {
      .screen_x = tachyon->window_width / 2,
      .screen_y = 60,
      .centered = true,
      .color = tVec3f(1.f),
      .string = message
    });

    if (editor.current_selectable.is_entity) {
      DisplaySelectedEntityProperties(tachyon, state);
    } else {
      DisplaySelectedObjectProperties(tachyon, state);
    }
  }
}

void LevelEditor::OpenLevelEditor(Tachyon* tachyon, State& state) {
  auto& fx = tachyon->fx;
  auto& meshes = state.meshes;

  show_overlay_message("Entering editor");

  state.is_level_editor_open = true;

  // @todo factor
  objects(meshes.astrolabe_rear).disabled = true;
  objects(meshes.astrolabe_base).disabled = true;
  objects(meshes.astrolabe_plate).disabled = true;
  objects(meshes.astrolabe_fragment_ul).disabled = true;
  objects(meshes.astrolabe_fragment_ll).disabled = true;
  objects(meshes.astrolabe_ring).disabled = true;
  objects(meshes.astrolabe_hand).disabled = true;
  objects(meshes.target_reticle).disabled = true;

  // Disable all in-game entity objects, since we use placeholders in the editor
  for_all_entity_types() {
    auto& mesh_ids = EntityDispatcher::GetMeshes(state, type);

    for (auto mesh_id : mesh_ids) {
      objects(mesh_id).disabled = true;
    }
  }

  editor.selectables.clear();

  SpawnEntityPlaceholders(tachyon, state);
  TrackDecorativeObjects(tachyon, state);
  InitEditorCamera(tachyon, state);

  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  fx.accumulation_blur_factor = 0.f;
}

void LevelEditor::HandleLevelEditor(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    HandleCameraActions(tachyon, state, dt);
    HandleEditorActions(tachyon, state);
  }

  if (editor.is_anything_selected) {
    UpdateCurrentGizmo(tachyon, state);
  }

  if (editor.is_in_placement_mode) {
    UpdatePlacer(tachyon, state);
  }

  HandleUI(tachyon, state);
}

void LevelEditor::CloseLevelEditor(Tachyon* tachyon, State& state) {
  if (editor.is_editing_entity_properties) {
    // If we happened to hit the E key while editing entity properties,
    // ignore the hotkey behavior which normally closes the editor
    return;
  }

  auto& meshes = state.meshes;

  tachyon->use_high_visibility_mode = false;
  state.is_level_editor_open = false;

  show_overlay_message("Leaving editor");

  // @todo factor
  objects(meshes.astrolabe_rear).disabled = false;
  objects(meshes.astrolabe_base).disabled = false;
  objects(meshes.astrolabe_plate).disabled = false;
  objects(meshes.astrolabe_fragment_ul).disabled = false;
  objects(meshes.astrolabe_fragment_ll).disabled = false;
  objects(meshes.astrolabe_ring).disabled = false;
  objects(meshes.astrolabe_hand).disabled = false;
  objects(meshes.target_reticle).disabled = false;

  // Re-enable all in-game entity objects
  for_all_entity_types() {
    auto& mesh_ids = EntityDispatcher::GetMeshes(state, type);

    for (auto mesh_id : mesh_ids) {
      objects(mesh_id).disabled = false;
    }
  }

  if (editor.is_anything_selected) {
    DeselectCurrent(tachyon, state);
  } else {
    ProceduralGeneration::RebuildProceduralObjects(tachyon, state);
  }

  Items::SpawnItemObjects(tachyon, state);

  SaveLevelData(tachyon, state);
  RemoveEntityPlaceholders(tachyon, state);

  if (editor.is_in_placement_mode) {
    DestroyPlacer(tachyon, state);

    editor.is_in_placement_mode = false;
  }

  editor.selectables.clear();
}