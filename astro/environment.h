#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace Environment {
    void Init(Tachyon* tachyon, State& state);
    void HandleEnvironment(Tachyon* tachyon, State& state);
  }
}