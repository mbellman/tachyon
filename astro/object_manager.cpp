#include "astro/object_manager.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

static void RemoveLastObject(Tachyon* tachyon, uint16 mesh_index) {
  auto& objects = objects(mesh_index);
  uint16 total_active = objects.total_active;

  if (total_active > 0) {
    auto& last_object = objects[total_active - 1];
  
    remove(last_object);
  }
}

void ObjectManager::CreateObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player);

  create(meshes.ground_plane);
  create(meshes.water_plane);

  create(meshes.astrolabe_base);
  create(meshes.astrolabe_ring);
  create(meshes.astrolabe_hand);
}

// @todo replace with EntityDispatcher::DestroyObjects()
void ObjectManager::DeleteObjectsForEntityType(Tachyon* tachyon, State& state, EntityType type) {
  auto& meshes = state.meshes;

  switch (type) {
    case SHRUB: {
      RemoveLastObject(tachyon, meshes.shrub_branches);

      break;
    }

    case OAK_TREE: {
      RemoveLastObject(tachyon, meshes.oak_tree_trunk);

      break;
    };

    case WILLOW_TREE: {
      RemoveLastObject(tachyon, meshes.willow_tree_trunk);

      break;
    };

    default:
      // @todo log error
      break;
  }
}

void ObjectManager::ProvisionAvailableObjectsForEntities(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize determine on-screen/in-range entities
  // and use reduced-fidelity object groups, or single objects,
  // for more distant entities

  // @todo refactor
  for_entities(state.shrubs) {
    auto& shrub = state.shrubs[i];
    auto& branches = objects(meshes.shrub_branches)[i];

    branches.position = shrub.position;
    branches.rotation = shrub.orientation;
    branches.color = shrub.tint;
  }

  // @todo refactor
  for_entities(state.oak_trees) {
    auto& tree = state.oak_trees[i];
    auto& trunk = objects(meshes.oak_tree_trunk)[i];

    trunk.position = tree.position;
    trunk.rotation = tree.orientation;
    trunk.color = tree.tint;
  }

  // @todo refactor
  for_entities(state.willow_trees) {
    auto& tree = state.oak_trees[i];
    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.position = tree.position;
    trunk.rotation = tree.orientation;
    trunk.color = tree.tint;
  }
}