#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Autopilot {
    void HandleAutopilot(Tachyon* tachyon, State& state, const float dt);
    bool IsAutopilotActive(const State& state);
    tVec3f GetDockingPosition(Tachyon* tachyon, const State& state);
    float GetDockingAlignment(const State& state, const tVec3f& docking_position);
  }
}