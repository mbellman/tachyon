#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace WandAbilities {
    void CheckForHints(Tachyon* tachyon, State& state);
    void HandleWandAbilities(Tachyon* tachyon, State& state);
  }
}