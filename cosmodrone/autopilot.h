#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Autopilot {
    void HandleAutopilot(Tachyon* tachyon, State& state, const float dt);
    bool IsAutopilotActive(const State& state);
    bool IsDoingDockingApproach(const State& state);
    bool IsDocked(const State& state);
    tVec3f GetDockingPosition(Tachyon* tachyon, const State& state);
    tVec3f GetDockingPosition(Tachyon* tachyon, const State& state, const tObject& object);
    float GetDockingAlignment(const State& state, const tVec3f& docking_position);
    void Undock(Tachyon* tachyon, State& state);
  }
}