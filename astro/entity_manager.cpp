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

GameEntity EntityManager::CreateNewEntity(State& state, EntityType type) {
  GameEntity entity;
  entity.type = type;
  entity.id = running_entity_id++;

  return entity;
}

void EntityManager::SaveNewEntity(State& state, const GameEntity& entity) {
  auto& container = EntityDispatcher::GetEntityContainer(state, entity.type);

  container.push_back(entity);
}

GameEntity* EntityManager::FindEntity(State& state, const EntityRecord& record) {
  auto& entities = EntityDispatcher::GetEntityContainer(state, record.type);

  return FindEntityByRecord(entities, record);
}

GameEntity* EntityManager::FindEntityByUniqueName(State& state, const std::string& unique_name) {
  for_all_entity_types() {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      if (entity.unique_name == unique_name) {
        return &entity;
      }
    }
  }

  return nullptr;
}

void EntityManager::DeleteEntity(State& state, const EntityRecord& record) {
  auto& entities = EntityDispatcher::GetEntityContainer(state, record.type);

  DeleteEntityByRecord(entities, record);
}