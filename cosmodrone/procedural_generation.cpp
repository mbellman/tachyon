#include "cosmodrone/procedural_generation.h"
#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

#define load_mesh(__mesh_entry, __file, __total)\
  __mesh_entry = Tachyon_AddMesh(\
    tachyon,\
    Tachyon_LoadMesh("./cosmodrone/assets" __file),\
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
  for (int32 i = 0; i < 250; i++) {
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

static void GenerateElevatorCars(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.elevator_car);

  for (int32 i = 0; i < 4; i++) {
    auto& car = create(meshes.elevator_car);

    car.scale = 3000.f;
    car.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI + i * t_HALF_PI);
    car.material = tVec4f(0.2f, 1.f, 0, 0);

    car.position = car.rotation.toMatrix4f().transformVec3f(tVec3f(0, 0, -1.f)) * 4000.f;
    car.position.y = Tachyon_GetRandom(-50000.f, 50000.f);

    commit(car);
  }
}

static void GenerateStationTorus3(Tachyon* tachyon, const State& state, const tVec3f& position) {
  auto& meshes = state.meshes;

  // @todo optimize
  auto& torus_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3);
  auto& body_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_body);
  auto& frame_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_frame);
  auto& lights_asset = MeshLibrary::FindMeshAsset(meshes.station_torus_3_lights);

  auto& body = create(meshes.station_torus_3_body);
  auto& frame = create(meshes.station_torus_3_frame);
  auto& lights = create(meshes.station_torus_3_lights);

  body.position =
  frame.position =
  lights.position =
    position;

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

static void GenerateElevatorToruses(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // station_torus_2
  {
    auto& asset = MeshLibrary::FindMeshAsset(meshes.station_torus_2);

    remove_all(meshes.station_torus_2);

    for (int32 i = 1; i <= 2; i++) {
      auto& torus = create(meshes.station_torus_2);
      auto& torus2 = create(meshes.station_torus_2);

      torus.position = tVec3f(0, 50000.f + i * -2000000.f, 0);
      torus2.position = torus.position * -1.f;

      apply_scale(torus, asset);
      apply_scale(torus2, asset);

      apply_surface(torus, asset);
      apply_surface(torus2, asset);

      commit(torus);
      commit(torus2);
    }
  }

  // station_torus_3
  {
    auto positions = {
      tVec3f(0, -300000.f, 0),
      tVec3f(0, -500000.f, 0),

      tVec3f(0, -1700000.f, 0),
      tVec3f(0, -1900000.f, 0),

      tVec3f(0, -3700000.f, 0),
      tVec3f(0, -4100000.f, 0),

      tVec3f(0, -4700000.f, 0),
      tVec3f(0, -4900000.f, 0)
    };

    for (int32 i = 0; i < 8; i++) {
      auto& position = *(positions.begin() + i);

      GenerateStationTorus3(tachyon, state, position);
      GenerateStationTorus3(tachyon, state, tVec3f(0, 250000.f, 0) + position * -1.f);
    }
  }

  // elevator_torus_1
  {
    auto& asset = MeshLibrary::FindMeshAsset(meshes.elevator_torus_1);

    remove_all(meshes.elevator_torus_1);

    for (int32 i = 0; i < 8; i++) {
      auto& torus = create(meshes.elevator_torus_1);
      auto& torus2 = create(meshes.elevator_torus_1);
      float interval = i % 2 == 0 ? -300000.f : -400000.f;

      torus.position = tVec3f(0, i * interval - 150000.f, 0);
      torus2.position = torus.position * -1.f + tVec3f(0, 300000.f, 0);

      apply_scale(torus, asset);
      apply_scale(torus2, asset);

      apply_surface(torus, asset);
      apply_surface(torus, asset);

      commit(torus);
      commit(torus2);
    }

    for (int32 i = 14; i < 20; i++) {
      auto& torus = create(meshes.elevator_torus_1);
      auto& torus2 = create(meshes.elevator_torus_1);
      float interval = i % 2 == 0 ? -300000.f : -400000.f;

      torus.position = tVec3f(0, i * interval, 0);
      torus2.position = torus.position * -1.f;

      apply_scale(torus, asset);
      apply_scale(torus2, asset);

      apply_surface(torus, asset);
      apply_surface(torus2, asset);

      commit(torus);
      commit(torus2);
    }
  }
}

void ProceduralGeneration::LoadMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  load_mesh(meshes.procedural_track_1, "/station-parts/track_1.obj", 500);
  load_mesh(meshes.elevator_car, "/elevator_car.obj", 20);
}

void ProceduralGeneration::GenerateWorld(Tachyon* tachyon, State& state) {
  GenerateElevator(tachyon, state);
  GenerateElevatorCars(tachyon, state);
  GenerateElevatorToruses(tachyon, state);
}