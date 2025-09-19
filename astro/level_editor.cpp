#include <format>
#include <map>
#include <string>
#include <vector>

#include "astro/level_editor.h"
#include "astro/data_loader.h"
#include "astro/entity_manager.h"
#include "astro/entity_dispatcher.h"
#include "astro/object_manager.h"

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
  bool is_object_selected = false;
  GizmoAction current_gizmo_action = POSITION;
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
    std::to_string(entity.astro_end_time)
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
 * Returns a list of all decorative mesh indexes.
 * ----------------------------
 */
std::vector<uint16>& GetDecorativeMeshes(State& state) {
  auto& meshes = state.meshes;

  static std::vector<uint16> decorative_meshes = {
    meshes.rock_1,
    meshes.ground_1
  };

  return decorative_meshes;
}

/**
 * ----------------------------
 * Writes all entities/objects in the level to a file.
 * ----------------------------
 */
void SaveLevelData(Tachyon* tachyon, State& state) {
  std::string level_data = "";

  for_all_entity_types() {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      level_data += SerializeEntity(entity) + "\n";
    }
  }

  for (auto mesh_index : GetDecorativeMeshes(state)) {
    auto& objects = objects(mesh_index);

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
  const float camera_movement_speed = is_key_held(tKey::SPACE) ? 10000.f : 5000.f;

  // Object swiveling or mouse panning
  {
    if (!is_left_mouse_held_down()) {
      if (is_key_held(tKey::SHIFT) && editor.is_object_selected) {
        HandleSelectedObjectCameraSwiveling(tachyon, state);
      } else {
        camera.orientation.yaw += tachyon->mouse_delta_x * camera_panning_speed * dt;
        camera.orientation.pitch += tachyon->mouse_delta_y * camera_panning_speed * dt;
      }
    }
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
    .entity_record = {
      entity.id,
      entity.type
    },
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

  auto& up = objects(state.meshes.gizmo_rotator)[0];
  auto& left = objects(state.meshes.gizmo_rotator)[1];
  auto& forward = objects(state.meshes.gizmo_rotator)[2];

  auto& current_object = editor.current_selectable.placeholder;

  up.rotation = current_object.rotation;
  left.rotation = current_object.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
  forward.rotation = current_object.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI);

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
 * Selects a given Selectable, either an entity or plain object.
 * ----------------------------
 */
static void MakeSelection(Tachyon* tachyon, State& state, Selectable& selectable) {
  editor.is_object_selected = true;
  editor.current_selectable = selectable;

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

  editor.is_object_selected = false;

  DestroyGizmo(tachyon, state);
  SyncSelectables(tachyon);
  SaveLevelData(tachyon, state);
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
    float distance_limit = 20000.f; // @todo base on placeholder scale
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
 * Deletes the currently-selected entity or object.
 * ----------------------------
 */
static void DeleteSelectedObject(Tachyon* tachyon, State& state) {
  auto& selected = editor.current_selectable;

  remove(selected.placeholder);

  if (selected.is_entity) {
    ForgetSelectableEntity(selected.entity_record.id);

    EntityManager::DeleteEntity(state, selected.entity_record);
    EntityDispatcher::DestroyObjects(tachyon, state, selected.entity_record.type);
  } else {
    ForgetSelectableObject(selected.placeholder);
  }

  DestroyGizmo(tachyon, state);

  editor.is_object_selected = false;
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
 * Handler for moving selected objects around.
 * ----------------------------
 */
static void HandleSelectedObjectPositionActions(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& placeholder = editor.current_selectable.placeholder;
  float move_speed = (placeholder.position - camera.position).magnitude() / 4000.f;

  if (abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y)) {
    tVec3f camera_left = camera.orientation.getLeftDirection();
    tVec3f move_axis = GetClosestWorldAxis(camera_left);

    placeholder.position -= move_axis * move_speed * (float)tachyon->mouse_delta_x;
  } else {
    tVec3f move_axis = tVec3f(0, 1.f, 0);

    placeholder.position -= move_axis * move_speed * (float)tachyon->mouse_delta_y;
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
    tVec3f scale_axis = GetClosestWorldAxis(camera_left);

    // Ensure dragging the mouse right or left always scales in the correct direction,
    // regardless of which direction we're looking along X or Z
    if (scale_axis.x == 1.f) scale_axis.x = -1.f;
    if (scale_axis.z == 1.f) scale_axis.z = -1.f;

    placeholder.scale -= scale_axis * scale_speed * (float)tachyon->mouse_delta_x;
  } else {
    tVec3f scale_axis = tVec3f(0, 1.f, 0);

    placeholder.scale -= scale_axis * scale_speed * (float)tachyon->mouse_delta_y;
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
    // Find and sync the scale of the original entity
    auto* entity = EntityManager::FindEntity(state, editor.current_selectable.entity_record);

    entity->orientation = placeholder.rotation;
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
 * Creates a normal object and adds it to the scene.
 * ----------------------------
 */
static void CreateDecorativeObject(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  // @temporary
  {
    auto& rock = create(state.meshes.rock_1);

    rock.position = camera.position + camera.orientation.getDirection() * 5000.f;
    rock.scale = 500.f;
    rock.color = tVec3f(0.2f);

    commit(rock);

    TrackDecorativeObject(rock);
  }

  // @temporary
  {
    auto& ground = create(state.meshes.ground_1);

    ground.position = camera.position + camera.orientation.getDirection() * 8000.f;
    ground.scale = 2000.f;
    ground.color = tVec3f(0.4f, 0.7f, 0.2f);

    commit(ground);

    TrackDecorativeObject(ground);
  }
}

/**
 * ----------------------------
 * Creates an entity and adds it to the scene.
 * ----------------------------
 */
static void CreateEntity(Tachyon* tachyon, State& state) {
  // @todo
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

  if (editor.is_object_selected) {
    if (is_left_mouse_held_down()) {
      HandleSelectedObjectActions(tachyon, state);
    }

    if (did_press_key(tKey::BACKSPACE)) {
      DeleteSelectedObject(tachyon, state);
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
  }

  if (did_press_key(tKey::ENTER)) {
    CreateDecorativeObject(tachyon, state);
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
      auto& placeholder = EntityDispatcher::CreatePlaceholder(tachyon, state, entity);

      TrackSelectableEntity(entity, placeholder);
    }
  }
}

/**
 * ----------------------------
 * Puts all decorative objects in the selectables list.
 * ----------------------------
 */
static void TrackDecorativeObjects(Tachyon* tachyon, State& state) {
  for (auto mesh_index : GetDecorativeMeshes(state)) {
    for (auto& object : objects(mesh_index)) {
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
    EntityDispatcher::DestroyPlaceholders(tachyon, state, type);
  }
}

void LevelEditor::OpenLevelEditor(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.is_level_editor_open = true;

  show_alert_message("Entering editor");

  objects(meshes.astrolabe_base).disabled = true;
  objects(meshes.astrolabe_ring).disabled = true;
  objects(meshes.astrolabe_hand).disabled = true;

  // @todo iterate over entity types + meshes for that entity
  objects(meshes.oak_tree_trunk).disabled = true;
  objects(meshes.willow_tree_trunk).disabled = true;
  objects(meshes.shrub_branches).disabled = true;

  editor.selectables.clear();

  SpawnEntityPlaceholders(tachyon, state);
  TrackDecorativeObjects(tachyon, state);
  InitEditorCamera(tachyon, state);
}

void LevelEditor::CloseLevelEditor(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.is_level_editor_open = false;

  show_alert_message("Leaving editor");

  objects(meshes.astrolabe_base).disabled = false;
  objects(meshes.astrolabe_ring).disabled = false;
  objects(meshes.astrolabe_hand).disabled = false;

  // @todo iterate over entity types + meshes for that entity
  objects(meshes.oak_tree_trunk).disabled = false;
  objects(meshes.willow_tree_trunk).disabled = false;
  objects(meshes.shrub_branches).disabled = false;

  if (editor.is_object_selected) {
    DeselectCurrent(tachyon, state);
  }

  SaveLevelData(tachyon, state);
  RemoveEntityPlaceholders(tachyon, state);

  editor.selectables.clear();
}

void LevelEditor::HandleLevelEditor(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    HandleCameraActions(tachyon, state, dt);
    HandleEditorActions(tachyon, state);
  }

  if (editor.is_object_selected) {
    UpdateCurrentGizmo(tachyon, state);
  }

  // @todo create a HandleUI() method
  Tachyon_DrawUIText(tachyon, state.debug_text_large, {
    .screen_x = tachyon->window_width / 2,
    .screen_y = 30,
    .centered = true,
    .color = tVec3f(1.f),
    .string = "Level Editor"
  });

  if (editor.is_object_selected) {
    // @temporary
    Tachyon_DrawUIText(tachyon, state.debug_text, {
      .screen_x = tachyon->window_width / 2,
      .screen_y = 60,
      .centered = true,
      .color = tVec3f(1.f),
      .string = "Object selected"
    });
  }
}