#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace PlayerCharacter {
    void UpdatePlayer(Tachyon* tachyon, State& state);
    bool CanTakeDamage(Tachyon* tachyon, const State& state);
    void TakeDamage(Tachyon* tachyon, State& state, const float damage);
  }
}