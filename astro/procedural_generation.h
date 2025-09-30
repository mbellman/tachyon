#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace ProceduralGeneration {
    void RebuildProceduralObjects(Tachyon* tachyon, State& state);
    void UpdateProceduralObjects(Tachyon* tachyon, State& state);
  }
}