#include "cosmodrone/hud_system.h"
#include "cosmodrone/target_system.h"

using namespace Cosmodrone;

static uint16 GetTargetInspectorMeshIndex(uint16 source_mesh, State& state) {
  auto& meshes = state.meshes;

  if (source_mesh == meshes.antenna_3) {
    return meshes.antenna_3_wireframe;
  }

  return meshes.antenna_3_wireframe;
}

static void RemoveTargetInspectorMeshes(Tachyon* tachyon, State& state) {
  #define remove_objects(mesh_index)\
    if (objects(mesh_index).total_active > 0) {\
      remove(objects(mesh_index)[0]);\
    }\

  auto& meshes = state.meshes;

  remove_objects(meshes.antenna_3_wireframe);
}

static void HandleOdometer(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto& wedge = objects(meshes.hud_wedge)[0];
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  wedge.position = camera.position + state.view_forward_direction * 550.f + left * (200.f + camera.fov * 3.f);
  wedge.scale = 150.f + camera.fov * 1.8f;
  wedge.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);

  wedge.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.3f);

  commit(wedge);
}

static void HandleTargetInspector(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto& wedge = objects(meshes.hud_wedge)[1];
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  wedge.position = camera.position + state.view_forward_direction * 550.f - left * (200.f + camera.fov * 3.f);
  wedge.scale = 150.f + camera.fov * 1.8f;
  wedge.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);

  wedge.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_PI * 1.3f) *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), t_PI);

  commit(wedge);

  auto* target = TargetSystem::GetSelectedTargetTracker(state);

  if (target != nullptr) {
    auto target_mesh_index = GetTargetInspectorMeshIndex(target->object.mesh_index, state);
    auto& objects = objects(target_mesh_index);

    if (objects.total_active == 0) {
      create(target_mesh_index);
    }

    auto& preview = objects[0];

    preview.scale = 50.f;
    preview.color = tVec3f(0.2f, 0.5f, 1.f);

    preview.position =
      wedge.position +
      state.view_forward_direction * 20.f +
      left * 40.f +
      state.view_up_direction * 110.f;

    preview.rotation =
      camera.rotation.opposite() *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f * sinf(state.current_game_time * 0.5f)) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), state.current_game_time);

    commit(preview);
  } else {
    RemoveTargetInspectorMeshes(tachyon, state);
  }
}

void HUDSystem::HandleHUD(Tachyon* tachyon, State& state, const float dt) {
  HandleOdometer(tachyon, state, dt);
  HandleTargetInspector(tachyon, state, dt);
}