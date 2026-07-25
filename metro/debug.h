#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace Debug {
    void HandleFrameStart(Tachyon* tachyon, State& state);
    void ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius);
  }
}