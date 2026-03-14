#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace ProceduralBehavior {
    namespace Generation {
      void RebuildSimpleProceduralObjects(Tachyon* tachyon, State& state);
      void RebuildAllProceduralObjects(Tachyon* tachyon, State& state);
      void UpdateProceduralObjects(Tachyon* tachyon, State& state);
    }
  }
}