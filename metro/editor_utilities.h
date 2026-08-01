#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace EditorUtilities {
    tVec3f GetClosestBasisAxis(const Quaternion& rotation, const tVec3f& vector);
    void ShowPositionGizmo(Tachyon* tachyon, State& state, const tVec3f& position, const Quaternion& basis_rotation);
    void SwivelAroundPosition(Tachyon* tachyon, State& state, const tVec3f& position);
  }
}