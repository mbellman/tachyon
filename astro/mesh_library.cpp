#include "astro/mesh_library.h"
#include "astro/entity_dispatcher.h"

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)
#define MODEL_MESH_LOD_2(lod_1_path, lod_2_path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(lod_1_path), Tachyon_LoadMesh(lod_2_path), total)

using namespace astro;

static void AddHUDMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.astrolabe_rear        = MODEL_MESH("./astro/3d_models/astrolabe_rear.obj", 1);
  meshes.astrolabe_base        = MODEL_MESH("./astro/3d_models/astrolabe_base.obj", 1);
  meshes.astrolabe_plate       = MODEL_MESH("./astro/3d_models/astrolabe_plate.obj", 1);
  meshes.astrolabe_fragment_ul = MODEL_MESH("./astro/3d_models/astrolabe_fragment_ul.obj", 1);
  meshes.astrolabe_fragment_ll = MODEL_MESH("./astro/3d_models/astrolabe_fragment_ll.obj", 1);
  meshes.astrolabe_ring        = MODEL_MESH("./astro/3d_models/astrolabe_ring.obj", 1);
  meshes.astrolabe_hand        = MODEL_MESH("./astro/3d_models/astrolabe_hand.obj", 1);
  meshes.target_reticle        = MODEL_MESH("./astro/3d_models/target_reticle.obj", 1);

  mesh(meshes.astrolabe_rear).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_base).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_plate).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_fragment_ul).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_fragment_ll).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_ring).shadow_cascade_ceiling = 0;
  mesh(meshes.astrolabe_hand).shadow_cascade_ceiling = 0;
  mesh(meshes.target_reticle).shadow_cascade_ceiling = 0;
}

static void AddDecorativeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.flat_ground   = PLANE_MESH(1000);
  meshes.rock_1        = MODEL_MESH("./astro/3d_models/rock_1.obj", 5000);
  meshes.river_edge    = MODEL_MESH("./astro/3d_models/river_edge.obj", 5000);
  meshes.ground_1      = MODEL_MESH("./astro/3d_models/ground_1.obj", 5000);
  meshes.lookout_tower = MODEL_MESH("./astro/3d_models/decoratives/lookout_tower.obj", 100);

  mesh(meshes.flat_ground).shadow_cascade_ceiling = 0;
  mesh(meshes.rock_1).shadow_cascade_ceiling = 2;
  mesh(meshes.river_edge).shadow_cascade_ceiling = 2;
  mesh(meshes.ground_1).shadow_cascade_ceiling = 2;
}

static void AddEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for_all_entity_types() {
    EntityDispatcher::AddMeshes(tachyon, state, type);
  }

  // @temporary
  // @todo define in entity behavior files
  mesh(meshes.dirt_path_placeholder).shadow_cascade_ceiling = 0;
  mesh(meshes.dirt_path).shadow_cascade_ceiling = 0;

  mesh(meshes.flower_bush_placeholder).type = GRASS_MESH;
  mesh(meshes.flower_bush_placeholder).shadow_cascade_ceiling = 2;
  mesh(meshes.flower_bush_leaves).type = GRASS_MESH;
  mesh(meshes.flower_bush_leaves).shadow_cascade_ceiling = 2;

  mesh(meshes.glow_flower_placeholder).type = GRASS_MESH;
  mesh(meshes.glow_flower_placeholder).shadow_cascade_ceiling = 2;
  mesh(meshes.glow_flower_petals).type = GRASS_MESH;
  mesh(meshes.glow_flower_petals).shadow_cascade_ceiling = 2;
}

static void AddItemMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.item_astro_part = CUBE_MESH(3);
  meshes.item_gate_key = MODEL_MESH("./astro/3d_models/gate_key.obj", 1);
}

