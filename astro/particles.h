#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace Particles {
    void InitParticles(Tachyon* tachyon, State& state);
    void HandleParticles(Tachyon* tachyon, State& state);
  }
}