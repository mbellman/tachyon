#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace ProceduralBehavior {
    namespace Trees {
      void GenerateTreeMushrooms(Tachyon* tachyon, State& state);
      void UpdateTreeMushrooms(Tachyon* tachyon, State& state);
    }
  }
}