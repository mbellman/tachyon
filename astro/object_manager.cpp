#include "astro/object_manager.h"

using namespace astro;

void ObjectManager::CreateObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player);

  create(meshes.ground_plane);
  create(meshes.water_plane);
}

void ObjectManager::CreateObjectsForEntity(Tachyon* tachyon, State& state, EntityType type) {
  auto& meshes = state.meshes;

  switch (type) {
    case OAK_TREE: {
      create(meshes.oak_tree_trunk);

      break;
    };

    case WILLOW_TREE: {
      create(meshes.willow_tree_trunk);

      break;
    };

    default:
      // @todo log error
      break;
  }
}