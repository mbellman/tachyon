#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace SpellSystem {
    void CastStun(Tachyon* tachyon, State& state);
    void CastHoming(Tachyon* tachyon, State& state);
    void HandleSpells(Tachyon* tachyon, State& state, const float dt);
  }
}