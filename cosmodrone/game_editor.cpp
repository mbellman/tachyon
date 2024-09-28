#include <format>
#include <math.h>

#include "cosmodrone/game_editor.h"
#include "cosmodrone/mesh_library.h"

#define case(value, __code)\
  case value: {\
    __code\
    break;\
  }\

using namespace Cosmodrone;

constexpr static float PITCH_LIMIT = t_HALF_PI * 0.99f;

enum ActionType {
  POSITION,
  ROTATE,
  SCALE
};

struct EditorState {
  bool is_object_picker_active = false;
  uint16 object_picker_index = 0;

  bool use_modified_action = false;
  float running_angle_x = 0.f;
  float running_angle_y = 0.f;

  bool is_object_selected = false;
  ActionType action_type = ActionType::POSITION;
  tObject selected_object;
} editor;

static const MeshAsset& GetSelectedObjectPickerMeshAsset() {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& selected_mesh = placeable_meshes[editor.object_picker_index];

  return selected_mesh;
}

static tVec3f GetMostSimilarGlobalAxis(const tVec3f& vector) {
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

static tVec3f GetMostSimilarObjectAxis(const tVec3f& vector, const tObject& object) {
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

static void HandleCamera(Tachyon* tachyon, State& state, const float dt) {
  if (!is_window_focused()) {
    return;
  }

  if (editor.is_object_selected && is_mouse_held_down()) {
    return;
  }

  auto& camera = tachyon->scene.camera;
  float slerp_alpha = 30.f * dt;

  if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0 || slerp_alpha > 1.f) {
    slerp_alpha = 1.f;
  }

  // Handle mouse movements
  {
    if (is_key_held(tKey::SHIFT) && editor.is_object_selected) {
      auto offset = camera.position - editor.selected_object.position;
      auto unit_offset = offset.unit();

      tCamera3p camera3p;
      camera3p.radius = offset.magnitude();
      camera3p.azimuth = atan2f(unit_offset.z, unit_offset.x);
      camera3p.altitude = atan2f(unit_offset.y, unit_offset.xz().magnitude());

      if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) {
        camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
        camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
        camera3p.limitAltitude(0.99f);

        camera.position = editor.selected_object.position + camera3p.calculatePosition();
      }

      camera.orientation.face(editor.selected_object.position - camera.position, tVec3f(0, 1.f, 0));
    } else {
      camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
      camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;
    }

    if (camera.orientation.pitch > PITCH_LIMIT) camera.orientation.pitch = PITCH_LIMIT;
    else if (camera.orientation.pitch < -PITCH_LIMIT) camera.orientation.pitch = -PITCH_LIMIT;
  }

  // Handle WASD controls
  {
    const float speed = is_key_held(tKey::SPACE) ? 50000.f : 2000.f;

    if (is_key_held(tKey::W)) {
      camera.position += camera.orientation.getDirection() * dt * speed;
    } else if (is_key_held(tKey::S)) {
      camera.position += camera.orientation.getDirection() * -dt * speed;
    }

    if (is_key_held(tKey::A)) {
      camera.position += camera.orientation.getLeftDirection() * dt * speed;
    } else if (is_key_held(tKey::D)) {
      camera.position += camera.orientation.getRightDirection() * dt * speed;
    }
  }

  camera.rotation = Quaternion::slerp(camera.rotation, camera.orientation.toQuaternion(), slerp_alpha);
}

static void HandleObjectPickerInputs(Tachyon* tachyon) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto max = placeable_meshes.size() - 1;

  if (did_press_key(tKey::Q)) {
    // Cycle object picker left
    if (editor.object_picker_index == 0) {
      editor.object_picker_index = max;
    } else {
      editor.object_picker_index--;
    }
  }

  if (did_press_key(tKey::E)) {
    // Cycle object picker right
    if (editor.object_picker_index == max) {
      editor.object_picker_index = 0;
    } else {
      editor.object_picker_index++;
    }
  }
}

