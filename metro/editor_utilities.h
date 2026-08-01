#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace EditorUtilities {
    void ShowPositionGizmo(Tachyon* tachyon, State& state, const tVec3f& position);
    void SwivelAroundPosition(Tachyon* tachyon, State& state, const tVec3f& position);
  }
}