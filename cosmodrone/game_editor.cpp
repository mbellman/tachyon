#include <math.h>

#include "cosmodrone/game_editor.h"
#include "cosmodrone/mesh_library.h"

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
  auto abs_x = abs(vector.x);
  auto abs_y = abs(vector.y);
  auto abs_z = abs(vector.z);

  if (abs_x > abs_y && abs_x > abs_z) {
    return tVec3f(vector.x, 0, 0).unit();
  } else if (abs_y > abs_x && abs_y > abs_z) {
    return tVec3f(0, vector.y, 0).unit();
  } else {
    return tVec3f(0, 0, vector.z).unit();
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

  camera.rotation = camera.orientation.toQuaternion();
}

static void HandleObjectPickerInputs(Tachyon* tachyon) {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto max = placeable_meshes.size() - 1;

  if (did_press_key(tKey::ARROW_LEFT)) {
    // Cycle object picker left
    if (editor.object_picker_index == 0) {
      editor.object_picker_index = max;
    } else {
      editor.object_picker_index--;
    }
  }

  if (did_press_key(tKey::ARROW_RIGHT)) {
    // Cycle object picker right
    if (editor.object_picker_index == max) {
      editor.object_picker_index = 0;
    } else {
      editor.object_picker_index++;
    }
  }
}

static void HandleObjectPickerCycleChange(Tachyon* tachyon) {
  if (editor.is_object_picker_active && editor.is_object_selected) {
    remove(editor.selected_object);
  }

  if (!editor.is_object_picker_active) {
    editor.is_object_picker_active = true;
  }

  auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
  auto mesh_index = selected_mesh.mesh_index;
  auto& selected = create(mesh_index);
  auto& camera = tachyon->scene.camera;

  selected.position = camera.position + camera.orientation.getDirection() * 4000.f;

  editor.selected_object = selected;
  editor.is_object_selected = true;
}

static void HandleActionTypeCycleChange(Tachyon* tachyon, int8 change) {
  if (change > 0) {
    switch (editor.action_type) {
      case ActionType::POSITION:
        editor.action_type = ActionType::ROTATE;
        break;
      case ActionType::ROTATE:
        editor.action_type = ActionType::SCALE;
        break;
      case ActionType::SCALE:
        editor.action_type = ActionType::POSITION;
        break;
    }
  } else {
    switch (editor.action_type) {
      case ActionType::POSITION:
        editor.action_type = ActionType::SCALE;
        break;
      case ActionType::ROTATE:
        editor.action_type = ActionType::POSITION;
        break;
      case ActionType::SCALE:
        editor.action_type = ActionType::ROTATE;
        break;
    }
  }
}

static void HandleSelectedObjectMouseAction(Tachyon* tachyon) {
  auto& camera = tachyon->scene.camera;
  auto& selected = *get_original_object(editor.selected_object);

  if (editor.action_type == ActionType::POSITION) {
    auto axis = GetMostSimilarGlobalAxis(camera.orientation.getRightDirection());

    if (abs(tachyon->mouse_delta_x) > abs(tachyon->mouse_delta_y)) {
      selected.position += axis * (float)tachyon->mouse_delta_x;
    } else {
      selected.position -= tVec3f(0, 1.f, 0) * (float)tachyon->mouse_delta_y;
    }
  }
}

static void HandleInputs(Tachyon* tachyon, State& state) {
  if (editor.is_object_picker_active) {
    HandleObjectPickerInputs(tachyon);
  }

  if (did_press_key(tKey::ARROW_LEFT) || did_press_key(tKey::ARROW_RIGHT)) {
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

  selected.scale = tVec3f(1000.f);
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

      indicator.position = camera.position + selected_object_direction * 150.f;
      indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
      indicator.rotation = selected.rotation;
      indicator.scale = tVec3f(40.f);

      commit(indicator);
    } else if (editor.action_type == ActionType::ROTATE) {
      objects(state.meshes.editor_rotation).disabled = false;

      auto& indicator = objects(state.meshes.editor_rotation)[0];
      auto selected_object_direction = (selected.position - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 150.f;
      indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
      indicator.rotation = selected.rotation;
      indicator.scale = tVec3f(40.f);

      commit(indicator);
    } else if (editor.action_type == ActionType::SCALE) {
      objects(state.meshes.editor_scale).disabled = false;

      auto& indicator = objects(state.meshes.editor_scale)[0];
      auto selected_object_direction = (selected.position - camera.position).unit();

      indicator.position = camera.position + selected_object_direction * 150.f;
      indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
      indicator.rotation = selected.rotation;
      indicator.scale = tVec3f(40.f);

      commit(indicator);
    }
  }

  if (did_right_click_down()) {
    // Deselect the object
    selected.color = tVec3f(1.f);

    editor.is_object_selected = false;
    editor.is_object_picker_active = false;
  }

  commit(selected);

  editor.selected_object = selected;

  add_dev_label("Position", selected.position.toString());
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
}

void Editor::EnableEditor(Tachyon* tachyon, State& state) {
  state.is_editor_active = true;

  auto& camera = tachyon->scene.camera;
  auto forward = state.ship_position - camera.position;

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));
}

void Editor::DisableEditor(Tachyon* tachyon, State& state) {
  state.is_editor_active = false;

  if (editor.is_object_picker_active && editor.is_object_selected) {
    remove(editor.selected_object);
  }

  editor.is_object_picker_active = false;
  editor.is_object_selected = false;
}