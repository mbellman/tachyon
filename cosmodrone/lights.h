#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Lights {
    void InitLights(Tachyon* tachyon, State& state);
    void UpdateLights(Tachyon* tachyon, State& state);
  }
}