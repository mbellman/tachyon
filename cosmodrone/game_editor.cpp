#include "cosmodrone/game_editor.h"
#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

constexpr static float PITCH_LIMIT = t_HALF_PI * 0.99f;

struct EditorState {
  bool show_object_picker = false;
  uint16 object_picker_index = 0;

  bool is_object_selected = false;
  float selected_object_distance = 4000.f;
  tObject selected_object;
} editor;

static const MeshAsset& GetSelectedObjectPickerMeshAsset() {
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& selected_mesh = placeable_meshes[editor.object_picker_index];

  return selected_mesh;
}

static void HandleCamera(Tachyon* tachyon, State& state, const float dt) {
  if (!is_window_focused()) {
    return;
  }

  auto& camera = tachyon->scene.camera;

  // Handle mouse movements
  {
    if (is_key_held(tKey::SHIFT) && editor.is_object_selected) {
      auto offset = camera.position - editor.selected_object.position;
      auto unit_offset = offset.unit();

      tCamera3p camera3p;
      camera3p.radius = editor.selected_object_distance;
      camera3p.azimuth = atan2f(unit_offset.z, unit_offset.x);
      camera3p.altitude = atan2f(unit_offset.y, unit_offset.xz().magnitude());

      if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) {
        camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
        camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
        camera3p.limitAltitude(0.99f);

        camera.position = editor.selected_object.position + camera3p.calculatePosition();

        camera.orientation.face(editor.selected_object.position - camera.position, tVec3f(0, 1.f, 0));
      }
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

static void HandleInputs(Tachyon* tachyon, State& state) {
  if (editor.show_object_picker) {
    auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
    auto max = placeable_meshes.size() - 1;

    if (did_wheel_up()) {
      if (editor.is_object_selected) {
        remove(editor.selected_object);
      }

      if (editor.object_picker_index == 0) {
        editor.object_picker_index = max;
      } else {
        editor.object_picker_index--;
      }
    }

    if (did_wheel_down()) {
      if (editor.is_object_selected) {
        remove(editor.selected_object);
      }

      if (editor.object_picker_index == max) {
        editor.object_picker_index = 0;
      } else {
        editor.object_picker_index++;
      }
    }
  }

  if (did_wheel_down() || did_wheel_up()) {
    auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
    auto mesh_index = selected_mesh.mesh_index;

    create(mesh_index);

    editor.show_object_picker = true;
  }
}

static void HandleObjectPicker(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& selected_mesh = GetSelectedObjectPickerMeshAsset();
  auto mesh_name = selected_mesh.mesh_name;
  auto mesh_index = selected_mesh.mesh_index;
  auto& instances = objects(mesh_index);

  if (instances.total_visible == 0) {
    create(mesh_index);
  }

  editor.is_object_selected = true;

  editor.selected_object = instances[instances.total_visible - 1];
  editor.selected_object.scale = tVec3f(1000.f);
  editor.selected_object.position = camera.position + camera.orientation.getDirection() * editor.selected_object_distance;
  editor.selected_object.color = tVec4f(1.f, 1.f, 1.f, uint32(tachyon->running_time * 2.f) % 2 == 0 ? 0.2f : 0.6f);

  if (is_window_focused() && did_press_mouse()) {
    editor.selected_object.color = tVec3f(1.f);

    editor.show_object_picker = false;
    editor.is_object_selected = false;
  }

  commit(editor.selected_object);

  // @todo refactor
  {
    objects(state.meshes.editor_position).disabled = false;

    auto& indicator = objects(state.meshes.editor_position)[0];

    indicator.position = camera.position + camera.orientation.getDirection() * 200.f;
    indicator.color = tVec4f(1.f, 1.f, 1.f, 1.f);
    indicator.scale = tVec3f(70.f);

    commit(indicator);
  }

  add_dev_label("Object", (
    mesh_name + " (" +
    std::to_string(instances.total_active) + " active)"
  ));
}

void Editor::InitializeEditor(Tachyon* tachyon, State& state) {
  create(state.meshes.editor_position);
  create(state.meshes.editor_rotation);
  create(state.meshes.editor_scale);
}

void Editor::HandleEditor(Tachyon* tachyon, State& state, const float dt) {
  HandleCamera(tachyon, state, dt);
  HandleInputs(tachyon, state);

  if (editor.show_object_picker) {
    HandleObjectPicker(tachyon, state);
  } else {
    // @todo refactor
    objects(state.meshes.editor_position).disabled = true;
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

  if (editor.show_object_picker && editor.is_object_selected) {
    remove(editor.selected_object);
  }

  editor.show_object_picker = false;
}