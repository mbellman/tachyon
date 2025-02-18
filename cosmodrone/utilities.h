#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Utilities {
    const std::vector<uint16>& GetTargetableMeshes(const State& state);
    const std::vector<uint16>& GetDockableMeshes(const State& state);
    const tVec3f GetDockingPositionOffset(const State& state);
    const tVec3f GetDockingPositionOffset(const State& state, const uint16 mesh_index);
    const Quaternion GetDockedRotation(const State& state, const uint16 mesh_index);
    const Quaternion GetDockedCameraRotation(const State& state, const tObject& target);
    const float GetDockedCameraDistance(const State& state, const uint16 mesh_index);
    const uint16 GetTargetWireframeMesh(const State& state, const uint16 mesh_index);
    const std::string GetTargetName(const State& state, const uint16 mesh_index);
    const tVec3f GetBeaconColor(const State& state, const uint16 mesh_index);
    const float GetMaxShipSpeed(const State& state);
  }
}