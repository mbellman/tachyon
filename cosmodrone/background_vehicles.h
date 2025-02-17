#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace BackgroundVehicles {
    void LoadVehicleMeshes(Tachyon* tachyon, State& state);
    void InitVehicles(Tachyon* tachyon, State& state);
    void UpdateVehicles(Tachyon* tachyon, State& state, const float dt);
  }
}