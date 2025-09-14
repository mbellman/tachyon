#include "astro/mesh_library.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

using namespace astro;

void MeshLibrary::AddMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.player = CUBE_MESH(1);

  meshes.ground_plane = PLANE_MESH(1);
  meshes.water_plane = PLANE_MESH(1);

  meshes.astrolabe_base = MODEL_MESH("./astro/models/astrolabe_base.obj", 1);
  meshes.astrolabe_ring = MODEL_MESH("./astro/models/astrolabe_ring.obj", 1);
  meshes.astrolabe_hand = MODEL_MESH("./astro/models/astrolabe_hand.obj", 1);

  mesh(meshes.astrolabe_base).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_ring).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_hand).shadow_cascade_ceiling = 0;

  meshes.shrub_branches = CUBE_MESH(100);
  meshes.oak_tree_trunk = CUBE_MESH(100);
  meshes.willow_tree_trunk = CUBE_MESH(100);
}