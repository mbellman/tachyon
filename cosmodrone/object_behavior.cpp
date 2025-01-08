#include <functional>

#include "cosmodrone/object_behavior.h"

#define for_dynamic_objects(__mesh_index, ...)\
  auto& initial_objects = objects(__mesh_index).initial_objects;\
  for (auto& initial : initial_objects) {\
    auto* original = get_original_object(initial);\
    if (original != nullptr) {\
      auto& object = *original;\
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

void ObjectBehavior::InitObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  StoreInitialMeshObjects(tachyon, {
    meshes.antenna_4_dish,

    meshes.elevator_torus_1,
    meshes.elevator_torus_1_frame,

    meshes.station_torus_1,

    meshes.station_torus_2_body,
    meshes.station_torus_2_supports,
    meshes.station_torus_2_frame,

    meshes.station_torus_3_body,
    meshes.station_torus_3_frame,
    meshes.station_torus_3_lights,

    meshes.habitation_4_body,
    meshes.habitation_4_core,
    meshes.habitation_4_frame,
    meshes.habitation_4_panels,
    meshes.habitation_4_lights,

    meshes.arch_1_body,
    meshes.arch_1_details,
    meshes.arch_1_frame,

    meshes.gate_tower_1,
    meshes.background_ship_1,

    meshes.elevator_car
  });
}

void ObjectBehavior::UpdateObjects(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  for_dynamic_objects(meshes.elevator_car, {
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


  UpdateRotator(tachyon, state, dt, meshes.habitation_4_body, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_core, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_frame, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_panels, 0.15f);
  UpdateRotator(tachyon, state, dt, meshes.habitation_4_lights, 0.15f);

  UpdateRotator(tachyon, state, dt, meshes.elevator_torus_1, 0.2f);
  UpdateRotator(tachyon, state, dt, meshes.elevator_torus_1_frame, -0.1f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_1, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_body, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_supports, 0.05f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_2_frame, 0.05f);

  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_body, 0.08f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_frame, 0.08f);
  UpdateRotator(tachyon, state, dt, meshes.station_torus_3_lights, 0.08f);

  UpdateRotatorWithVariation(tachyon, state, dt, meshes.antenna_4_dish, 0.5f);

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
}