static void HandleObjectPickerCycleChange(Tachyon* tachyon) {
  auto& camera = tachyon->scene.camera;

  tVec3f spawn_position = camera.position + camera.orientation.getDirection() * 4000.f;

  if (editor.is_object_picker_active && editor.is_object_selected) {
    spawn_position = get_original_object(editor.selected_object)->position;

    remove(editor.selected_object);
  }

  if (!editor.is_object_picker_active) {
    editor.is_object_picker_active = true;
  }

  auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
  auto mesh_index = selected_mesh.mesh_index;
  auto& selected = create(mesh_index);

  selected.position = spawn_position;
  selected.scale = selected_mesh.defaults.scale;
  selected.rotation = Quaternion(1.f, 0, 0, 0);
  selected.color = selected_mesh.defaults.color;
  selected.material = selected_mesh.defaults.material;

  editor.selected_object = selected;
  editor.is_object_selected = true;
}

static void HandleActionTypeCycleChange(Tachyon* tachyon, int8 change) {
  if (change > 0) {
    switch (editor.action_type) {
      case(ActionType::POSITION, {
        editor.action_type = ActionType::ROTATE;
      })
      case(ActionType::ROTATE, {
        editor.action_type = ActionType::SCALE;
      })
      case(ActionType::SCALE, {
        editor.action_type = ActionType::POSITION;
      })
    }
  } else {
    switch (editor.action_type) {
      case(ActionType::POSITION, {
        editor.action_type = ActionType::SCALE;
      })
      case(ActionType::ROTATE, {
        editor.action_type = ActionType::POSITION;
      })
      case(ActionType::SCALE, {
        editor.action_type = ActionType::ROTATE;
      })
    }
  }
}

static void HandleRotationSnapping(float& running_angle, float& angle) {
  constexpr static float SNAP_INCREMENT = t_PI / 12.f;

  if (abs(running_angle) > SNAP_INCREMENT) {
    angle = running_angle > 0.f ? SNAP_INCREMENT : -SNAP_INCREMENT;

    running_angle = 0.f;
  } else {
    angle = 0.f;
  }
}

static void HandleSelectedObjectMouseAction(Tachyon* tachyon) {
  auto& camera = tachyon->scene.camera;
  auto& selected = *get_original_object(editor.selected_object);
  auto is_horizontal_action = abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y);
  auto camera_up = camera.orientation.getUpDirection();
  auto camera_right = camera.orientation.getRightDirection();

  switch (editor.action_type) {
    case(ActionType::POSITION, {
      auto distance = (selected.position - camera.position).magnitude();
      auto movement_factor = distance / 5000.f;
      auto use_object_axis = editor.use_modified_action;

      if (is_key_held(tKey::CONTROL)) {
        // Hold CONTROL for fine repositioning
        movement_factor *= 0.1f;
      }

      if (is_horizontal_action) {
        auto axis = use_object_axis
          ? GetMostSimilarObjectAxis(camera_right, selected)
          : GetMostSimilarGlobalAxis(camera_right);

        selected.position += axis * (float)tachyon->mouse_delta_x * movement_factor;
      } else {
        auto axis = use_object_axis
          ? GetMostSimilarObjectAxis(camera_up, selected)
          : tVec3f(0, 1.f, 0);

        selected.position -= axis * (float)tachyon->mouse_delta_y * movement_factor;
      }
    })
    case(ActionType::ROTATE, {
      constexpr static float SNAP_INCREMENT = t_PI / 12.f;
      auto use_snapping = editor.use_modified_action;

      if (is_horizontal_action) {
        auto axis = GetMostSimilarObjectAxis(camera_up, selected);
        auto angle = (float)tachyon->mouse_delta_x * 0.005f;

        editor.running_angle_x += angle;

        if (use_snapping) {
          HandleRotationSnapping(editor.running_angle_x, angle);
        }

        selected.rotation *= Quaternion::fromAxisAngle(axis, angle);
      } else {
        auto axis = GetMostSimilarObjectAxis(camera_right, selected);
        auto angle = (float)tachyon->mouse_delta_y * 0.005f;

        editor.running_angle_y += angle;

        if (use_snapping) {
          HandleRotationSnapping(editor.running_angle_y, angle);
        }

        selected.rotation *= Quaternion::fromAxisAngle(axis, angle);
      }
    })
    case(ActionType::SCALE, {
      // @todo
    })
  }
}

