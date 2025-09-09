#include "astro/object_manager.h"

using namespace astro;

void astro::CreateObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player);

  create(meshes.ground_plane);
  create(meshes.water_plane);
}