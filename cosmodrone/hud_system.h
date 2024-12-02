#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace HUDSystem {
    void HandleHUD(Tachyon* tachyon, State& state, const float dt);
  }
}