static void ResetSelectedObject(Tachyon* tachyon) {
  auto& selected = *get_original_object(editor.selected_object);

  switch (editor.action_type) {
    case(ActionType::ROTATE, {
      selected.rotation = Quaternion(1.f, 0, 0, 0);
    })
    case(ActionType::SCALE, {
      selected.scale = tVec3f(1000.f);
    })
  }
}

static void RestoreSelectedObject(Tachyon* tachyon, tObject& object) {
  // @todo restore base color (editor.selected_object.color?)
  object.color = tVec3f(1.f);

  commit(object);
}

enum Direction {
  UP, RIGHT, LEFT, DOWN
};

static void CopySelectedObject(Tachyon* tachyon, Direction direction) {
  auto& camera = tachyon->scene.camera;
  auto& selected = *get_original_object(editor.selected_object);
  auto& copy = create(selected.mesh_index);

  RestoreSelectedObject(tachyon, selected);

  copy.rotation = selected.rotation;
  copy.scale = selected.scale;
  copy.color = selected.color;
  copy.material = selected.material;

  tVec3f axis;

  switch (direction) {
    case(LEFT, {
      axis = GetMostSimilarObjectAxis(camera.orientation.getLeftDirection(), selected);
    })
    case(RIGHT, {
      axis = GetMostSimilarObjectAxis(camera.orientation.getRightDirection(), selected);
    })
    case(UP, {
      axis = GetMostSimilarObjectAxis(camera.orientation.getUpDirection(), selected);
    })
    case(DOWN, {
      axis = GetMostSimilarObjectAxis(camera.orientation.getUpDirection().invert(), selected);
    })
  }

  copy.position = selected.position + axis * copy.scale;

  commit(copy);

  editor.selected_object = copy;
}

// @todo improve accuracy using collision planes/scale
static void MaybeSelectObject(Tachyon* tachyon) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& camera = tachyon->scene.camera;
  auto forward = camera.orientation.getDirection();
  float highest_dot = -1.f;
  float closest_distance = 3.402823466e+38F;
  tObject candidate;

  for (auto& mesh : placeable_meshes) {
    auto& instances = objects(mesh.mesh_index);

    if (instances.disabled) {
      continue;
    }

    for (auto& object : instances) {
      auto camera_to_object = object.position - camera.position;
      auto object_dot = tVec3f::dot(forward, camera_to_object.unit());
      auto distance = camera_to_object.magnitude();

      if (distance > 30000.f) continue;

      if (object_dot > highest_dot) {
        highest_dot = object_dot;
        closest_distance = distance;
        candidate = object;
      }
    }
  }

  if (highest_dot > 0.f) {
    editor.is_object_selected = true;
    editor.selected_object = candidate;
  }
}

static inline std::string Serialize(float f) {
  return std::format("{:.3f}", f);
}

static inline std::string Serialize(const tVec3f& vector) {
  return Serialize(vector.x) + "," + Serialize(vector.y) + "," + Serialize(vector.z);
}

static inline std::string Serialize(const Quaternion& quaternion) {
  return (
    Serialize(quaternion.w) + "," +
    Serialize(quaternion.x) + "," +
    Serialize(quaternion.y) + "," +
    Serialize(quaternion.z)
  );
}

static void SaveWorldData(Tachyon* tachyon) {
  auto start = Tachyon_GetMicroseconds();
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  std::string data = "";

  for (auto& mesh : placeable_meshes) {
    auto& instances = objects(mesh.mesh_index);

    data += ("@" + mesh.mesh_name + "\n");

    // Perform a sequence-preserving loop over the objects
    // to ensure they always serialize in the same order
    // (e.g. if objects are removed/shuffled during runtime)
    for (uint16 id = 0; id <= instances.highest_used_id; id++) {
      tObject* instance = instances.getById(id);

      if (instance != nullptr) {
        data += Serialize(instance->position) + ",";
        data += Serialize(instance->scale) + ",";
        data += Serialize(instance->rotation) + ",";
        data += std::to_string(instance->color.rgba) + "\n";
      }
    }
  }

  Tachyon_WriteFileContents("./cosmodrone/data/world.txt", data);

  auto save_time = Tachyon_GetMicroseconds() - start;
  auto message = std::format("Saved world data in {}us", save_time);

  add_console_message(message, tVec3f(1.f));
}

