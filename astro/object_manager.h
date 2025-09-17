#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_state.h"

namespace astro {
  namespace ObjectManager {
    void CreateObjects(Tachyon* tachyon, State& state);
    void DeleteObjectsForEntityType(Tachyon* tachyon, State& state, EntityType type);
    void ProvisionAvailableObjectsForEntities(Tachyon* tachyon, State& state);
  }
}