#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Beacons {
    void InitBeacons(Tachyon* tachyon, State& state);
    void UpdateBeacons(Tachyon* tachyon, State& state);
  }
}