static void HandleInputs(Tachyon* tachyon, State& state) {
  if (editor.is_object_picker_active) {
    HandleObjectPickerInputs(tachyon);
  }

  if (did_press_key(tKey::Q) || did_press_key(tKey::E)) {
    HandleObjectPickerCycleChange(tachyon);
  }

  if (did_wheel_down()) {
    HandleActionTypeCycleChange(tachyon, +1);
  } else if (did_wheel_up()) {
    HandleActionTypeCycleChange(tachyon, -1);
  }

  if (editor.is_object_selected) {
    if (is_mouse_held_down()) {
      HandleSelectedObjectMouseAction(tachyon);
    }

    if (did_press_key(tKey::ENTER)) {
      ResetSelectedObject(tachyon);
    }

    if (did_press_key(tKey::BACKSPACE)) {
      remove(editor.selected_object);

      editor.is_object_selected = false;
      editor.is_object_picker_active = false;

      SaveWorldData(tachyon);
    }

    if (did_press_key(tKey::ARROW_LEFT)) {
      CopySelectedObject(tachyon, LEFT);
    }

    if (did_press_key(tKey::ARROW_RIGHT)) {
      CopySelectedObject(tachyon, RIGHT);
    }

    if (did_press_key(tKey::ARROW_UP)) {
      CopySelectedObject(tachyon, UP);
    }

    if (did_press_key(tKey::ARROW_DOWN)) {
      CopySelectedObject(tachyon, DOWN);
    }
  }

  if (did_left_click_down() && !editor.is_object_selected) {
    MaybeSelectObject(tachyon);
  }

  if (did_press_key(tKey::C)) {
    editor.use_modified_action = !editor.use_modified_action;
  }

  if (did_press_key(tKey::G)) {
    objects(state.meshes.editor_guideline).disabled = !objects(state.meshes.editor_guideline).disabled;
  }

  if (did_press_key(tKey::ENTER) && !editor.is_object_selected) {
    SaveWorldData(tachyon);
  }
}

static void HandleObjectPicker(Tachyon* tachyon, State& state) {
  auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
  auto mesh_index = selected_mesh.mesh_index;
  auto& instances = objects(mesh_index);

  editor.selected_object = instances[instances.total_visible - 1];

  add_dev_label("Object", (
    selected_mesh.mesh_name + " (" +
    std::to_string(instances.total_active) + " active)"
  ));
}

