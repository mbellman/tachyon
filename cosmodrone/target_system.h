#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace TargetSystem {
    void HandleTargets(Tachyon* tachyon, State& state, const float dt);
  };
}