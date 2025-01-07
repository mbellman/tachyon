#include "cosmodrone/object_behavior.h"

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
    meshes.background_ship_1
  });
}

void ObjectBehavior::UpdateObjects(Tachyon* tachyon, State& state, const float dt) {
  // @todo
}