static void HandleSelectedObject(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& selected = *get_original_object(editor.selected_object);

  selected.color = tVec4f(1.f, 1.f, 1.f, uint32(tachyon->running_time * 2.f) % 2 == 0 ? 0.1f : 0.2f);

  // @todo refactor
  {
    objects(state.meshes.editor_position).disabled = true;
    objects(state.meshes.editor_rotation).disabled = true;
    objects(state.meshes.editor_scale).disabled = true;

    if (editor.action_type == ActionType::POSITION) {
      objects(state.meshes.editor_position).disabled = false;

      auto& indicator = objects(state.meshes.editor_position)[0];
      auto selected_object_direction = (selected.position - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.scale = tVec3f(90.f);

      if (editor.use_modified_action) {
        indicator.rotation = selected.rotation;
        indicator.color = tVec4f(1.f, 1.f, 0.f, 1.f);
      } else {
        indicator.rotation = Quaternion(1.f, 0, 0, 0);
        indicator.color = tVec4f(1.f);
      }

      commit(indicator);
    } else if (editor.action_type == ActionType::ROTATE) {
      objects(state.meshes.editor_rotation).disabled = false;

      auto& indicator = objects(state.meshes.editor_rotation)[0];
      auto selected_object_direction = (selected.position - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.rotation = selected.rotation;
      indicator.scale = tVec3f(90.f);

      if (editor.use_modified_action) {
        indicator.color = tVec4f(1.f, 1.f, 0.f, 1.f);
      } else {
        indicator.color = tVec4f(1.f);
      }

      commit(indicator);
    } else if (editor.action_type == ActionType::SCALE) {
      objects(state.meshes.editor_scale).disabled = false;

      auto& indicator = objects(state.meshes.editor_scale)[0];
      auto selected_object_direction = (selected.position - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 600.f;
      indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
      indicator.rotation = selected.rotation;
      indicator.scale = tVec3f(90.f);

      commit(indicator);
    }
  }

  // @todo move to HandleInputs()
  if (did_right_click_down()) {
    // Deselect the object
    RestoreSelectedObject(tachyon, selected);

    editor.is_object_selected = false;
    editor.is_object_picker_active = false;

    SaveWorldData(tachyon);
  }

  commit(selected);

  editor.selected_object = selected;

  add_dev_label("Position", selected.position.toString());
  add_dev_label("Rotation", selected.rotation.toString());
}

static void HandleGuidelines(Tachyon* tachyon, State& state) {
  if (objects(state.meshes.editor_guideline).disabled) {
    return;
  }

  auto& camera = tachyon->scene.camera;

  for (auto& guideline : objects(state.meshes.editor_guideline)) {
    float distance_x = abs(guideline.position.x - camera.position.x);
    float distance_y = abs(guideline.position.y - camera.position.y);
    float distance_z = abs(guideline.position.z - camera.position.z);
    float brightness;

    if (guideline.scale.y > guideline.scale.x) {
      // Vertical line
      float scale = 20.f + (distance_x + distance_z) / 2000.f;
      brightness = 1.f - (distance_x + distance_z) / 1000000.f;

      guideline.scale = tVec3f(scale, guideline.scale.y, scale);
    } else if (guideline.scale.x > guideline.scale.z) {
      // Horizontal x
      float scale = 20.f + (distance_y + distance_z) / 2000.f;
      brightness = 1.f - (distance_y + distance_z) / 1000000.f;

      guideline.scale = tVec3f(guideline.scale.x, scale, scale);
    } else {
      // Horizontal z
      float scale = 20.f + (distance_x + distance_y) / 2000.f;
      brightness = 1.f - (distance_y + distance_x) / 1000000.f;

      guideline.scale = tVec3f(scale, scale, guideline.scale.z);
    }

    if (brightness < 0.f) brightness = 0.f;
    brightness *= brightness;
    brightness *= brightness;
    if (brightness < 0.1f) brightness = 0.1f;

    float emissive = brightness * 0.5f;
    if (emissive < 0.1f) emissive = 0.1f;

    guideline.color = tVec4f(brightness, 0, 0, emissive);

    commit(guideline);
  }
}

void Editor::InitializeEditor(Tachyon* tachyon, State& state) {
  create(state.meshes.editor_position);
  create(state.meshes.editor_rotation);
  create(state.meshes.editor_scale);
}

void Editor::HandleEditor(Tachyon* tachyon, State& state, const float dt) {
  HandleCamera(tachyon, state, dt);
  HandleInputs(tachyon, state);

  if (editor.is_object_picker_active) {
    HandleObjectPicker(tachyon, state);
  }

  if (editor.is_object_selected) {
    HandleSelectedObject(tachyon, state);
  } else {
    objects(state.meshes.editor_position).disabled = true;
    objects(state.meshes.editor_rotation).disabled = true;
    objects(state.meshes.editor_scale).disabled = true;
  }

  HandleGuidelines(tachyon, state);
}

void Editor::EnableEditor(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto forward = state.ship_position - camera.position;

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));

  objects(state.meshes.editor_guideline).disabled = false;

  tachyon->show_developer_tools = true;

  state.is_editor_active = true;
}

void Editor::DisableEditor(Tachyon* tachyon, State& state) {
  if (editor.is_object_selected) {
    if (editor.is_object_picker_active) {
      remove(editor.selected_object);
    } else {
      auto& selected = *get_original_object(editor.selected_object);

      RestoreSelectedObject(tachyon, selected);
    }
  }

  editor.is_object_picker_active = false;
  editor.is_object_selected = false;

  objects(state.meshes.editor_position).disabled = true;
  objects(state.meshes.editor_rotation).disabled = true;
  objects(state.meshes.editor_scale).disabled = true;

  state.is_editor_active = false;
}