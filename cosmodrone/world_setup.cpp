#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

static void LoadShipPartMeshes(Tachyon* tachyon, MeshIds& meshes) {
  // @todo define a list for these
  auto hull_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/hull.obj");
  auto streams_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/streams.obj");
  auto thrusters_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/thrusters.obj");
  auto trim_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/trim.obj");

  meshes.hull = Tachyon_AddMesh(tachyon, hull_mesh, 1);
  meshes.streams = Tachyon_AddMesh(tachyon, streams_mesh, 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, thrusters_mesh, 1);
  meshes.trim = Tachyon_AddMesh(tachyon, trim_mesh, 1);
}

static void LoadCelestialBodyMeshes(Tachyon* tachyon, MeshIds& meshes) {
  // @todo have separate meshes for each celestial body
  // @todo define a list of celestial bodies + properties
  auto planet_mesh = Tachyon_CreateSphereMesh(24);

  meshes.planet = Tachyon_AddMesh(tachyon, planet_mesh, 3);
}

static void LoadDebugMeshes(Tachyon* tachyon, MeshIds& meshes) {
  auto sphere_mesh = Tachyon_CreateSphereMesh(4);
  auto cube_mesh = Tachyon_CreateCubeMesh();

  meshes.sphere = Tachyon_AddMesh(tachyon, sphere_mesh, 40 * 40 * 40);
  meshes.cube = Tachyon_AddMesh(tachyon, cube_mesh, 6);
}

static void SetupFlightSimLevel(Tachyon* tachyon, MeshIds& meshes) {
  create(meshes.planet);
  create(meshes.planet);
  create(meshes.planet);

  for (int32 i = 0; i < 40; i++) {
    for (int32 j = 0; j < 40; j++) {
      for (int32 k = 0; k < 40; k++) {
        auto& sphere = create(meshes.sphere);

        sphere.position = tVec3f(
          (i - 20) * 4000.f,
          (j - 20) * 4000.f,
          (k - 20) * 4000.f
        );

        sphere.scale = 50.f;
        sphere.color = tVec3f(0.2f, 0.5f, 1.f);
        sphere.material = tVec4f(0.8f, 0.f, 1.f, 1.f);

        commit(sphere);
      }
    }
  }

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

static void CreateDebugMeshes(Tachyon* tachyon, MeshIds& meshes) {
  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);

  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);
}

void WorldSetup::LoadMeshes(Tachyon* tachyon, MeshIds& meshes) {
  LoadShipPartMeshes(tachyon, meshes);
  LoadCelestialBodyMeshes(tachyon, meshes);
  LoadDebugMeshes(tachyon, meshes);

  Tachyon_InitializeObjects(tachyon);
}

void WorldSetup::InitializeGameWorld(Tachyon* tachyon, MeshIds& meshes) {
  SetupFlightSimLevel(tachyon, meshes);
  CreateDebugMeshes(tachyon, meshes);
}