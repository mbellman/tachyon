#include "metro/world_editor.h"
#include "metro/editor_utilities.h"

using namespace metro;

static enum SelectionType {
  NOTHING_SELECTED = -1,
  BICYCLE,
  STATIC_ENTITY,
  INTERACTIVE_ENTITY
};

static struct EditorState {
  SelectionType selection_type = NOTHING_SELECTED;
  void* selection = nullptr;
} editor;

static bool IsAnythingSelected() {
  return editor.selection_type != NOTHING_SELECTED;
}

static void Deselect() {
  editor.selection_type = NOTHING_SELECTED;
  editor.selection = nullptr;
}

static void HandleFreeCameraControls(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_forward = camera.orientation.getDirection();
  tVec3f camera_left = camera.orientation.getLeftDirection();

  // WASD movement
  {
    if (is_key_held(tKey::W)) {
      camera.position += camera_forward * 15000.f * state.dt;
    }

    if (is_key_held(tKey::A)) {
      camera.position += camera_left * 15000.f * state.dt;
    }

    if (is_key_held(tKey::S)) {
      camera.position -= camera_forward * 15000.f * state.dt;
    }

    if (is_key_held(tKey::D)) {
      camera.position -= camera_left * 15000.f * state.dt;
    }
  }

  // Mouse movement
  {
    const float camera_mouse_speed = 0.2f;

    camera.orientation.yaw += tachyon->mouse_delta_x * camera_mouse_speed * state.dt;
    camera.orientation.pitch += tachyon->mouse_delta_y * camera_mouse_speed * state.dt;

    camera.rotation = camera.orientation.toQuaternion();
  }
}

static void HandleClickActions(Tachyon* tachyon, State& state) {
  if (did_left_click_down()) {
    // @temporary
    editor.selection_type = BICYCLE;
  }

  if (did_right_click_down()) {
    Deselect();
  }
}

static void ShowSelectionGizmo(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  tVec3f p = camera.position + camera.orientation.getDirection() * 2000.f;

  EditorUtilities::ShowPositionGizmo(tachyon, state, p);
}

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

  show_overlay_message("Entering editor");
}

void WorldEditor::Update(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  if (is_window_focused()) {
    HandleFreeCameraControls(tachyon, state);
    HandleClickActions(tachyon, state);

    if (IsAnythingSelected()) {
      ShowSelectionGizmo(tachyon, state);
    }
  }
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  Deselect();

  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}