#pragma once

#include "engine/tachyon.h"
#include "astro/entity_types.h"
#include "astro/game_state.h"

namespace astro {
  namespace ObjectManager {
    void CreateObjects(Tachyon* tachyon, State& state);
  }
}