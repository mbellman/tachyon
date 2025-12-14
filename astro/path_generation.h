#pragma once

#include <vector>

#include "astro/game_state.h"

namespace astro {
  namespace PathGeneration {
    void GeneratePath(Tachyon* tachyon, State& state, const std::vector<GameEntity>& nodes, std::vector<PathSegment>& segments);
  }
}