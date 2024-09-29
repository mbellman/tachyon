#include "cosmodrone/world_behavior.h"

using namespace Cosmodrone;

const static float orbital_rate = 0.001f;
const static tVec3f orbit_rotation_axis = tVec3f(0.5f, 0, -1.f).unit();

static void UpdateCelestialBodies(Tachyon* tachyon, State& state, const float dt) {
  static const tVec3f sun_direction = tVec3f(0, -1.f, 0).unit();
  static const tVec3f moon_direction = tVec3f(0, 1.f, 0.5f).unit();
  static const tVec3f sunlight_direction = sun_direction.invert();
  static const float moon_distance = 6000000.f;
  static const float moon_scale = 350000.f;

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& earth = objects(meshes.planet)[0];
  auto& moon = objects(meshes.planet)[1];

  earth.position = camera.position + tVec3f(0, -4000000.f, 0);
  earth.color = tVec3f(0.1f, 0.2f, 1.f);
  earth.scale = tVec3f(1500000.f);
  earth.material = tVec4f(0.4f, 0, 1.f, 0.3);

  // Sun/moon
  {
    auto orbit_angle = 0.4f + state.current_game_time * orbital_rate;
    auto orbit_rotation_matrix = Quaternion::fromAxisAngle(orbit_rotation_axis, orbit_angle).toMatrix4f();
    auto unit_moon_position = orbit_rotation_matrix.transformVec3f(moon_direction);
    auto unit_sun_position = orbit_rotation_matrix.transformVec3f(sun_direction);
    auto sunlight_direction = unit_sun_position.invert();

    tachyon->scene.directional_light_direction = sunlight_direction;

    moon.position = camera.position + unit_moon_position * moon_distance;
    moon.scale = tVec3f(moon_scale);
    moon.color = tVec3f(0.8f);
    moon.material = tVec4f(1.f, 0, 0, 0.1f);
  }

  commit(earth);
  commit(moon);
}

static void UpdateBackgroundElements(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Space elevator
  {
    static const Quaternion base_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_PI * 0.15f);
    float current_angle = t_PI * 1.5f + state.current_game_time * orbital_rate;

    auto& elevator = objects(meshes.space_elevator)[0];

    elevator.position = camera.position + tVec3f(0, -4000000.f, 0);
    elevator.scale = tVec3f(1500000.f);
    elevator.rotation = base_rotation * Quaternion::fromAxisAngle(orbit_rotation_axis, current_angle);
    elevator.material = tVec4f(0.3f, 0.8f, 0, 0);

    commit(elevator);
  }
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, const float dt) {
  state.current_game_time += dt;

  UpdateCelestialBodies(tachyon, state, dt);
  UpdateBackgroundElements(tachyon, state, dt);
}