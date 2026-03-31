#include "astro/facade_geometry.h"

using namespace astro;

// @todo!!!!!!!!!!!!
void FacadeGeometry::HandleFacades(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  if (!state.use_vantage_camera) {
    // objects(meshes.facade_sg_castle).disabled = true;

    return;
  }

  // objects(meshes.facade_sg_castle).disabled = false;
}