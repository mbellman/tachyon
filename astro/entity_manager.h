#pragma once

#include "astro/entities.h"
#include "astro/game_state.h"

namespace astro {
  namespace EntityManager {
    EntityRecord CreateEntity(State& state, EntityType type);
    GameEntity* FindEntity(State& state, const EntityRecord& record);
    void DeleteEntity(State& state, const EntityRecord& record);
  }
}