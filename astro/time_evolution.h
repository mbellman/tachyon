#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace TimeEvolution {
    void UpdateAstroTime(Tachyon* tachyon, State& state);
  }
}