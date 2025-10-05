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
    const std::vector<uint16>& GetMeshes(State& state, EntityType type);
    uint16 GetPlaceholderMesh(State& state, EntityType type);
    void TimeEvolve(Tachyon* tachyon, State& state, EntityType type, const float dt);
  }
}