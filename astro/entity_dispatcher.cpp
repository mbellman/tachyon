#include "astro/entity_dispatcher.h"

#include "astro/entity_behaviors/Altar.h"
#include "astro/entity_behaviors/Bandit.h"
#include "astro/entity_behaviors/ChestnutTree.h"
#include "astro/entity_behaviors/DirtPathNode.h"
#include "astro/entity_behaviors/FlowerBush.h"
#include "astro/entity_behaviors/FogSpawn.h"
#include "astro/entity_behaviors/Gate.h"
#include "astro/entity_behaviors/GlowFlower.h"
#include "astro/entity_behaviors/House.h"
#include "astro/entity_behaviors/ItemPickup.h"
#include "astro/entity_behaviors/Lamppost.h"
#include "astro/entity_behaviors/LightPost.h"
#include "astro/entity_behaviors/Lilac_Bush.h"
#include "astro/entity_behaviors/LilyPad.h"
#include "astro/entity_behaviors/LesserGuard.h"
#include "astro/entity_behaviors/LowGuard.h"
#include "astro/entity_behaviors/Mushroom.h"
#include "astro/entity_behaviors/Npc.h"
#include "astro/entity_behaviors/OakTree.h"
#include "astro/entity_behaviors/RiverLog.h"
#include "astro/entity_behaviors/Sculpture_1.h"
#include "astro/entity_behaviors/Shrub.h"
#include "astro/entity_behaviors/SmallStoneBridge.h"
#include "astro/entity_behaviors/StonePathNode.h"
#include "astro/entity_behaviors/StoneWall.h"
#include "astro/entity_behaviors/WaterWheel.h"
#include "astro/entity_behaviors/WillowTree.h"
#include "astro/entity_behaviors/WindChimes.h"
#include "astro/entity_behaviors/WoodenFence.h"
#include "astro/entity_behaviors/WoodenBridge.h"
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
    return __Behavior::_GetPlaceholderMesh(state.meshes)

#define dispatch_TimeEvolve(__entity_type, __Behavior)\
  case __entity_type:\
    __Behavior::_TimeEvolve(tachyon, state);\
    break\

#define create_dispatch_cases(behavior_macro)\
  behavior_macro(BANDIT, Bandit);\
  behavior_macro(CHESTNUT_TREE, ChestnutTree);\
  behavior_macro(STONE_PATH_NODE, StonePathNode);\
  behavior_macro(DIRT_PATH_NODE, DirtPathNode);\
  behavior_macro(FOG_SPAWN, FogSpawn);\
  behavior_macro(ITEM_PICKUP, ItemPickup);\
  behavior_macro(LESSER_GUARD, LesserGuard);\
  behavior_macro(LOW_GUARD, LowGuard);\
  behavior_macro(OAK_TREE, OakTree);\
  behavior_macro(RIVER_LOG, RiverLog);\
  behavior_macro(SHRUB, Shrub);\
  behavior_macro(FLOWER_BUSH, FlowerBush);\
  behavior_macro(LILAC_BUSH, LilacBush);\
  behavior_macro(LILY_PAD, LilyPad);\
  behavior_macro(MUSHROOM, Mushroom);\
  behavior_macro(GLOW_FLOWER, GlowFlower);\
  behavior_macro(SMALL_STONE_BRIDGE, SmallStoneBridge);\
  behavior_macro(STONE_WALL, StoneWall);\
  behavior_macro(GATE, Gate);\
  behavior_macro(LIGHT_POST, LightPost);\
  behavior_macro(LAMPPOST, Lamppost);\
  behavior_macro(WIND_CHIMES, WindChimes);\
  behavior_macro(SCULPTURE_1, Sculpture_1);\
  behavior_macro(ALTAR, Altar);\
  behavior_macro(HOUSE, House);\
  behavior_macro(NPC, Npc);\
  behavior_macro(WATER_WHEEL, WaterWheel);\
  behavior_macro(WILLOW_TREE, WillowTree);\
  behavior_macro(WOODEN_BRIDGE, WoodenBridge);\
  behavior_macro(WOODEN_FENCE, WoodenFence);\
  behavior_macro(WOODEN_GATE_DOOR, WoodenGateDoor);

