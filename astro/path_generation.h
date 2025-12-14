#pragma once

#include <vector>

#include "astro/game_state.h"

namespace astro {
  namespace PathGeneration {
    void GeneratePaths(Tachyon* tachyon, State& state, const std::vector<GameEntity>& nodes, std::vector<PathSegment>& segments, const uint16 mesh_index);
  }
}