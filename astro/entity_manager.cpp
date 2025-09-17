#include "astro/entity_manager.h"

using namespace astro;

static int32 running_entity_id = 0;

static EntityRecord CreateTreeEntity(State& state, EntityType type) {
  GameEntity tree;
  tree.type = type;
  tree.id = running_entity_id++;

  if (type == OAK_TREE) {
    state.oak_trees.push_back(tree);
  }
  else if (type == WILLOW_TREE) {
    state.willow_trees.push_back(tree);
  }

  return { tree.id, type };
}

static EntityRecord CreatePlantEntity(State& state, EntityType type) {
  GameEntity plant;
  plant.type = type;
  plant.id = running_entity_id++;

  if (type == SHRUB) {
    state.shrubs.push_back(plant);
  }

  return { plant.id, type };
}

static GameEntity* FindEntityFromArray(std::vector<GameEntity>& array, const EntityRecord& record) {
  for (auto& entity : array) {
    if (entity.id == record.id) {
      return &entity;
    }
  }

  return nullptr;
}

void DeleteEntityFromArray(std::vector<GameEntity>& array, const EntityRecord& record) {
  for (size_t i = 0; i < array.size(); i++) {
    if (record.id == array[i].id) {
      array.erase(array.begin() + i);

      break;
    }
  }
}

EntityRecord EntityManager::CreateEntity(State& state, EntityType type) {
  switch (type) {
    case SHRUB:
      return CreatePlantEntity(state, type);

    case OAK_TREE:
    case WILLOW_TREE:
      return CreateTreeEntity(state, type);

    default:
      // Unknown entity type requested
      // @todo log error
      return {
        .id = -1,
        .type = UNSPECIFIED
      };
  }
}

GameEntity* EntityManager::FindEntity(State& state, const EntityRecord& record) {
  switch (record.type) {
    case SHRUB:
      return FindEntityFromArray(state.shrubs, record);
    case OAK_TREE:
      return FindEntityFromArray(state.oak_trees, record);
    case WILLOW_TREE:
      return FindEntityFromArray(state.willow_trees, record);
    default:
      // @todo log error
      return nullptr;
  }
}

void EntityManager::DeleteEntity(State& state, const EntityRecord& record) {
  switch (record.type) {
    case SHRUB:
      DeleteEntityFromArray(state.shrubs, record);

      break;

    case OAK_TREE:
      DeleteEntityFromArray(state.oak_trees, record);

      break;

    case WILLOW_TREE:
      DeleteEntityFromArray(state.willow_trees, record);

      break;

    default:
      // @todo log error
      break;
  }
}