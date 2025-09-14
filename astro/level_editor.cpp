#include "astro/level_editor.h"

using namespace astro;

static void InitEditorCamera(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f forward = camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f);

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));
}

static void HandleFreeCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  const float camera_panning_speed = 0.5f;
  const float camera_movement_speed = is_key_held(tKey::SPACE) ? 10000.f : 5000.f;

  // Mouse movement
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

void LevelEditor::OpenLevelEditor(Tachyon* tachyon, State& state) {
  state.is_level_editor_open = true;

  objects(state.meshes.astrolabe_base).disabled = true;
  objects(state.meshes.astrolabe_ring).disabled = true;
  objects(state.meshes.astrolabe_hand).disabled = true;

  InitEditorCamera(tachyon, state);
}

void LevelEditor::CloseLevelEditor(Tachyon* tachyon, State& state) {
  state.is_level_editor_open = false;

  objects(state.meshes.astrolabe_base).disabled = false;
  objects(state.meshes.astrolabe_ring).disabled = false;
  objects(state.meshes.astrolabe_hand).disabled = false;
}

void LevelEditor::HandleLevelEditor(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    HandleFreeCamera(tachyon, state, dt);
  }
}