#include <functional>

#include "cosmodrone/object_behavior.h"

#define for_dynamic_objects(__mesh_index, code)\
  auto& initial_objects = objects(__mesh_index).initial_objects;\
  for (auto& initial : initial_objects) {\
    auto* original = get_original_object(initial);\
    if (original != nullptr) {\
      auto& object = *original;\
      code\
    }\
  }\

#define run_behavior(__mesh_indexes, code)\
  {\
    auto indexes = __mesh_indexes;\
    for (int i = 0; i < indexes.size(); i++) {\
      auto mesh_index = *(indexes.begin() + i);\
      for_dynamic_objects(mesh_index, code);\
    }\
  }

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

  run_behavior({ meshes.elevator_car }, {
    auto direction = object.object_id % 2 == 0 ? 1.f : -1.f;

    object.position.y += 25000.f * dt * direction;

    // @temporary
    if (std::abs(object.position.y - state.ship_position.y) > 500000.f) {
      object.position.y = state.ship_position.y + 499999.f * -direction;
    }

    commit(object);
  });
}