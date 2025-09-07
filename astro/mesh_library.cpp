#include "astro/mesh_library.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

using namespace astro;

void astro::AddMeshes(Tachyon* tachyon, State& state) {
  state.meshes.cube = CUBE_MESH(1);
  state.meshes.plane = PLANE_MESH(1);
}