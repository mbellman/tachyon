#pragma once

#include "engine/tachyon_types.h"
#include "astro/game_state.h"

namespace astro {
  namespace Combat {
    void HandleWandSwing(Tachyon* tachyon, State& state);
    void HandleWandStrikeWindow(Tachyon* tachyon, State& state);
  }
}