#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace CommonBike {
    void Spawn(Tachyon* tachyon, State& state, const Bicycle& bike);
    void Update(Tachyon* tachyon, State& state, Bicycle& bike, const int32 index);
    void Destroy(Tachyon* tachyon, State& state, Bicycle& bike);
  }
}