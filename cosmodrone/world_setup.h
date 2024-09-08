#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace WorldSetup {
    void LoadMeshes(Tachyon* tachyon, State& state);
    void InitializeGameWorld(Tachyon* tachyon, State& state);
  }
}