#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace ControlSystem {
    void HandleControls(Tachyon* tachyon, State& state, const float dt);
  }
}