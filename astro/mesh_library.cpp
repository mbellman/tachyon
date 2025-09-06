#include "astro/mesh_library.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)

using namespace astro;

void astro::AddMeshes(Tachyon* tachyon, State& state) {
  state.meshes.cube = CUBE_MESH(1);
}