#include "cosmodrone/world_behavior.h"

#define for_dynamic_objects(__mesh_index, code)\
  auto& initial_objects = objects(__mesh_index).initial_objects;\
  for (auto& initial : initial_objects) {\
    auto* original = get_original_object(initial);\
    if (original != nullptr) {\
      auto& object = *original;\
      code\
    }\
  }\

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

    earth.position = camera.position + tVec3f(0, -4000000.f, 0);
    earth.color = tVec3f(0.1f, 0.2f, 1.f);
    earth.scale = tVec3f(1500000.f);
    earth.material = tVec4f(0.4f, 0, 1.f, 0.3);

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

static void UpdateSpaceElevator(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  static const Quaternion base_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_PI * 0.15f);
  float current_angle = t_PI * 1.5f + state.current_game_time * orbital_rate;
  auto& elevator = objects(meshes.space_elevator)[0];

  elevator.position = camera.position + tVec3f(0, -4000000.f, 0);
  elevator.scale = tVec3f(1500000.f);
  elevator.rotation = base_rotation * Quaternion::fromAxisAngle(orbit_rotation_axis, current_angle);
  elevator.material = tVec4f(0.3f, 0.8f, 0, 0);

  commit(elevator);
}

static inline void UpdateRotatorObjects(Tachyon* tachyon, const State& state, const float dt, const uint16 mesh_index, const float rate) {
  for_dynamic_objects(mesh_index, {
    auto axis = initial.rotation.getUpDirection();

    object.rotation = Quaternion::fromAxisAngle(axis, state.current_game_time * rate) * initial.rotation;

    commit(object);
  });
}

static void UpdateRotators(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  UpdateRotatorObjects(tachyon, state, dt, meshes.habitation_4_body, 0.15f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.habitation_4_core, 0.15f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.habitation_4_frame, 0.15f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.habitation_4_panels, 0.15f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.habitation_4_lights, 0.15f);

  UpdateRotatorObjects(tachyon, state, dt, meshes.elevator_torus_1, 0.2f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.elevator_torus_1_frame, -0.1f);

  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_1, 0.05f);

  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_2_body, 0.05f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_2_supports, 0.05f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_2_frame, 0.05f);

  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_3_body, 0.08f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_3_frame, 0.08f);
  UpdateRotatorObjects(tachyon, state, dt, meshes.station_torus_3_lights, 0.08f);
}

static void UpdateLocalEntities(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  float scene_time = tachyon->scene.scene_time;

  // @todo turn these into placeable entities
  auto& elevator_car = objects(meshes.elevator_car_1)[0];
  auto& elevator_car_frame = objects(meshes.elevator_car_1_frame)[0];

  elevator_car.scale = elevator_car_frame.scale = tVec3f(12000.f);
  elevator_car.rotation = elevator_car_frame.rotation = Quaternion(0.707f, -0.707f, 0, 0);

  elevator_car.position.x = elevator_car_frame.position.x = 10065.f;
  elevator_car.position.z = elevator_car_frame.position.z = 11148.f;

  elevator_car_frame.material = tVec4f(0.4f, 1.f, 0, 0);

  float y = elevator_car.position.y + 20000.f * dt;

  // @temporary
  if (y > 400000.f) {
    y = -50000.f;
  }

  elevator_car.position.y = y;
  elevator_car_frame.position.y = y;

  commit(elevator_car);
  commit(elevator_car_frame);
}

static void UpdateGasFlareLights(Tachyon* tachyon, State& state, const float dt) {
  for (auto light_index : state.gas_flare_light_indexes) {
    auto& light = tachyon->point_lights[light_index];
    auto t = state.current_game_time * 0.5f + light.position.x;
    auto power = 5.f * (0.5f * sinf(t) + 0.5f);

    light.power = power;
  }
}

void WorldBehavior::UpdateWorld(Tachyon* tachyon, State& state, const float dt) {
  // Game time cycle-dependent entities
  UpdateCelestialBodies(tachyon, state);
  UpdateSpaceElevator(tachyon, state);

  // @todo dev mode only
  if (state.is_editor_active) {
    return;
  }

  state.current_game_time += dt;

  // Game time cycle-independent entities
  UpdateRotators(tachyon, state, dt);
  UpdateLocalEntities(tachyon, state, dt);
  UpdateGasFlareLights(tachyon, state, dt);
}