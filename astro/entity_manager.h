#pragma once

#include "astro/entities.h"
#include "astro/game_state.h"

namespace astro {
  namespace EntityManager {
    EntityRecord CreateEntity(State& state, EntityType type);
    void SaveNewEntity(State& state, const GameEntity& entity);
    GameEntity* FindEntity(State& state, const EntityRecord& record);
    void DeleteEntity(State& state, const EntityRecord& record);
  }
}