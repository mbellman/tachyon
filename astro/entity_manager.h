#pragma once

#include <string>

#include "astro/entities.h"
#include "astro/game_state.h"

namespace astro {
  namespace EntityManager {
    GameEntity CreateNewEntity(State& state, EntityType type);
    void SaveNewEntity(State& state, const GameEntity& entity);
    GameEntity* FindEntity(State& state, const EntityRecord& record);
    GameEntity* FindEntityByUniqueName(State& state, const std::string& unique_name);
    void DeleteEntity(State& state, const EntityRecord& record);
    void CreateEntityAssociations(State& state);
  }
}