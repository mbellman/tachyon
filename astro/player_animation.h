#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace PlayerAnimation {
    void Update(Tachyon* tachyon, State& state);
    float GetRunCycleAnimationTime(State& state);
  }
}