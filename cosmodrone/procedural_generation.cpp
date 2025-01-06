#include "cosmodrone/procedural_generation.h"
#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

#define load_mesh(__mesh_entry, __file, __total)\
  __mesh_entry = Tachyon_AddMesh(\
    tachyon,\
    Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" __file ".obj"),\
    __total\
  )

#define apply_scale(__object, __asset)\
  __object.scale = __asset.defaults.scale

#define apply_surface(__object, __asset)\
  __object.color = __asset.defaults.color;\
  __object.material = __asset.defaults.material

#define apply(__object, __asset, __property)\
  __object.__property = __asset.defaults.__property

static void GenerateElevator(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.procedural_track_1);

  // Down
  for (int32 i = 0; i < 150; i++) {
    auto& track = create(meshes.procedural_track_1);

    track.position = tVec3f(0, i * -24000.f, 0);
    track.scale = 12000.f;
    track.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);

    commit(track);
  }

  // Up
  for (int32 i = 1; i < 200; i++) {
    auto& track = create(meshes.procedural_track_1);

    track.position = tVec3f(0, i * 24000.f, 0);
    track.scale = 12000.f;
    track.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);

    commit(track);
  }
}

static void GenerateElevatorToruses(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // station_torus_3
  {
    auto& torus_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3);
    auto& body_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_body);
    auto& frame_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_frame);
    auto& lights_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_lights);

    for (int32 i = 0; i < 10; i++) {
      auto& body = create(meshes.station_torus_3_body);
      auto& frame = create(meshes.station_torus_3_frame);
      auto& lights = create(meshes.station_torus_3_lights);

      body.position =
      frame.position =
      lights.position =
        tVec3f(0, i * -300000.f, 0);

      apply_scale(body, torus_asset);
      apply_scale(frame, torus_asset);
      apply_scale(lights, torus_asset);

      apply_surface(body, body_asset);
      apply_surface(frame, frame_asset);
      apply_surface(lights, lights_asset);

      commit(body);
      commit(frame);
      commit(lights);
    }
  }
}

void ProceduralGeneration::LoadMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  load_mesh(meshes.procedural_track_1, "track_1", 500);
}

void ProceduralGeneration::GenerateWorld(Tachyon* tachyon, State& state) {
  GenerateElevator(tachyon, state);
  GenerateElevatorToruses(tachyon, state);
}