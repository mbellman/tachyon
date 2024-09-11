#include "cosmodrone/game_editor.h"
#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

static const float PITCH_LIMIT = (3.141592f / 2.f) * 0.99f;

struct EditorState {
  bool show_object_picker = false;
  uint16 object_picker_index = 0;

  tObject* selected_object = nullptr;
} editor;

static void HandleCamera(Tachyon* tachyon, State& state, const float dt) {
  if (!is_window_focused()) {
    return;
  }

  auto& camera = tachyon->scene.camera;

  // Handle mouse movements
  {
    camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
    camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;

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
      if (editor.object_picker_index == 0) {
        editor.object_picker_index = max;
      } else {
        editor.object_picker_index--;
      }
    }

    if (did_wheel_down()) {
      if (editor.object_picker_index == max) {
        editor.object_picker_index = 0;
      } else {
        editor.object_picker_index++;
      }
    }
  }

  if (did_wheel_down() || did_wheel_up()) {
    editor.show_object_picker = true;
  }
}

static void HandleObjectPicker(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& placeable_meshes = MeshLibrary::GetPlaceableMeshAssets();
  auto& selected_mesh = placeable_meshes[editor.object_picker_index];
  auto mesh_name = selected_mesh.mesh_name;
  auto mesh_index = selected_mesh.mesh_index;

  if (objects(mesh_index).total_visible == 0) {
    create(mesh_index);
  }

  auto& preview_object = objects(mesh_index)[0];

  preview_object.scale = tVec3f(1000.f);
  preview_object.position = camera.position + camera.orientation.getDirection() * 2000.f;

  commit(preview_object);

  add_dev_label("Object", mesh_name);
}

void Editor::HandleEditor(Tachyon* tachyon, State& state, const float dt) {
  HandleCamera(tachyon, state, dt);
  HandleInputs(tachyon, state);

  if (editor.show_object_picker) {
    HandleObjectPicker(tachyon, state);
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

  editor.show_object_picker = false;
}