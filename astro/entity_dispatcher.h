#pragma once

#include <variant>

#include "astro/entities.h"
#include "astro/game_state.h"

#define for_all_entity_types() for (auto entity_type : EntityDispatcher::GetAllEntityTypes())
#define for_entities_of_type(__type)\
  auto& entities = EntityDispatcher::GetAllEntitiesOfType(state, __type);\
  for_entities(entities)\

namespace astro {
  namespace EntityDispatcher {
    const std::vector<EntityType>& GetAllEntityTypes();
    std::vector<GameEntity>& GetAllEntitiesOfType(State& state, EntityType type);

    void SpawnObjects(Tachyon* tachyon, State& state, EntityType);
    tObject& SpawnPlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity);
    void DestroyPlaceholders(Tachyon* tachyon, State& state, EntityType type);
  }
}