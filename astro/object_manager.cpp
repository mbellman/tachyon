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

void ObjectManager::ProvisionAvailableObjectsForEntities(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize determine on-screen/in-range entities
  // and use reduced-fidelity object groups, or single objects,
  // for more distant entities

  // @todo refactor
  for_entities(state.willow_trees) {
    auto& tree = state.oak_trees[i];
    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.position = tree.position;
    trunk.rotation = tree.orientation;
    trunk.color = tree.tint;
  }
}