std::vector<GameEntity>& EntityDispatcher::GetEntityContainer(State& state, EntityType type) {
  switch (type) {
    dispatch_GetEntityContainer(BANDIT, state.bandits);
    dispatch_GetEntityContainer(CHESTNUT_TREE, state.chestnut_trees);
    dispatch_GetEntityContainer(STONE_PATH_NODE, state.stone_path_nodes);
    dispatch_GetEntityContainer(DIRT_PATH_NODE, state.dirt_path_nodes);
    dispatch_GetEntityContainer(ITEM_PICKUP, state.item_pickups);
    dispatch_GetEntityContainer(FOG_SPAWN, state.fog_spawns);
    dispatch_GetEntityContainer(LESSER_GUARD, state.lesser_guards);
    dispatch_GetEntityContainer(LOW_GUARD, state.low_guards);
    dispatch_GetEntityContainer(OAK_TREE, state.oak_trees);
    dispatch_GetEntityContainer(RIVER_LOG, state.river_logs);
    dispatch_GetEntityContainer(SHRUB, state.shrubs);
    dispatch_GetEntityContainer(FLOWER_BUSH, state.flower_bushes);
    dispatch_GetEntityContainer(LILAC_BUSH, state.lilac_bushes);
    dispatch_GetEntityContainer(LILY_PAD, state.lily_pads);
    dispatch_GetEntityContainer(MUSHROOM, state.mushrooms);
    dispatch_GetEntityContainer(GLOW_FLOWER, state.glow_flowers);
    dispatch_GetEntityContainer(SMALL_STONE_BRIDGE, state.small_stone_bridges);
    dispatch_GetEntityContainer(STONE_WALL, state.stone_walls);
    dispatch_GetEntityContainer(GATE, state.gates);
    dispatch_GetEntityContainer(LIGHT_POST, state.light_posts);
    dispatch_GetEntityContainer(LAMPPOST, state.lampposts);
    dispatch_GetEntityContainer(WIND_CHIMES, state.wind_chimes);
    dispatch_GetEntityContainer(SCULPTURE_1, state.sculpture_1s);
    dispatch_GetEntityContainer(ALTAR, state.altars);
    dispatch_GetEntityContainer(HOUSE, state.houses);
    dispatch_GetEntityContainer(NPC, state.npcs);
    dispatch_GetEntityContainer(WATER_WHEEL, state.water_wheels);
    dispatch_GetEntityContainer(WILLOW_TREE, state.willow_trees);
    dispatch_GetEntityContainer(WOODEN_BRIDGE, state.wooden_bridges);
    dispatch_GetEntityContainer(WOODEN_FENCE, state.wooden_fences);
    dispatch_GetEntityContainer(WOODEN_GATE_DOOR, state.wooden_gate_doors);

    default:
      printf("EntityDispatcher: Failed to get container for entity type: %d\n", type);
      exit(0);
      break;
  }
}

void EntityDispatcher::AddMeshes(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    create_dispatch_cases(dispatch_AddMeshes)

    default:
      printf("EntityDispatcher: Failed to add meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

const std::vector<uint16>& EntityDispatcher::GetMeshes(State& state, EntityType type) {
  switch (type) {
    create_dispatch_cases(dispatch_GetMeshes)

    default:
      printf("EntityDispatcher: Failed to get meshes for entity type: %d\n", type);
      exit(0);
      break;
  }
}

uint16 EntityDispatcher::GetPlaceholderMesh(State& state, EntityType type) {
  switch (type) {
    create_dispatch_cases(dispatch_GetPlaceholderMesh)

    default:
      printf("EntityDispatcher: Failed to get placeholder mesh for entity type: %d\n", type);
      exit(0);
      break;
  }
}

void EntityDispatcher::TimeEvolve(Tachyon* tachyon, State& state, EntityType type) {
  switch (type) {
    create_dispatch_cases(dispatch_TimeEvolve)

    default:
      printf("EntityDispatcher: Failed to time-evolve entity type: %d\n", type);
      exit(0);
      break;
  }
}