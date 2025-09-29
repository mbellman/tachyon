#pragma once

#include "astro/entity_types.h"
#include "astro/game_state.h"

#define for_all_entity_types() for (auto type : entity_types)

#define for_entities_of_type(__type)\
  auto& entities = EntityDispatcher::GetEntityContainer(state, __type);\
  for_entities(entities)\

namespace astro {
  namespace EntityDispatcher {
    std::vector<GameEntity>& GetEntityContainer(State& state, EntityType type);

    void AddMeshes(Tachyon* tachyon, State& state, EntityType type);
    void SpawnObjects(Tachyon* tachyon, State& state, const GameEntity& entity);
    void DestroyObjects(Tachyon* tachyon, State& state, EntityType type);
    tObject& CreatePlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity);
    void DestroyPlaceholders(Tachyon* tachyon, State& state, EntityType type);
    void TimeEvolve(Tachyon* tachyon, State& state, EntityType type);
  }
}