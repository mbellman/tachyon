#include <functional>

#include "cosmodrone/object_behavior.h"

#define for_dynamic_objects(__mesh_index, ...)\
  auto& initial_objects = objects(__mesh_index).initial_objects;\
  for (auto& initial : initial_objects) {\
    auto* live = get_live_object(initial);\
    if (live != nullptr) {\
      auto& object = *live;\
      __VA_ARGS__\
    }\
  }\

using namespace Cosmodrone;

static inline void StoreInitialMeshObjects(Tachyon* tachyon, const std::vector<uint16>& mesh_indexes) {
  for (auto mesh_index : mesh_indexes) {
    auto& group = objects(mesh_index);

    group.initial_objects.clear();

    for (auto& object : group) {
      group.initial_objects.push_back(object);
    }
  }
}

static inline void UpdateRotator(Tachyon* tachyon, const State& state, const float dt, const uint16 mesh_index, const float rate) {
  for_dynamic_objects(mesh_index, {
    auto axis = initial.rotation.getUpDirection();

    object.rotation = Quaternion::fromAxisAngle(axis, state.current_game_time * rate) * initial.rotation;

    commit(object);
  });
}

static inline void UpdateRotatorWithVariation(Tachyon* tachyon, const State& state, const float dt, const uint16 mesh_index, const float rate) {
  for_dynamic_objects(mesh_index, {
    auto axis = initial.rotation.getUpDirection();

    auto offset =
      initial.position.x * 0.005f +
      initial.position.y * 0.005f +
      initial.position.z * 0.005f;

    auto adjusted_rate = rate * (initial.object_id % 2 == 0 ? -1.f : 1.f);

    object.rotation =
      Quaternion::fromAxisAngle(axis, offset + state.current_game_time * adjusted_rate) *
      initial.rotation;

    commit(object);
  });
}

static inline void UpdateRotatingArchPart(Tachyon* tachyon, const State& state, const float dt, uint16 mesh_index) {
  for_dynamic_objects(mesh_index, {
    auto axis = initial.rotation.getUpDirection();
    auto offset = initial.position.y * 0.00001f;
    auto rate = 0.01f * sinf(initial.position.y);

    object.rotation = Quaternion::fromAxisAngle(axis, offset + state.current_game_time * rate) * initial.rotation;

    commit(object);
  });
}

static inline void UpdateStationDronePart(Tachyon* tachyon, const State& state, uint16 mesh_index) {
  for_dynamic_objects(mesh_index, {
    float center_distance = sqrtf(initial.position.x * initial.position.x + initial.position.z * initial.position.z);
    float t = 10000.f * (state.current_game_time / center_distance) + 2.f * float(object.object_id);

    object.position.x = center_distance * sinf(t);
    object.position.z = center_distance * cosf(t);

    float angle = -atan2f(object.position.z, object.position.x);

    object.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);

    commit(object);
  });
}

void ObjectBehavior::InitObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  StoreInitialMeshObjects(tachyon, {
    meshes.antenna_4_dish,
    meshes.antenna_5_dish,

    meshes.elevator_torus_1,
    meshes.elevator_torus_1_frame,

    meshes.station_torus_1,

    meshes.station_torus_2_body,
    meshes.station_torus_2_supports,
    meshes.station_torus_2_frame,

    meshes.station_torus_3_body,
    meshes.station_torus_3_frame,
    meshes.station_torus_3_lights,

    meshes.platform_torus,

    meshes.habitation_4_body,
    meshes.habitation_4_core,
    meshes.habitation_4_frame,
    meshes.habitation_4_panels,
    meshes.habitation_4_lights,

    meshes.solar_rotator_body,
    meshes.solar_rotator_frame,
    meshes.solar_rotator_panels,

    meshes.solar_rotator_2_body,
    meshes.solar_rotator_2_frame,
    meshes.solar_rotator_2_panels,
    meshes.solar_rotator_2_lights,

    meshes.floater_1_base,
    meshes.floater_1_spokes,
    meshes.floater_1_panels,

    meshes.arch_1_body,
    meshes.arch_1_details,
    meshes.arch_1_frame,

    meshes.gate_tower_1,
    meshes.background_ship_1,

    meshes.station_drone_core,
    meshes.station_drone_frame,
    meshes.station_drone_rotator,
    meshes.station_drone_light,

    meshes.procedural_elevator_car,
    meshes.procedural_elevator_car_light
  });
}

