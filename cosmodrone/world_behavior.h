#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace WorldBehavior {
    void UpdateWorld(Tachyon* tachyon, State& state, const float dt);
  }
}