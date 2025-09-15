#include "astro/level_editor.h"

using namespace astro;

static void InitEditorCamera(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  tVec3f forward = camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f);

  camera.orientation.face(forward, tVec3f(0, 1.f, 0));
}

static void HandleFreeCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  const float camera_panning_speed = 0.3f;
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
  }

  // @todo refactor
  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];
    auto& placeholder = create(meshes.oak_tree_placeholder);

    placeholder.position = entity.position;
    placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
    placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

    commit(placeholder);
  }

  // @todo refactor
  for_entities(state.willow_trees) {
    auto& entity = state.willow_trees[i];
    auto& placeholder = create(meshes.willow_tree_placeholder);

    placeholder.position = entity.position;
    placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
    placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

    commit(placeholder);
  }
}

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

  RemoveEntityPlaceholders(tachyon, state);
}

void LevelEditor::HandleLevelEditor(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    HandleFreeCamera(tachyon, state, dt);
  }
}