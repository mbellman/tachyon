#include "astro/object_manager.h"

using namespace astro;

// @todo remove
void ObjectManager::CreateObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player);
  create(meshes.wand);

  create(meshes.water_plane);

  for (uint16 i = 0; i < 100; i++) {
    create(meshes.snow_particle);
  }

  create(meshes.astrolabe_rear);
  create(meshes.astrolabe_base);
  create(meshes.astrolabe_plate);
  create(meshes.astrolabe_fragment_ul);
  create(meshes.astrolabe_fragment_ll);
  create(meshes.astrolabe_ring);
  create(meshes.astrolabe_hand);

  create(meshes.item_gate_key);

  create(meshes.target_reticle);
}