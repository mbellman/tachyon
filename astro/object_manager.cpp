#include "astro/object_manager.h"

using namespace astro;

// @todo remove
void ObjectManager::CreateObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player);

  create(meshes.water_plane);

  create(meshes.astrolabe_rear);
  create(meshes.astrolabe_base);
  create(meshes.astrolabe_ring);
  create(meshes.astrolabe_hand);
  create(meshes.target_reticle);
}