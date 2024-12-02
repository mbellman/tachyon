#include "cosmodrone/hud_system.h"

using namespace Cosmodrone;

static void HandleOdometer(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto& wedge = objects(meshes.hud_wedge)[0];
  tVec3f left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  wedge.position = camera.position + state.view_forward_direction * 550.f + left * 340.f;
  wedge.scale = 150.f + camera.fov * 1.8f;
  wedge.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);
  wedge.rotation = camera.rotation.opposite() * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.3f);

  commit(wedge);
}

void HUDSystem::HandleHUD(Tachyon* tachyon, State& state, const float dt) {
  HandleOdometer(tachyon, state, dt);
}