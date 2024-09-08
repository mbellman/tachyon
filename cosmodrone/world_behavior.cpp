#include "cosmodrone/world_behavior.h"

using namespace Cosmodrone;

static void UpdateCelestialBodies(Tachyon* tachyon, State& state, MeshIds& meshes, const float dt) {
  auto& camera = tachyon->scene.camera;

  auto& earth = objects(meshes.planet)[0];
  auto& moon = objects(meshes.planet)[1];
  auto& sun = objects(meshes.planet)[2];

  earth.position = camera.position + tVec3f(0, -5000000.f, 0);
  earth.color = tVec3f(0.3f, 0.7f, 1.f);
  earth.scale = tVec3f(1000000.f);
  earth.material = tVec4f(0.4f, 0, 1.f, 0.3);

  moon.position = camera.position + tVec3f(-2000000.f, -5000000.f, -5000000.f);
  moon.color = tVec3f(0.8f);
  moon.scale = tVec3f(250000.f);
  moon.material = tVec4f(1.f, 0, 0, 0.1f);

  sun.position = camera.position + tVec3f(5000000.f);
  sun.color = tVec4f(1.f, 0.9f, 0.5f, 1.f);
  sun.scale = tVec3f(350000.f);

  commit(earth);
  commit(moon);
  commit(sun);
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, MeshIds& meshes, const float dt) {
  UpdateCelestialBodies(tachyon, state, meshes, dt);
}