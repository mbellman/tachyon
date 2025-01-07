#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace WorldSetup {
    void InitWorld(Tachyon* tachyon, State& state);
    void RebuildWorld(Tachyon* tachyon, State& state);
  }
}