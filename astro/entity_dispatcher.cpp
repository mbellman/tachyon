#include <variant>

#include "astro/entity_dispatcher.h"

#include "astro/entity_behaviors/Bandit.h"
#include "astro/entity_behaviors/DirtPath.h"
#include "astro/entity_behaviors/Flowers.h"
#include "astro/entity_behaviors/ItemPickup.h"
#include "astro/entity_behaviors/LowGuard.h"
#include "astro/entity_behaviors/OakTree.h"
#include "astro/entity_behaviors/RiverLog.h"
#include "astro/entity_behaviors/Shrub.h"
#include "astro/entity_behaviors/SmallStoneBridge.h"
#include "astro/entity_behaviors/StoneWall.h"
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
    __Behavior::_TimeEvolve(tachyon, state, dt);\
    break\

std::vector<GameEntity>& EntityDispatcher::GetEntityContainer(State& state, EntityType type) {
  switch (type) {
    dispatch_GetEntityContainer(BANDIT, state.bandits);
    dispatch_GetEntityContainer(DIRT_PATH, state.dirt_paths);
    dispatch_GetEntityContainer(ITEM_PICKUP, state.item_pickups);
    dispatch_GetEntityContainer(LOW_GUARD, state.low_guards);
    dispatch_GetEntityContainer(OAK_TREE, state.oak_trees);
    dispatch_GetEntityContainer(RIVER_LOG, state.river_logs);
    dispatch_GetEntityContainer(SHRUB, state.shrubs);
    dispatch_GetEntityContainer(FLOWERS, state.flowers);
    dispatch_GetEntityContainer(SMALL_STONE_BRIDGE, state.small_stone_bridges);
    dispatch_GetEntityContainer(STONE_WALL, state.stone_walls);
    dispatch_GetEntityContainer(WILLOW_TREE, state.willow_trees);
    dispatch_GetEntityContainer(WOODEN_GATE_DOOR, state.wooden_gate_doors);

    default:
      // @todo log error
      printf("EntityDispatcher: Failed to get container for entity type: %d\n", type);
      exit(0);
      break;
  }
}

void EntityDispatcher::AddMeshes(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    dispatch_AddMeshes(BANDIT, Bandit);
    dispatch_AddMeshes(DIRT_PATH, DirtPath);
    dispatch_AddMeshes(ITEM_PICKUP, ItemPickup);
    dispatch_AddMeshes(LOW_GUARD, LowGuard);
    dispatch_AddMeshes(OAK_TREE, OakTree);
    dispatch_AddMeshes(RIVER_LOG, RiverLog);
    dispatch_AddMeshes(SHRUB, Shrub);
    dispatch_AddMeshes(FLOWERS, Flowers);
    dispatch_AddMeshes(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_AddMeshes(STONE_WALL, StoneWall);
    dispatch_AddMeshes(WILLOW_TREE, WillowTree);
    dispatch_AddMeshes(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      printf("EntityDispatcher: Failed to add meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

const std::vector<uint16>& EntityDispatcher::GetMeshes(State& state, EntityType type) {
  switch (type) {
    dispatch_GetMeshes(BANDIT, Bandit);
    dispatch_GetMeshes(DIRT_PATH, DirtPath);
    dispatch_GetMeshes(ITEM_PICKUP, ItemPickup);
    dispatch_GetMeshes(LOW_GUARD, LowGuard);
    dispatch_GetMeshes(OAK_TREE, OakTree);
    dispatch_GetMeshes(RIVER_LOG, RiverLog);
    dispatch_GetMeshes(SHRUB, Shrub);
    dispatch_GetMeshes(FLOWERS, Flowers);
    dispatch_GetMeshes(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_GetMeshes(STONE_WALL, StoneWall);
    dispatch_GetMeshes(WILLOW_TREE, WillowTree);
    dispatch_GetMeshes(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      printf("EntityDispatcher: Failed to get meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

uint16 EntityDispatcher::GetPlaceholderMesh(State& state, EntityType type) {
  switch (type) {
    dispatch_GetPlaceholderMesh(BANDIT, Bandit);
    dispatch_GetPlaceholderMesh(DIRT_PATH, DirtPath);
    dispatch_GetPlaceholderMesh(ITEM_PICKUP, ItemPickup);
    dispatch_GetPlaceholderMesh(LOW_GUARD, LowGuard);
    dispatch_GetPlaceholderMesh(OAK_TREE, OakTree);
    dispatch_GetPlaceholderMesh(RIVER_LOG, RiverLog);
    dispatch_GetPlaceholderMesh(SHRUB, Shrub);
    dispatch_GetPlaceholderMesh(FLOWERS, Flowers);
    dispatch_GetPlaceholderMesh(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_GetPlaceholderMesh(STONE_WALL, StoneWall);
    dispatch_GetPlaceholderMesh(WILLOW_TREE, WillowTree);
    dispatch_GetPlaceholderMesh(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      printf("EntityDispatcher: Failed to get placeholder mesh for entity type: %d\n", type);
      exit(0);
      break;
  }
}

void EntityDispatcher::TimeEvolve(Tachyon* tachyon, State& state, EntityType type, const float dt) {
  switch (type) {
    dispatch_TimeEvolve(BANDIT, Bandit);
    dispatch_TimeEvolve(DIRT_PATH, DirtPath);
    dispatch_TimeEvolve(ITEM_PICKUP, ItemPickup);
    dispatch_TimeEvolve(LOW_GUARD, LowGuard);
    dispatch_TimeEvolve(OAK_TREE, OakTree);
    dispatch_TimeEvolve(RIVER_LOG, RiverLog);
    dispatch_TimeEvolve(SHRUB, Shrub);
    dispatch_TimeEvolve(FLOWERS, Flowers);
    dispatch_TimeEvolve(SMALL_STONE_BRIDGE, SmallStoneBridge);
    dispatch_TimeEvolve(STONE_WALL, StoneWall);
    dispatch_TimeEvolve(WILLOW_TREE, WillowTree);
    dispatch_TimeEvolve(WOODEN_GATE_DOOR, WoodenGateDoor);

    default:
      printf("EntityDispatcher: Failed to time-evolve entity type: %d\n", type);
      exit(0);
      break;
  }
}