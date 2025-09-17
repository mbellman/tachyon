#include "astro/entity_manager.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

static int32 running_entity_id = 0;

static inline GameEntity* FindEntityByRecord(std::vector<GameEntity>& entities, const EntityRecord& record) {
  for (auto& entity : entities) {
    if (entity.id == record.id) {
      return &entity;
    }
  }

  return nullptr;
}

static inline void DeleteEntityByRecord(std::vector<GameEntity>& entities, const EntityRecord& record) {
  // @todo @optimize If we wanted, we could swap with the last element
  // and declare a bound on the number of "active" entities, instead of
  // removing them from the array and shuffling data around memory.
  // However, this will suffice for now.
  for (size_t i = 0; i < entities.size(); i++) {
    if (record.id == entities[i].id) {
      entities.erase(entities.begin() + i);

      break;
    }
  }
}

EntityRecord EntityManager::CreateEntity(State& state, EntityType type) {
  GameEntity entity;
  entity.type = type;
  entity.id = running_entity_id++;

  auto& entities = EntityDispatcher::GetAllEntitiesOfType(state, type);

  entities.push_back(entity);

  return { entity.id, type };
}

GameEntity* EntityManager::FindEntity(State& state, const EntityRecord& record) {
  auto& entities = EntityDispatcher::GetAllEntitiesOfType(state, record.type);

  return FindEntityByRecord(entities, record);
}

void EntityManager::DeleteEntity(State& state, const EntityRecord& record) {
  auto& entities = EntityDispatcher::GetAllEntitiesOfType(state, record.type);

  DeleteEntityByRecord(entities, record);
}