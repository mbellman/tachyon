#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace Debug {
    void HandleFrameStart(Tachyon* tachyon, State& state);
    void ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius);
    void ShowDebugVector(Tachyon* tachyon, State& state, const tVec3f& position, const tVec3f& vector, const tVec3f& color);
    void ShowDebugPlane(Tachyon* tachyon, State& state, const CollisionPlane& plane, const tVec3f& color);
  }
}