#include "cosmodrone/utilities.h"

using namespace Cosmodrone;

/**
 * @todo description
 */
const std::vector<uint16>& Utilities::GetTargetableMeshes(const State& state) {
  const static std::vector<uint16> targetable_meshes = {
    state.meshes.antenna_3,
    state.meshes.antenna_5,
    state.meshes.charge_pad,
    state.meshes.fighter_dock,
    state.meshes.freight_dock,
    state.meshes.floater_1,
    state.meshes.station_drone_core,
    state.meshes.procedural_elevator_car
  };

  return targetable_meshes;
}

/**
 * @todo description
 */
const std::vector<uint16>& Utilities::GetDockableMeshes(const State& state) {
  const static std::vector<uint16> dockable_meshes = {
    state.meshes.antenna_3,
    state.meshes.antenna_5,
    state.meshes.charge_pad,
    state.meshes.fighter_dock,
    state.meshes.freight_dock,
    state.meshes.floater_1
  };

  return dockable_meshes;
}

/**
 * @todo description
 */
const tVec3f Utilities::GetDockingPositionOffset(const State& state) {
  return Utilities::GetDockingPositionOffset(state, state.docking_target.mesh_index);
}

/**
 * @todo description
 */
const tVec3f Utilities::GetDockingPositionOffset(const State& state, const uint16 mesh_index) {
  if (mesh_index == state.meshes.antenna_3) {
    return tVec3f(0, -1.f, -1.f).unit() * 0.75f;
  }

  if (mesh_index == state.meshes.antenna_5) {
    return tVec3f(0, -1.f, -1.f).unit() * 0.8f;
  }

  if (mesh_index == state.meshes.fighter_dock) {
    return tVec3f(0, 0.3f, 0.35f);
  }

  if (mesh_index == state.meshes.freight_dock) {
    return tVec3f(0, 0.21f, 0.62f);
  }

  if (mesh_index == state.meshes.floater_1) {
    return tVec3f(0, 0.8f, 0);
  }

  return tVec3f(0.f);
}

/**
 * @todo description
 */
const Quaternion Utilities::GetDockedRotation(const State& state, const uint16 mesh_index) {
  if (
    mesh_index == state.meshes.antenna_3 ||
    mesh_index == state.meshes.antenna_5 ||
    mesh_index == state.meshes.floater_1
  ) {
    return Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
  }

  return Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f);
}

/**
 * @todo description
 */
const Quaternion Utilities::GetDockedCameraRotation(const State& state, const tObject& target) {
  auto& meshes = state.meshes;

  if (
    target.mesh_index == meshes.antenna_3 ||
    target.mesh_index == meshes.floater_1
  ) {
    return (
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.6f) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.2f) *
      target.rotation.opposite()
    );
  }

  if (target.mesh_index == meshes.antenna_5) {
    return (
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI) *
      target.rotation.opposite()
    );
  }

  return (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.2f) *
    target.rotation.opposite()
  );
}

/**
 * @todo description
 */
const float Utilities::GetDockedCameraDistance(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.fighter_dock) {
    return 16000.f;
  }

  if (mesh_index == meshes.freight_dock) {
    return 18000.f;
  }

  return 30000.f;
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

  if (mesh_index == meshes.freight_dock) {
    return meshes.freight_wireframe;
  }

  return meshes.antenna_3_wireframe;
}

/**
 * @todo description
 */
const std::string Utilities::GetTargetName(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.antenna_3) {
    return "RADIO TELESCOPE";
  }

  if (mesh_index == meshes.antenna_5) {
    return "SENTRY RADAR";
  }

  if (mesh_index == meshes.fighter_dock) {
    return "PEREGRINE";
  }

  if (mesh_index == meshes.freight_dock) {
    return "CARGO FERRY";
  }

  if (mesh_index == meshes.floater_1) {
    return "STARFLOWER";
  }

  if (mesh_index == meshes.station_drone_core) {
    return "SENTINEL";
  }

  if (mesh_index == meshes.procedural_elevator_car) {
    return "ORBITAL LIFT";
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

  if (mesh_index == meshes.fighter_dock || mesh_index == meshes.freight_dock) {
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
    return 150000.f;
  }

  return 20000.f;
}

/**
 * @todo description
 */
const float Utilities::GetCameraDistanceTarget(const State& state) {
  if (
    state.flight_mode == ::AUTO_DOCK && (
      state.auto_dock_stage == ::APPROACH ||
      state.auto_dock_stage == ::DOCKING_CONNECTION ||
      state.auto_dock_stage == ::DOCKED
    )
  ) {
    return state.ship_camera_distance;
  }

  if (state.flight_system == ::FIGHTER) {
    return 16000.f;
  }

  return 1900.f;
}