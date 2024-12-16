#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Autopilot {
    void HandleAutopilot(Tachyon* tachyon, State& state, const float dt);
  }
}