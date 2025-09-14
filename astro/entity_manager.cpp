#include "astro/entity_manager.h"

using namespace astro;

static int32 running_entity_id = 0;

static EntityRecord CreateTreeEntity(State& state, EntityType type) {
  TreeEntity tree;
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
  PlantEntity plant;
  plant.type = type;
  plant.id = running_entity_id++;

  if (type == SHRUB) {
    state.shrubs.push_back(plant);
  }

  return { plant.id, type };
}

template<class T>
static BaseEntity* FindEntityFromArray(std::vector<T>& array, const EntityRecord& record) {
  for (auto& entity : array) {
    if (entity.id == record.id) {
      return &entity;
    }
  }

  return nullptr;
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
      return {
        .id = -1,
        .type = UNSPECIFIED
      };
  }
}

BaseEntity* EntityManager::FindEntity(State& state, const EntityRecord& record) {
  switch (record.type) {
    case SHRUB:
      return FindEntityFromArray(state.shrubs, record);
    case OAK_TREE:
      return FindEntityFromArray(state.oak_trees, record);
    case WILLOW_TREE:
      return FindEntityFromArray(state.willow_trees, record);
    default:
      return nullptr;
  }
}