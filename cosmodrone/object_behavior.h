#pragma once

#include "engine/tachyon_types.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace ObjectBehavior {
    void InitObjects(Tachyon* tachyon, State& state);
    void UpdateObjects(Tachyon* tachyon, State& state, const float dt);
  }
}