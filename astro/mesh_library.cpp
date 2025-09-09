#include "astro/mesh_library.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

using namespace astro;

void astro::AddMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.player = CUBE_MESH(1);

  meshes.ground_plane = PLANE_MESH(1);
  meshes.water_plane = PLANE_MESH(1);
}