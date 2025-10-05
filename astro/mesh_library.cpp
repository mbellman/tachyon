#include "astro/mesh_library.h"
#include "astro/entity_dispatcher.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

using namespace astro;

static void AddHUDMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.astrolabe_base = MODEL_MESH("./astro/3d_models/astrolabe_base.obj", 1);
  meshes.astrolabe_ring = MODEL_MESH("./astro/3d_models/astrolabe_ring.obj", 1);
  meshes.astrolabe_hand = MODEL_MESH("./astro/3d_models/astrolabe_hand.obj", 1);

  mesh(meshes.astrolabe_base).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_ring).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_hand).shadow_cascade_ceiling = 0;
}

static void AddDecorativeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.flat_ground = PLANE_MESH(1000);
  meshes.rock_1 = MODEL_MESH("./astro/3d_models/rock_1.obj", 5000);
  meshes.ground_1 = MODEL_MESH("./astro/3d_models/ground_1.obj", 5000);
}

static void AddEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for_all_entity_types() {
    EntityDispatcher::AddMeshes(tachyon, state, type);
  }
}

static void AddEditorMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.gizmo_arrow = MODEL_MESH("./astro/3d_models/editor/gizmo_arrow.obj", 3);
  meshes.gizmo_resizer = MODEL_MESH("./astro/3d_models/editor/gizmo_resizer.obj", 3);
  meshes.gizmo_rotator = MODEL_MESH("./astro/3d_models/editor/gizmo_rotator.obj", 3);

  mesh(meshes.gizmo_arrow).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_resizer).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_rotator).shadow_cascade_ceiling = 0;
}

void MeshLibrary::AddMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @temporary
  meshes.player = CUBE_MESH(1);
  meshes.water_plane = PLANE_MESH(1);

  // @temporary
  meshes.grass = MODEL_MESH("./astro/3d_models/grass.obj", 5000);
  meshes.small_grass = MODEL_MESH("./astro/3d_models/grass.obj", 10000);

  mesh(meshes.grass).type = GRASS_MESH;
  mesh(meshes.grass).shadow_cascade_ceiling = 2;
  mesh(meshes.small_grass).type = GRASS_MESH;
  mesh(meshes.small_grass).shadow_cascade_ceiling = 0;

  AddHUDMeshes(tachyon, state);
  AddDecorativeMeshes(tachyon, state);
  AddEntityMeshes(tachyon, state);

  // @todo dev mode only
  AddEditorMeshes(tachyon, state);
}