static void AddProceduralMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // grass
  {
    meshes.grass = MODEL_MESH("./astro/3d_models/grass.obj", 30000);

    mesh(meshes.grass).type = GRASS_MESH;
    mesh(meshes.grass).shadow_cascade_ceiling = 2;
  }

  // small_grass
  {
    meshes.small_grass = MODEL_MESH_LOD_2("./astro/3d_models/small_grass.obj", "./astro/3d_models/small_grass_lod.obj", 50000);

    mesh(meshes.small_grass).type = GRASS_MESH;
    mesh(meshes.small_grass).shadow_cascade_ceiling = 2;
    mesh(meshes.small_grass).use_lowest_lod_for_shadows = true;
  }

  // ground_flower
  {
    meshes.ground_flower = MODEL_MESH("./astro/3d_models/flower.obj", 20000);

    mesh(meshes.ground_flower).type = GRASS_MESH;
    mesh(meshes.ground_flower).shadow_cascade_ceiling = 2;
  }

  // tiny_ground_flowers
  {
    meshes.tiny_ground_flower = MODEL_MESH("./astro/3d_models/tiny_flower.obj", 20000);

    mesh(meshes.tiny_ground_flower).type = GRASS_MESH;
    mesh(meshes.tiny_ground_flower).shadow_cascade_ceiling = 2;
  }

  // bush_flower
  {
    meshes.bush_flower   = MODEL_MESH("./astro/3d_models/flower.obj", 1000);

    mesh(meshes.bush_flower).type = GRASS_MESH;
    mesh(meshes.bush_flower).shadow_cascade_ceiling = 2;
  }

  // ground_1_leaves
  {
    meshes.ground_1_leaves = MODEL_MESH("./astro/3d_models/ground_1_leaves.obj", 200);

    mesh(meshes.ground_1_leaves).type = GRASS_MESH;
    mesh(meshes.ground_1_leaves).shadow_cascade_ceiling = 2;
  }

  // p_dirt_path
  // @todo rename
  {
    meshes.p_dirt_path   = MODEL_MESH("./astro/3d_models/dirt_path.obj", 10000);

    mesh(meshes.p_dirt_path).shadow_cascade_ceiling = 0;
  }
}

static void AddEditorMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.gizmo_arrow   = MODEL_MESH("./astro/3d_models/editor/gizmo_arrow.obj", 3);
  meshes.gizmo_resizer = MODEL_MESH("./astro/3d_models/editor/gizmo_resizer.obj", 3);
  meshes.gizmo_rotator = MODEL_MESH("./astro/3d_models/editor/gizmo_rotator.obj", 3);
  meshes.editor_placer = MODEL_MESH("./astro/3d_models/editor/placer.obj", 1);

  mesh(meshes.gizmo_arrow).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_resizer).shadow_cascade_ceiling = 0;
  mesh(meshes.gizmo_rotator).shadow_cascade_ceiling = 0;

  mesh(meshes.editor_placer).type = ION_THRUSTER_MESH;
  mesh(meshes.editor_placer).shadow_cascade_ceiling = 0;
}

void MeshLibrary::AddMeshes(Tachyon* tachyon, State& state) {
  log_time("AddMeshes()");

  auto& meshes = state.meshes;

  // @temporary
  meshes.player = MODEL_MESH("./astro/3d_models/guy.obj", 1);
  meshes.wand = MODEL_MESH("./astro/3d_models/wand.obj", 1);
  meshes.water_plane = PLANE_MESH(1);
  meshes.snow_particle = MODEL_MESH("./astro/3d_models/snow.obj", 100);

  mesh(meshes.water_plane).type = WATER_MESH;
  mesh(meshes.water_plane).shadow_cascade_ceiling = 0;
  mesh(meshes.snow_particle).shadow_cascade_ceiling = 0;

  AddHUDMeshes(tachyon, state);
  AddDecorativeMeshes(tachyon, state);
  AddEntityMeshes(tachyon, state);
  AddItemMeshes(tachyon, state);
  AddProceduralMeshes(tachyon, state);

  // @todo dev mode only
  AddEditorMeshes(tachyon, state);
}