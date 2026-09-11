#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace BackgroundVehicles {
    void Update(Tachyon* tachyon, State& state);
    void SpawnBicycle(Tachyon* tachyon, State& state, Bicycle& bike);
    void SpawnScooter(Tachyon* tachyon, State& state, Scooter& scooter);
    void DestroyBicycle(Tachyon* tachyon, State& state, Bicycle& bike);
  }
}