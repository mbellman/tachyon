#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace ElectricScooter {
    void Spawn(Tachyon* tachyon, State& state, const Scooter& scooter);
    void HandlePhysics(Tachyon* tachyon, State& state, Scooter& scooter);
    void Update(Tachyon* tachyon, State& state, Scooter& scooter, const int32 index);
    void Destroy(Tachyon* tachyon, State& state, Scooter& scooter);
  }
}