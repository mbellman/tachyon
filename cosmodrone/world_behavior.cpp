#include "cosmodrone/background_vehicles.h"
#include "cosmodrone/object_behavior.h"
#include "cosmodrone/world_behavior.h"

using namespace Cosmodrone;

const static float orbital_rate = 0.001f;
const static tVec3f orbit_rotation_axis = tVec3f(0.5f, 0, -1.f).unit();

const static Quaternion base_earth_rotation =
  Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_PI * 0.25f);

const static Quaternion base_moon_rotation =
  Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_PI * 0.2f) *
  Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), t_PI * 0.5f);

static void UpdateCelestialBodies(Tachyon* tachyon, State& state) {
  static const tVec3f sun_direction = tVec3f(0, -1.f, 0).unit();
  static const tVec3f moon_direction = tVec3f(0, 1.f, 0.5f).unit();
  static const tVec3f sunlight_direction = sun_direction.invert();
  static const float earth_distance = -1E8f;
  static const float moon_distance = 6000000.f;
  static const float moon_scale = 350000.f;

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Earth
  {
    auto& earth = objects(meshes.earth)[0];
    auto& atmosphere = objects(meshes.earth_atmosphere)[0];

    earth.position = camera.position + tVec3f(0, earth_distance, 0);
    earth.scale = tVec3f(35000000.f);
    earth.color = tVec3f(0.1f, 0.2f, 1.f);
    earth.material = tVec4f(0.4f, 0.f, 1.f, 0.3);

    earth.rotation =
      Quaternion::fromAxisAngle(orbit_rotation_axis, 0.5f) *
      base_earth_rotation;

    atmosphere.position = earth.position;
    atmosphere.scale = earth.scale * 1.04f;

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

    auto& moon = objects(meshes.moon)[0];

    moon.position = camera.position + unit_moon_position * moon_distance;
    moon.scale = tVec3f(moon_scale);
    moon.color = tVec3f(0.8f);
    moon.material = tVec4f(1.f, 0, 0, 0.1f);

    moon.rotation =
      Quaternion::fromAxisAngle(orbit_rotation_axis, state.current_game_time * orbital_rate) *
      base_moon_rotation;

    commit(moon);
  }
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
    auto& bulb = *get_live_object(blinking_light.bulb);
    auto power = 0.5f * sinf(state.current_game_time * 3.f + light.position.x * 0.03f) + 0.5f;
    power = powf(power, 5.f);

    light.power = power;
    bulb.color = tVec4f(1.f, 0.5f, 0.2f, power);

    commit(bulb);
  }
}

// @todo lights.cpp
// @todo handle multiple moving lights per object
static void UpdateMovingLights(Tachyon* tachyon, State& state) {
  for (auto& moving_light : state.moving_lights) {
    auto& light = tachyon->point_lights[moving_light.light_index];
    auto& bulb = *get_live_object(moving_light.light_object);

    light.position = bulb.position;

    if (bulb.mesh_index == state.meshes.station_drone_light) {
      // @optimize
      light.position = bulb.position + bulb.rotation.getDirection() * 1500.f;
    }

    if (bulb.mesh_index == state.meshes.procedural_elevator_car_light) {
      // @optimize
      light.position =
        bulb.position +
        bulb.rotation.toMatrix4f() * (tVec3f(0, 1.f, -0.5f) * 3400.f);
    }
  }
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, const float dt) {
  // Do these first so they can be updated in editor mode when changing game time
  UpdateCelestialBodies(tachyon, state);

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