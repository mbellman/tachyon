#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace WorldSetup {
    void InitializeGameWorld(Tachyon* tachyon, State& state);
    void StoreInitialObjects(Tachyon* tachyon, State& state);
    void RebuildWorld(Tachyon* tachyon, State& state);
  }
}