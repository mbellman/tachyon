#include "astro/object_manager.h"

using namespace astro;

void astro::CreateObjects(Tachyon* tachyon, State& state) {
  create(state.meshes.cube);

  create(state.meshes.plane);
}