#include <variant>

#include "astro/entity_dispatcher.h"

#include "astro/entity_behaviors/Bandit.h"
#include "astro/entity_behaviors/LowGuard.h"
#include "astro/entity_behaviors/OakTree.h"
#include "astro/entity_behaviors/RiverLog.h"
#include "astro/entity_behaviors/Shrub.h"
#include "astro/entity_behaviors/SmallStoneBridge.h"
#include "astro/entity_behaviors/WillowTree.h"
#include "astro/entity_behaviors/WoodenGateDoor.h"

using namespace astro;

#define dispatch_GetEntityContainer(__entity_type, __entities)\
  case __entity_type:\
    return __entities\

#define dispatch_AddMeshes(__entity_type, __Behavior)\
  case __entity_type:\
    __Behavior::_AddMeshes(tachyon, state.meshes);\
    break\

#define dispatch_GetMeshes(__entity_type, __Behavior)\
  case __entity_type:\
    return __Behavior::_GetMeshes(state.meshes)\

#define dispatch_GetPlaceholderMesh(__entity_type, __Behavior)\
  case __entity_type:\
    return __Behavior::_GetPlaceholderMesh(state.meshes);

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
    dispatch_GetEntityContainer(RIVER_LOG, state.river_logs);
    dispatch_GetEntityContainer(LOW_GUARD, state.low_guards);
    dispatch_GetEntityContainer(BANDIT, state.bandits);

    default:
      // @todo log error
      printf("Failed to get container for entity type: %d\n", type);
      exit(0);
      break;
  }
}

void EntityDispatcher::AddMeshes(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    dispatch_AddMeshes(SHRUB, Shrub);
    dispatch_AddMeshes(OAK_TREE, OakTree);
    dispatch_AddMeshes(WILLOW_TREE, WillowTree);
    dispatch_AddMeshes(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_AddMeshes(WOODEN_GATE_DOOR, WoodenGateDoor);
    dispatch_AddMeshes(RIVER_LOG, RiverLog);
    dispatch_AddMeshes(LOW_GUARD, LowGuard);
    dispatch_AddMeshes(BANDIT, Bandit);

    default:
      printf("Failed to add meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

const std::vector<uint16>& EntityDispatcher::GetMeshes(State& state, EntityType type) {
  switch (type) {
    dispatch_GetMeshes(SHRUB, Shrub);
    dispatch_GetMeshes(OAK_TREE, OakTree);
    dispatch_GetMeshes(WILLOW_TREE, WillowTree);
    dispatch_GetMeshes(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_GetMeshes(WOODEN_GATE_DOOR, WoodenGateDoor);
    dispatch_GetMeshes(RIVER_LOG, RiverLog);
    dispatch_GetMeshes(LOW_GUARD, LowGuard);
    dispatch_GetMeshes(BANDIT, Bandit);

    default:
      printf("Failed to get meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

uint16 EntityDispatcher::GetPlaceholderMesh(State& state, EntityType type) {
  switch (type) {
    dispatch_GetPlaceholderMesh(SHRUB, Shrub);
    dispatch_GetPlaceholderMesh(OAK_TREE, OakTree);
    dispatch_GetPlaceholderMesh(WILLOW_TREE, WillowTree);
    dispatch_GetPlaceholderMesh(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_GetPlaceholderMesh(WOODEN_GATE_DOOR, WoodenGateDoor);
    dispatch_GetPlaceholderMesh(RIVER_LOG, RiverLog);
    dispatch_GetPlaceholderMesh(LOW_GUARD, LowGuard);
    dispatch_GetPlaceholderMesh(BANDIT, Bandit);

    default:
      printf("Failed to get placeholder mesh for entity type: %d\n", type);
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
    dispatch_TimeEvolve(RIVER_LOG, RiverLog);
    dispatch_TimeEvolve(LOW_GUARD, LowGuard);
    dispatch_TimeEvolve(BANDIT, Bandit);

    default:
      // @todo log error
      printf("Failed to time-evolve entity type: %d\n", type);
      exit(0);
      break;
  }
}