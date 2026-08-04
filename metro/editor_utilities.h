#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace EditorUtilities {
    tVec3f GetClosestBasisAxis(const Quaternion& rotation, const tVec3f& vector);
    tVec3f GetClosestWorldAxis(const tVec3f& vector);
    tVec3f GetScalingAxis(const tVec3f& basis_axis, const Quaternion& basis_rotation);
    void ShowPositionGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation);
    void ShowScaleGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation);
    void ShowRotationGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation);
    void UseWASDCameraMovement(Tachyon* tachyon, State& state, const float speed);
    void UseMouseCameraLookaround(Tachyon* tachyon, State& state, const float sensitivity);
    void SwivelAroundPosition(Tachyon* tachyon, const tVec3f& position);
  }
}