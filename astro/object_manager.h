#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_state.h"

namespace astro {
  namespace ObjectManager {
    void CreateObjects(Tachyon* tachyon, State& state);
    void CreateObjectsForEntity(Tachyon* tachyon, State& state, EntityType type);
  }
}