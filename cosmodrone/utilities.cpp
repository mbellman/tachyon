#include "cosmodrone/utilities.h"

using namespace Cosmodrone;

/**
 * @todo description
 */
const std::vector<uint16>& Utilities::GetTargetableMeshes(const State& state) {
  // @todo
  return {};
}

/**
 * @todo description
 */
const std::vector<uint16>& Utilities::GetDockableMeshes(const State& state) {
  auto& meshes = state.meshes;

  const static std::vector<uint16> dockable_meshes = {
    meshes.antenna_3,
    meshes.antenna_5,
    meshes.charge_pad,
    meshes.fighter_dock,
    meshes.floater_1
  };

  return dockable_meshes;
}

/**
 * @todo description
 */
const uint16 Utilities::GetTargetWireframeMesh(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.antenna_3) {
    return meshes.antenna_3_wireframe;
  }

  if (mesh_index == meshes.floater_1) {
    return meshes.floater_1_wireframe;
  }

  if (mesh_index == meshes.station_drone_core) {
    return meshes.station_drone_wireframe;
  }

  if (mesh_index == meshes.fighter_dock) {
    return meshes.fighter_wireframe;
  }

  return meshes.antenna_3_wireframe;
}

/**
 * @todo description
 */
const std::string Utilities::GetTargetName(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.antenna_3) {
    return "ANTENNA-3";
  }

  if (mesh_index == meshes.antenna_5) {
    return "SENTRY RADAR";
  }

  if (mesh_index == meshes.fighter_dock) {
    return "PEREGRINE";
  }

  if (mesh_index == meshes.floater_1) {
    return "STARFLOWER";
  }

  if (mesh_index == meshes.station_drone_core) {
    return "SENTINEL";
  }

  if (mesh_index == meshes.procedural_elevator_car) {
    return "CABLE CAR";
  }

  return "--UNNAMED--";
}

/**
 * @todo description
 */
const tVec3f Utilities::GetBeaconColor(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.antenna_3) {
    return tVec3f(1.f, 0.6f, 0.2f);
  }

  if (mesh_index == meshes.antenna_5) {
    return tVec3f(1.f, 0.6f, 0.2f);
  }

  if (mesh_index == meshes.charge_pad) {
    return tVec3f(0.2f, 1.f, 0.5f);
  }

  if (mesh_index == meshes.fighter_dock) {
    return tVec3f(1.f, 0.3f, 0.2f);
  }

  if (mesh_index == meshes.floater_1) {
    return tVec3f(0.2f, 1.f, 0.5f);
  }

  return tVec3f(1.f);
}

/**
 * @todo description
 */
const float Utilities::GetMaxShipSpeed(const State& state) {
  if (state.flight_system == FlightSystem::FIGHTER) {
    return 100000.f;
  }

  return 20000.f;
}