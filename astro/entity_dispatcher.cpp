#include <variant>

#include "astro/entity_dispatcher.h"

#include "astro/entity_descriptions/OakTree.h"
#include "astro/entity_descriptions/Shrub.h"

using namespace astro;

#define dispatch_GetAllEntitiesOfType(__type, __entities)\
  case __type:\
    return __entities\

#define dispatch_SpawnPlaceholder(__type, __description)\
  case __type:\
    return __description::_SpawnPlaceholder(tachyon, state, entity)\

#define dispatch_DestroyPlaceholders(__type, __description)\
  case __type:\
    __description::_DestroyPlaceholders(tachyon, state);\
    break\

const std::vector<EntityType>& EntityDispatcher::GetAllEntityTypes() {
  const static std::vector<EntityType> entity_types = {
    SHRUB,
    OAK_TREE,
    WILLOW_TREE
  };

  return entity_types;
}

std::vector<GameEntity>& EntityDispatcher::GetAllEntitiesOfType(State& state, EntityType type) {
  switch (type) {
    dispatch_GetAllEntitiesOfType(SHRUB, state.shrubs);
    dispatch_GetAllEntitiesOfType(OAK_TREE, state.oak_trees);
    dispatch_GetAllEntitiesOfType(WILLOW_TREE, state.willow_trees);

    default:
      // @todo log error
      exit(0);
      break;
  }
}

tObject& EntityDispatcher::SpawnPlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity) {
  switch (entity.type) {
    dispatch_SpawnPlaceholder(SHRUB, Shrub);
    dispatch_SpawnPlaceholder(OAK_TREE, OakTree);

    case WILLOW_TREE:
      // @todo
      break;

    default:
      // @todo log error
      exit(0);
      break;
  }
}

void EntityDispatcher::DestroyPlaceholders(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    dispatch_DestroyPlaceholders(SHRUB, Shrub);
    dispatch_DestroyPlaceholders(OAK_TREE, OakTree);

    case WILLOW_TREE:
      // @todo
      break;

    default:
      // @todo log error
      exit(0);
      break;
  }
}