#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

static std::vector<MeshAsset> placeable_mesh_assets;

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

static void LoadPlaceableMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" #__name ".obj"), 1000)

  auto& meshes = state.meshes;

  load_mesh(antenna_1);
  load_mesh(antenna_2);
  load_mesh(module_1);
  load_mesh(module_2);
  load_mesh(habitation_1);
  load_mesh(habitation_2);
  load_mesh(silo_2);
  load_mesh(silo_3);
  load_mesh(silo_4);
  load_mesh(torus_1);
  load_mesh(station_torus_1);
  load_mesh(station_base);
  load_mesh(solar_panel_1);
  load_mesh(solar_panel_2);
  load_mesh(girder_1);
  load_mesh(girder_2);
  load_mesh(girder_3);
  load_mesh(girder_4);
  load_mesh(girder_5);
  load_mesh(girder_6);
  load_mesh(track_1);

  // @todo refactor
  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_1",
    .mesh_index = meshes.antenna_1,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_2",
    .mesh_index = meshes.antenna_2,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "module_1",
    .mesh_index = meshes.module_1,
    .defaults = {
      .scale = tVec3f(1000.f),
      .color = tVec3f(1.f, 0.7f, 0.2f),
      .material = tVec4f(0.6f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "module_2",
    .mesh_index = meshes.module_2,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(0.4f),
      .material = tVec4f(0.9f, 0, 0, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_1",
    .mesh_index = meshes.habitation_1,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_2",
    .mesh_index = meshes.habitation_2,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_2",
    .mesh_index = meshes.silo_2,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f, 0.7f, 0.6f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_3",
    .mesh_index = meshes.silo_3,
    .defaults = {
      .scale = tVec3f(3000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_4",
    .mesh_index = meshes.silo_4,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.8f, 1.f, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "torus_1",
    .mesh_index = meshes.torus_1
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_1",
    .mesh_index = meshes.station_torus_1,
    .defaults = {
      .scale = tVec3f(100000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_base",
    .mesh_index = meshes.station_base,
    .defaults = {
      .scale = tVec3f(80000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "solar_panel_1",
    .mesh_index = meshes.solar_panel_1
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "solar_panel_2",
    .mesh_index = meshes.solar_panel_2,
    .defaults = {
      .scale = tVec3f(5000.f),
      .color = 0x28F1,
      .material = tVec4f(0.2f, 1.f, 0.3f, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_1",
    .mesh_index = meshes.girder_1,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_2",
    .mesh_index = meshes.girder_2,
    .defaults = {
      .scale = tVec3f(4000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_3",
    .mesh_index = meshes.girder_3,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_4",
    .mesh_index = meshes.girder_4,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.2f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_5",
    .mesh_index = meshes.girder_5,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.8f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_6",
    .mesh_index = meshes.girder_6,
    .defaults = {
      .scale = tVec3f(20000.f),
      .color = tVec3f(0.6, 0.1f, 0.1f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "track_1",
    .mesh_index = meshes.track_1,
    .defaults = {
      .scale = tVec3f(12000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.2f, 0, 0.2f, 0.2f)
    }
  });
}

static void LoadGeneratedMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/generated/" #__name ".obj"), 1000)

  auto& meshes = state.meshes;

  load_mesh(antenna_2_frame);
  load_mesh(antenna_2_receivers);
  load_mesh(girder_6_core);
  load_mesh(girder_6_frame);
  load_mesh(module_2_core);
  load_mesh(module_2_frame);
}

static void LoadBackgroundMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo have separate meshes for each celestial body
  // @todo define a list of celestial bodies + properties
  auto planet_mesh = Tachyon_CreateSphereMesh(40);

  meshes.planet = Tachyon_AddMesh(tachyon, planet_mesh, 2);
  meshes.space_elevator = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/space-elevator.obj"), 1);
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
  LoadPlaceableMeshes(tachyon, state);
  LoadGeneratedMeshes(tachyon, state);
  LoadBackgroundMeshes(tachyon, state);
  LoadDebugMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);
}

const std::vector<MeshAsset>& MeshLibrary::GetPlaceableMeshAssets() {
  return placeable_mesh_assets;
}