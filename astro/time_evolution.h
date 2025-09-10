#pragma once

#include "engine/tachyon_types.h"
#include "astro/game_state.h"

namespace astro {
  namespace TimeEvolution {
    void HandleAstroTime(Tachyon* tachyon, State& state, const float dt);
  }
}