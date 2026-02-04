#include "astro/entity_dispatcher.h"

#include "astro/entity_behaviors/Altar.h"
#include "astro/entity_behaviors/Bandit.h"
#include "astro/entity_behaviors/BellFlower.h"
#include "astro/entity_behaviors/ChestnutTree.h"
#include "astro/entity_behaviors/DirtPathNode.h"
#include "astro/entity_behaviors/Flag.h"
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
#include "astro/entity_behaviors/RoseBush.h"
#include "astro/entity_behaviors/Sculpture_1.h"
#include "astro/entity_behaviors/Shrub.h"
#include "astro/entity_behaviors/SmallStoneBridge.h"
#include "astro/entity_behaviors/StarFlower.h"
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

#define create_dispatch_cases(dispatch_macro)\
  dispatch_macro(BANDIT, Bandit);\
  dispatch_macro(CHESTNUT_TREE, ChestnutTree);\
  dispatch_macro(STONE_PATH_NODE, StonePathNode);\
  dispatch_macro(DIRT_PATH_NODE, DirtPathNode);\
  dispatch_macro(FOG_SPAWN, FogSpawn);\
  dispatch_macro(ITEM_PICKUP, ItemPickup);\
  dispatch_macro(LESSER_GUARD, LesserGuard);\
  dispatch_macro(LOW_GUARD, LowGuard);\
  dispatch_macro(OAK_TREE, OakTree);\
  dispatch_macro(RIVER_LOG, RiverLog);\
  dispatch_macro(SHRUB, Shrub);\
  dispatch_macro(FLOWER_BUSH, FlowerBush);\
  dispatch_macro(LILAC_BUSH, LilacBush);\
  dispatch_macro(ROSE_BUSH, RoseBush);\
  dispatch_macro(BELLFLOWER, BellFlower);\
  dispatch_macro(STARFLOWER, StarFlower);\
  dispatch_macro(LILY_PAD, LilyPad);\
  dispatch_macro(MUSHROOM, Mushroom);\
  dispatch_macro(GLOW_FLOWER, GlowFlower);\
  dispatch_macro(SMALL_STONE_BRIDGE, SmallStoneBridge);\
  dispatch_macro(STONE_WALL, StoneWall);\
  dispatch_macro(GATE, Gate);\
  dispatch_macro(FLAG, Flag);\
  dispatch_macro(LIGHT_POST, LightPost);\
  dispatch_macro(LAMPPOST, Lamppost);\
  dispatch_macro(WIND_CHIMES, WindChimes);\
  dispatch_macro(SCULPTURE_1, Sculpture_1);\
  dispatch_macro(ALTAR, Altar);\
  dispatch_macro(HOUSE, House);\
  dispatch_macro(NPC, Npc);\
  dispatch_macro(WATER_WHEEL, WaterWheel);\
  dispatch_macro(WILLOW_TREE, WillowTree);\
  dispatch_macro(WOODEN_BRIDGE, WoodenBridge);\
  dispatch_macro(WOODEN_FENCE, WoodenFence);\
  dispatch_macro(WOODEN_GATE_DOOR, WoodenGateDoor);

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
    dispatch_GetEntityContainer(ROSE_BUSH, state.rose_bushes);
    dispatch_GetEntityContainer(BELLFLOWER, state.bellflowers);
    dispatch_GetEntityContainer(STARFLOWER, state.starflowers);
    dispatch_GetEntityContainer(LILY_PAD, state.lily_pads);
    dispatch_GetEntityContainer(MUSHROOM, state.mushrooms);
    dispatch_GetEntityContainer(GLOW_FLOWER, state.glow_flowers);
    dispatch_GetEntityContainer(SMALL_STONE_BRIDGE, state.small_stone_bridges);
    dispatch_GetEntityContainer(STONE_WALL, state.stone_walls);
    dispatch_GetEntityContainer(GATE, state.gates);
    dispatch_GetEntityContainer(FLAG, state.flags);
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