#include "astro/hud_system.h"

using namespace astro;

static void UpdateHealthBar(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Frame
  {
    auto& bar = objects(meshes.health_bar)[0];

    bar.position = (
      camera.position +
      camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f) * 2000.f +
      camera.rotation.getLeftDirection().invert() * 1275.f +
      camera.rotation.getUpDirection() * tVec3f(1.f, -1.f, 1.f) * 500.f
    );

    // Camera-facing rotation
    bar.rotation = (
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.camera_angle) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI * 1.4f)
    );

    // Roll correction
    bar.rotation = bar.rotation *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.025f);

    bar.scale = tVec3f(200.f);

    bar.color = tVec3f(0.7f, 0.4f, 0.1f);
    bar.material = tVec4f(0.2f, 1.f, 1.f, 0.1f);

    commit(bar);
  }

  // Health units
  {
    const float gap = 110.f;

    auto& bar = objects(meshes.health_bar)[0];
    tVec3f axis = bar.rotation.getUpDirection();
    tVec3f base_position = bar.position - axis * 120;

    for_range(1, 4) {
      int index = i -1;
      auto& unit = objects(meshes.health_unit)[index];

      unit.position = base_position + axis * float(index) * gap;
      unit.rotation = bar.rotation;
      unit.scale = bar.scale;

      unit.color = tVec3f(1.f, 0, 0);
      unit.material = tVec4f(0.4f, 0, 1.f, 0.4f);

      commit(unit);
    }
  }
}

void HUDSystem::Update(Tachyon* tachyon, State& state) {
  UpdateHealthBar(tachyon, state);
}