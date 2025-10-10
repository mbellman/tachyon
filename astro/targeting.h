#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace Targeting {
    void HandleCurrentTarget(Tachyon* tachyon, State& state);
    void SelectNearestAccessibleTarget(Tachyon* tachyon, State& state);
    void DeselectCurrentTarget(Tachyon* tachyon, State& state);
  }
}