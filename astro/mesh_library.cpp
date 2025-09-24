#include "astro/mesh_library.h"

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

  // @todo factor
  meshes.shrub_branches = CUBE_MESH(100);
  meshes.oak_tree_trunk = CUBE_MESH(100);
  meshes.willow_tree_trunk = CUBE_MESH(100);
  meshes.small_stone_bridge_base = MODEL_MESH("./astro/3d_models/small_stone_bridge/base.obj", 100);
  meshes.small_stone_bridge_columns = MODEL_MESH("./astro/3d_models/small_stone_bridge/columns.obj", 100);
  meshes.wooden_gate_door = MODEL_MESH("./astro/3d_models/wooden_gate_door/door.obj", 100);
  meshes.river_log = MODEL_MESH("./astro/3d_models/river_log/log.obj", 100);

  // @todo factor
  meshes.shrub_placeholder = CUBE_MESH(100);
  meshes.oak_tree_placeholder = CUBE_MESH(100);
  meshes.willow_tree_placeholder = CUBE_MESH(100);
  meshes.small_stone_bridge_placeholder = MODEL_MESH("./astro/3d_models/small_stone_bridge/placeholder.obj", 100);
  meshes.wooden_gate_door_placeholder = MODEL_MESH("./astro/3d_models/wooden_gate_door/placeholder.obj", 100);
  meshes.river_log_placeholder = MODEL_MESH("./astro/3d_models/river_log/log.obj", 100);
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

  meshes.player = CUBE_MESH(1);

  meshes.water_plane = PLANE_MESH(1);

  AddHUDMeshes(tachyon, state);
  AddDecorativeMeshes(tachyon, state);
  AddEntityMeshes(tachyon, state);

  // @todo dev mode only
  AddEditorMeshes(tachyon, state);
}