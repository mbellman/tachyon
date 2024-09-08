#include "cosmodrone/world_behavior.h"

using namespace Cosmodrone;

static void UpdateCelestialBodies(Tachyon* tachyon, State& state, const float dt) {
  static const tVec3f sun_direction = tVec3f(0, -1.f, 0).unit();
  static const tVec3f moon_direction = tVec3f(0, 1.f, 0.5f).unit();
  static const tVec3f orbit_rotation_axis = tVec3f(0.5f, 0, -1.f).unit();
  static const tVec3f sunlight_direction = sun_direction.invert();
  static const float moon_and_sun_distance = 6000000.f;
  static const float moon_and_sun_scale = 350000.f;

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& earth = objects(meshes.planet)[0];
  auto& moon = objects(meshes.planet)[1];
  auto& sun = objects(meshes.planet)[2];

  earth.position = camera.position + tVec3f(0, -4000000.f, 0);
  earth.color = tVec3f(0.3f, 0.7f, 1.f);
  earth.scale = tVec3f(1000000.f);
  earth.material = tVec4f(0.4f, 0, 1.f, 0.3);

  // Sun/moon
  {
    auto orbit_angle = 0.4f + tachyon->running_time * 0.001f;
    auto orbit_rotation_matrix = Quaternion::fromAxisAngle(orbit_rotation_axis, orbit_angle).toMatrix4f();
    auto unit_moon_position = orbit_rotation_matrix.transformVec3f(moon_direction);
    auto unit_sun_position = orbit_rotation_matrix.transformVec3f(sun_direction);
    auto sunlight_direction = unit_sun_position.invert();

    tachyon->scene.directional_light_direction = sunlight_direction;

    moon.position = camera.position + unit_moon_position * moon_and_sun_distance;
    moon.scale = tVec3f(moon_and_sun_scale);
    moon.color = tVec3f(0.8f);
    moon.material = tVec4f(1.f, 0, 0, 0.1f);

    sun.position = camera.position + unit_sun_position * moon_and_sun_distance;
    sun.scale = tVec3f(moon_and_sun_scale);
    sun.color = tVec4f(1.f, 0.9f, 0.5f, 1.f);
  }

  commit(earth);
  commit(moon);
  commit(sun);
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, const float dt) {
  UpdateCelestialBodies(tachyon, state, dt);
}