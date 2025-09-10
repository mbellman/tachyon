#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace ObjectManager {
    void CreateObjects(Tachyon* tachyon, State& state);
  }
}