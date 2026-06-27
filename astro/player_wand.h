#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace PlayerWand {
    void Update(Tachyon* tachyon, State& state);
    bool DidRecentlyPulse(Tachyon* tachyon, State& state);
    WandPulse GetPulse(Tachyon* tachyon);
  }
}