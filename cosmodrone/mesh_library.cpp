#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

static std::vector<MeshAsset> mesh_assets;

// @bug once in a while, meshes don't load in at the beginning! figure this out
static void LoadShipPartMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo define in a list
  auto hull_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/hull.obj");
  auto streams_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/streams.obj");
  auto thrusters_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/thrusters.obj");
  auto trim_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/trim.obj");

  meshes.hull = Tachyon_AddMesh(tachyon, hull_mesh, 1);
  meshes.streams = Tachyon_AddMesh(tachyon, streams_mesh, 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, thrusters_mesh, 1);
  meshes.trim = Tachyon_AddMesh(tachyon, trim_mesh, 1);
}

static void LoadStationPartMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo define in a list
  meshes.module_1 = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/module_1.obj"), 1000);
  meshes.torus_1 = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/torus_1.obj"), 1000);
  meshes.solar_panel_1 = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/solar_panel_1.obj"), 1000);

  // @todo refactor
  mesh_assets.push_back({
    .mesh_name = "module_1",
    .mesh_index = meshes.module_1
  });

  mesh_assets.push_back({
    .mesh_name = "torus_1",
    .mesh_index = meshes.torus_1
  });

  mesh_assets.push_back({
    .mesh_name = "solar_panel_1",
    .mesh_index = meshes.solar_panel_1
  });
}

static void LoadCelestialBodyMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo have separate meshes for each celestial body
  // @todo define a list of celestial bodies + properties
  auto planet_mesh = Tachyon_CreateSphereMesh(30);

  meshes.planet = Tachyon_AddMesh(tachyon, planet_mesh, 2);
}

static void LoadDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // auto sphere_mesh = Tachyon_CreateSphereMesh(4);
  auto cube_mesh = Tachyon_CreateCubeMesh();

  // meshes.sphere = Tachyon_AddMesh(tachyon, sphere_mesh, 40 * 40 * 40);
  meshes.cube = Tachyon_AddMesh(tachyon, cube_mesh, 6);
  meshes.editor_guideline = Tachyon_AddMesh(tachyon, cube_mesh, 3000);

  // @todo description
  {
    auto position_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/position-action-indicator.obj");
    auto rotation_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/rotate-action-indicator.obj");
    auto scale_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/scale-action-indicator.obj");

    meshes.editor_position = add_mesh(position_mesh, 1);
    meshes.editor_rotation = add_mesh(rotation_mesh, 1);
    meshes.editor_scale = add_mesh(scale_mesh, 1);
  }
}

void MeshLibrary::LoadMeshes(Tachyon* tachyon, State& state) {
  LoadShipPartMeshes(tachyon, state);
  LoadStationPartMeshes(tachyon, state);
  LoadCelestialBodyMeshes(tachyon, state);
  LoadDebugMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);
}

const std::vector<MeshAsset>& MeshLibrary::GetPlaceableMeshAssets() {
  return mesh_assets;
}