void ObjectBehavior::UpdateObjects(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  // @todo factor
  {
    for_dynamic_objects(meshes.procedural_elevator_car, {
      auto direction =
        object.object_id % 2 == 0
          ? 1.f
          : -1.f;

      object.position.y += 25000.f * dt * direction;

      // @temporary
      if (std::abs(object.position.y - state.ship_position.y) > 1000000.f) {
        object.position.y = state.ship_position.y + 999999.f * -direction;
      }

      commit(object);
    });
  }

  // @todo factor
  {
    for_dynamic_objects(meshes.procedural_elevator_car_light, {
      auto direction =
        object.object_id % 2 == 0
          ? 1.f
          : -1.f;

      object.position.y += 25000.f * dt * direction;

      // @temporary
      if (std::abs(object.position.y - state.ship_position.y) > 1000000.f) {
        object.position.y = state.ship_position.y + 999999.f * -direction;
      }

      commit(object);
    });
  }

  UpdateRotator(tachyon, state, dt, meshes.habitation_4_body, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_core, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_frame, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_panels, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_lights, 0.15f);

  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_body, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_frame, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_panels, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_2_body, 0.03f);
  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_2_frame, 0.03f);
  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_2_panels, 0.03f);
  UpdateRotator(tachyon, state, dt, meshes.solar_rotator_2_lights, 0.03f);

  UpdateRotator(tachyon, state, dt, meshes.elevator_torus_1, 0.2f);
  UpdateRotator(tachyon, state, dt, meshes.elevator_torus_1_frame, -0.1f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_1, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_body, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_supports, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_frame, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_body, 0.08f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_frame, 0.08f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_lights, 0.08f);

  UpdateRotator(tachyon, state, dt, meshes.platform_torus, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.floater_1_base, -0.2f);
  UpdateRotator(tachyon, state, dt, meshes.floater_1_spokes, 0.2f);
  UpdateRotator(tachyon, state, dt, meshes.floater_1_panels, 0.2f);

  UpdateRotatorWithVariation(tachyon, state, dt, meshes.antenna_4_dish, 0.5f);

  // antenna_5
  {
    for_dynamic_objects(meshes.antenna_5_dish, {
      object.rotation =
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), sinf(state.current_game_time * 0.3f)) *
        initial.rotation;

      commit(object);
    });
  }

  // Rotating arches
  {
    UpdateRotatingArchPart(tachyon, state, dt, meshes.arch_1_body);
    UpdateRotatingArchPart(tachyon, state, dt, meshes.arch_1_details);
    UpdateRotatingArchPart(tachyon, state, dt, meshes.arch_1_frame);
  }

  // Gate towers
  {
    for_dynamic_objects(meshes.gate_tower_1, {
      object.position.y =
        initial.position.y +
        150000.f * sinf(state.current_game_time * 0.05f + initial.position.x * 0.001f);

      commit(object);
    });
  }

  // Background ships
  {
    for_dynamic_objects(meshes.background_ship_1, {
      object.position.y =
        initial.position.y +
        50000.f * sinf(state.current_game_time * 0.1f + initial.position.x * 0.001f);

      object.rotation =
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.1f * sinf(state.current_game_time * 0.1f + initial.position.x * 0.01f)) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.1f * sinf(state.current_game_time * 0.13f + initial.position.y * 0.01f)) *
        initial.rotation;

      commit(object);
    });
  }

  // Station drones
  {
    UpdateStationDronePart(tachyon, state, meshes.station_drone_core);
    UpdateStationDronePart(tachyon, state, meshes.station_drone_frame);
    UpdateStationDronePart(tachyon, state, meshes.station_drone_rotator);
    UpdateStationDronePart(tachyon, state, meshes.station_drone_light);

    for_dynamic_objects(meshes.station_drone_rotator, {
      // @optimize
      tVec3f forward = object.rotation.getDirection();

      object.rotation *= Quaternion::fromAxisAngle(forward, -state.current_game_time);

      commit(object);
    });
  }
}