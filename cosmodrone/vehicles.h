#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Vehicles {
    void LoadVehicleMeshes(Tachyon* tachyon);
    void InitVehicles(Tachyon* tachyon, State& state);
    void UpdateVehicles(Tachyon* tachyon, State& state, const float dt);
  }
}