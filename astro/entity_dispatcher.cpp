#include <variant>

#include "astro/entity_dispatcher.h"

#include "astro/entity_behaviors/OakTree.h"
#include "astro/entity_behaviors/Shrub.h"
#include "astro/entity_behaviors/SmallStoneBridge.h"
#include "astro/entity_behaviors/WillowTree.h"
#include "astro/entity_behaviors/WoodenGateDoor.h"

using namespace astro;

#define dispatch_GetEntityContainer(__entity_type, __entities)\
  case __entity_type:\
    return __entities\

#define dispatch_SpawnObjects(__entity_type, __Behavior)\
  case __entity_type:\
    return __Behavior::_SpawnObjects(tachyon, state)\

#define dispatch_DestroyObjects(__entity_type, __Behavior)\
  case __entity_type:\
    __Behavior::_DestroyObjects(tachyon, state);\
    break\

#define dispatch_CreatePlaceholder(__entity_type, __Behavior, __entity)\
  case __entity_type:\
    return __Behavior::_CreatePlaceholder(tachyon, state, __entity)\

#define dispatch_DestroyPlaceholders(__entity_type, __Behavior)\
  case __entity_type:\
    __Behavior::_DestroyPlaceholders(tachyon, state);\
    break\

#define dispatch_TimeEvolve(__entity_type, __Behavior)\
  case __entity_type:\
    __Behavior::_TimeEvolve(tachyon, state);\
    break\

std::vector<GameEntity>& EntityDispatcher::GetEntityContainer(State& state, EntityType type) {
  switch (type) {
    dispatch_GetEntityContainer(SHRUB, state.shrubs);
    dispatch_GetEntityContainer(OAK_TREE, state.oak_trees);
    dispatch_GetEntityContainer(WILLOW_TREE, state.willow_trees);
    dispatch_GetEntityContainer(SMALL_STONE_BRIDGE, state.small_stone_bridges);
    dispatch_GetEntityContainer(WOODEN_GATE_DOOR, state.wooden_gate_doors);

    default:
      // @todo log error
      exit(0);
      break;
  }
}

void EntityDispatcher::SpawnObjects(Tachyon* tachyon, State& state, const GameEntity& entity) {
  switch (entity.type) {
    dispatch_SpawnObjects(SHRUB, Shrub);
    dispatch_SpawnObjects(OAK_TREE, OakTree);
    dispatch_SpawnObjects(WILLOW_TREE, WillowTree);
    dispatch_SpawnObjects(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_SpawnObjects(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      // @todo log error
      exit(0);
      break;
  }
}

void EntityDispatcher::DestroyObjects(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    dispatch_DestroyObjects(SHRUB, Shrub);
    dispatch_DestroyObjects(OAK_TREE, OakTree);
    dispatch_DestroyObjects(WILLOW_TREE, WillowTree);
    dispatch_DestroyObjects(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_DestroyObjects(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      // @todo log error
      exit(0);
      break;
  }
}

tObject& EntityDispatcher::CreatePlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity) {
  switch (entity.type) {
    dispatch_CreatePlaceholder(SHRUB, Shrub, entity);
    dispatch_CreatePlaceholder(OAK_TREE, OakTree, entity);
    dispatch_CreatePlaceholder(WILLOW_TREE, WillowTree, entity);
    dispatch_CreatePlaceholder(SMALL_STONE_BRIDGE, SmallStoneBridge, entity);
    dispatch_CreatePlaceholder(WOODEN_GATE_DOOR, WoodenGateDoor, entity);

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
    dispatch_DestroyPlaceholders(WILLOW_TREE, WillowTree);
    dispatch_DestroyPlaceholders(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_DestroyPlaceholders(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      // @todo log error
      exit(0);
      break;
  }
}

void EntityDispatcher::TimeEvolve(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    dispatch_TimeEvolve(SHRUB, Shrub);
    dispatch_TimeEvolve(OAK_TREE, OakTree);
    dispatch_TimeEvolve(WILLOW_TREE, WillowTree);
    dispatch_TimeEvolve(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_TimeEvolve(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      // @todo log error
      exit(0);
      break;
  }
}