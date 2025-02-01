#include "cosmodrone/background_vehicles.h"
#include "cosmodrone/object_behavior.h"
#include "cosmodrone/world_behavior.h"

using namespace Cosmodrone;

const static float orbital_rate = 0.001f;
const static tVec3f orbit_rotation_axis = tVec3f(0.5f, 0, -1.f).unit();

static void UpdateCelestialBodies(Tachyon* tachyon, State& state) {
  static const tVec3f sun_direction = tVec3f(0, -1.f, 0).unit();
  static const tVec3f moon_direction = tVec3f(0, 1.f, 0.5f).unit();
  static const tVec3f sunlight_direction = sun_direction.invert();
  static const float moon_distance = 6000000.f;
  static const float moon_scale = 350000.f;

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Earth
  {
    auto& earth = objects(meshes.planet)[0];
    auto& atmosphere = objects(meshes.earth_atmosphere)[0];

    earth.position = camera.position + tVec3f(0, -1E7f, 0);
    earth.color = tVec3f(0.1f, 0.2f, 1.f);
    earth.scale = tVec3f(3200000.f);
    earth.material = tVec4f(0.4f, 0.f, 1.f, 0.3);

    atmosphere.position = earth.position;
    atmosphere.scale = earth.scale * 1.015f;

    commit(earth);
    commit(atmosphere);
  }

  // Sun/moon
  {
    auto orbit_angle = 0.4f + state.current_game_time * orbital_rate;
    auto orbit_rotation_matrix = Quaternion::fromAxisAngle(orbit_rotation_axis, orbit_angle).toMatrix4f();
    auto unit_moon_position = orbit_rotation_matrix.transformVec3f(moon_direction);
    auto unit_sun_position = orbit_rotation_matrix.transformVec3f(sun_direction);
    auto sunlight_direction = unit_sun_position.invert();

    tachyon->scene.directional_light_direction = sunlight_direction;
    tachyon->scene.scene_time = state.current_game_time;

    auto& moon = objects(meshes.planet)[1];

    moon.position = camera.position + unit_moon_position * moon_distance;
    moon.scale = tVec3f(moon_scale);
    moon.color = tVec3f(0.8f);
    moon.material = tVec4f(1.f, 0, 0, 0.1f);

    commit(moon);
  }
}

// @todo remove
static void UpdateSpaceElevator(Tachyon* tachyon, State& state) {
  // auto& camera = tachyon->scene.camera;
  // auto& meshes = state.meshes;

  // static const Quaternion base_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_PI * 0.15f);
  // float current_angle = t_PI * 1.5f + state.current_game_time * orbital_rate;
  // auto& elevator = objects(meshes.space_elevator)[0];

  // elevator.position = camera.position + tVec3f(0, -5000000.f, 0);
  // elevator.scale = tVec3f(1700000.f);
  // elevator.rotation = base_rotation * Quaternion::fromAxisAngle(orbit_rotation_axis, current_angle);
  // elevator.material = tVec4f(0.3f, 0.8f, 0, 0);

  // commit(elevator);
}

// @todo lights.cpp
static void UpdateGasFlareLights(Tachyon* tachyon, State& state) {
  for (auto light_index : state.gas_flare_light_indexes) {
    auto& light = tachyon->point_lights[light_index];
    auto t = state.current_game_time * 0.5f + light.position.x;
    auto power = 5.f * (0.5f * sinf(t) + 0.5f);

    light.power = power;
  }
}

// @todo lights.cpp
static void UpdateBlinkingLights(Tachyon* tachyon, State& state) {
  for (auto& blinking_light : state.blinking_lights) {
    auto& light = tachyon->point_lights[blinking_light.light_index];
    auto& bulb = *get_original_object(blinking_light.bulb);
    auto power = 0.5f * sinf(state.current_game_time * 3.f + light.position.x * 0.03f) + 0.5f;
    power = powf(power, 5.f);

    light.power = power;
    bulb.color = tVec4f(1.f, 0.5f, 0.2f, power);

    commit(bulb);
  }
}

// @todo lights.cpp
static void UpdateMovingLights(Tachyon* tachyon, State& state) {
  for (auto& moving_light : state.moving_lights) {
    auto& light = tachyon->point_lights[moving_light.light_index];
    auto& bulb = *get_original_object(moving_light.light_object);

    light.position = bulb.position;

    if (bulb.mesh_index == state.meshes.station_drone_light) {
      // @optimize
      light.position = bulb.position + bulb.rotation.getDirection() * 2000.f;
    }
  }
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, const float dt) {
  // Do these first so they can be updated in editor mode when changing game time
  UpdateCelestialBodies(tachyon, state);
  // UpdateSpaceElevator(tachyon, state);

  // @todo dev mode only
  if (state.is_editor_active) {
    return;
  }

  state.current_game_time += dt;

  BackgroundVehicles::UpdateVehicles(tachyon, state, dt);
  ObjectBehavior::UpdateObjects(tachyon, state, dt);

  UpdateGasFlareLights(tachyon, state);
  UpdateBlinkingLights(tachyon, state);
  UpdateMovingLights(tachyon, state);
}