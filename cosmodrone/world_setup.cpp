#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

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

  auto sphere_mesh = Tachyon_CreateSphereMesh(4);
  auto cube_mesh = Tachyon_CreateCubeMesh();

  // meshes.sphere = Tachyon_AddMesh(tachyon, sphere_mesh, 40 * 40 * 40);
  meshes.cube = Tachyon_AddMesh(tachyon, cube_mesh, 6);
  meshes.editor_guideline = Tachyon_AddMesh(tachyon, cube_mesh, 3000);
}

static void SetupFlightSimLevel(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.planet);
  create(meshes.planet);

  // @todo improve ship part handling
  {
    auto& hull = create(meshes.hull);
    auto& streams = create(meshes.streams);
    auto& thrusters = create(meshes.thrusters);
    auto& trim = create(meshes.trim);

    hull.scale = 200.f;
    hull.material = tVec4f(0.8f, 1.f, 0.2f, 0);

    streams.scale = 200.f;
    streams.material = tVec4f(0.6f, 0, 0, 1.f);

    thrusters.scale = 200.f;
    thrusters.color = tVec3f(0.2f);
    thrusters.material = tVec4f(0.8f, 0, 0, 0.4f);

    trim.scale = 200.f;
    trim.material = tVec4f(0.2f, 1.f, 0, 0);

    commit(hull);
    commit(streams);
    commit(thrusters);
    commit(trim);
  }
}

static void CreateDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);

  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);
}

static void CreateEditorGuidelines(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  const float width = 500000.f;
  const float height = 1000000.f;
  const float thickness = 50.f;

  // Vertical lines
  for (uint32 i = 0; i < 10; i++) {
    for (uint32 j = 0; j < 10; j++) {
      auto& line = create(meshes.editor_guideline);

      auto x = -width + (width / 5.f) * float(i);
      auto z = -width + (width / 5.f) * float(j);

      line.scale = tVec3f(thickness, height, thickness);
      line.color = tVec4f(1.f, 0, 0, 0.3f);
      line.position = tVec3f(x, 0, z);

      commit(line);
    }
  }

  // Planes
  const float y_increment = height / 25.f;
  const float w_increment = width / 5.f;

  for (uint32 i = 0; i < 50; i++) {
    auto y = -height + y_increment * float(i);

    for (uint32 j = 0; j < 10; j++) {
      auto& line = create(meshes.editor_guideline);

      line.scale = tVec3f(width, thickness, thickness);
      line.position = tVec3f(0, y, -width + w_increment * float(j));
      line.color = tVec4f(1.f, 0, 0, 0.3f);

      commit(line);
    }

    for (uint32 j = 0; j < 10; j++) {
      auto& line = create(meshes.editor_guideline);

      line.scale = tVec3f(thickness, thickness, width);
      line.position = tVec3f(-width + w_increment * float(j), y, 0);
      line.color = tVec4f(1.f, 0, 0, 0.3f);

      commit(line);
    }
  }
}

void WorldSetup::LoadMeshes(Tachyon* tachyon, State& state) {
  LoadShipPartMeshes(tachyon, state);
  LoadStationPartMeshes(tachyon, state);
  LoadCelestialBodyMeshes(tachyon, state);
  LoadDebugMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);
}

void WorldSetup::InitializeGameWorld(Tachyon* tachyon, State& state) {
  SetupFlightSimLevel(tachyon, state);
  CreateDebugMeshes(tachyon, state);
  CreateEditorGuidelines(tachyon, state);
}