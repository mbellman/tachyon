#pragma once

#include "astro/entity_types.h"
#include "astro/game_state.h"

namespace astro {
  namespace EntityManager {
    GameEntity CreateNewEntity(State& state, EntityType type);
    void SaveNewEntity(State& state, const GameEntity& entity);
    GameEntity* FindEntity(State& state, const EntityRecord& record);
    void DeleteEntity(State& state, const EntityRecord& record);
  }
}