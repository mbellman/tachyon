#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace DataLoader {
    void LoadLevelData(Tachyon* tachyon, State& state);
    uint16 MeshIndexToId(State& state, uint16 mesh_index);
    uint16 MeshIdToIndex(State& state, uint16 mesh_id);
  }
}