#include "cosmodrone/procedural_generation.h"
#include "cosmodrone/mesh_library.h"

constexpr static uint16 TOTAL_TRACK_PIECES = 600;
constexpr static uint16 TOTAL_ELEVATOR_CARS = 16;

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

static AutoPlacedObjectList& GetAutoPlacedObjectList(State& state, const uint16 mesh_index) {
  for (auto& list : state.auto_placed_object_lists) {
    if (list.mesh_index == mesh_index) {
      return list;
    }
  }

  state.auto_placed_object_lists.push_back({
    .mesh_index = mesh_index
  });

  return state.auto_placed_object_lists.back();
}

static void GenerateElevator(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.procedural_track_1);

  // Down
  for (int32 i = 0; i < 300; i++) {
    auto& track = create(meshes.procedural_track_1);

    track.position = tVec3f(0, i * -24000.f, 0);
    track.scale = 12000.f;
    track.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
    track.material = tVec4f(1.f, 0, 0, 0.2f);

    commit(track);
  }

  // Up
  for (int32 i = 1; i < 300; i++) {
    auto& track = create(meshes.procedural_track_1);

    track.position = tVec3f(0, i * 24000.f, 0);
    track.scale = 12000.f;
    track.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
    track.material = tVec4f(1.f, 0, 0, 0.2f);

    commit(track);
  }
}

static void GenerateElevatorCars(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.procedural_elevator_car);
  remove_all(meshes.procedural_elevator_car_light);

  for (int32 i = 0; i < TOTAL_ELEVATOR_CARS; i++) {
    auto& car = create(meshes.procedural_elevator_car);
    auto& lights = create(meshes.procedural_elevator_car_light);
    auto rotation_angle = t_HALF_PI + (i % 4) * t_HALF_PI;

    car.scale = 3000.f;
    car.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);
    car.material = tVec4f(0.2f, 1.f, 0, 0);

    car.position = car.rotation.toMatrix4f().transformVec3f(tVec3f(0, 0, -1.f)) * 4000.f;

    car.position.y =
      -1000000.f +
      500000.f * floorf(i / 4.f) +
      Tachyon_GetRandom(-100000.f, 100000.f);

    lights.scale = car.scale;
    lights.rotation = car.rotation;
    lights.color = tVec4f(1.f, 0.8f, 0.6f, 1.f);
    lights.position = car.position;

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
    auto& list = GetAutoPlacedObjectList(state, meshes.station_torus_2);

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

      list.object_ids.push_back(torus.object_id);
      list.object_ids.push_back(torus2.object_id);
    }
  }

  // station_torus_3
  // @todo use actual station_torus_3 mesh
  {
    auto positions = {
      tVec3f(0, -650000.f, 0),
      tVec3f(0, -1000000.f, 0),

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
    auto& list = GetAutoPlacedObjectList(state, meshes.elevator_torus_1);

    for (int32 i = 1; i < 8; i++) {
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

      list.object_ids.push_back(torus.object_id);
      list.object_ids.push_back(torus2.object_id);
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

      list.object_ids.push_back(torus.object_id);
      list.object_ids.push_back(torus2.object_id);
    }
  }
}

static void GenerateElevatorTrackFrames(Tachyon* tachyon, const State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.procedural_track_supports_1);

  const static float spin_cycle[] = {
    0.f,
    t_TAU * (1.f / 6.f)
  };

  for (int32 i = 0; i < 100; i++) {
    auto& frame = create(meshes.procedural_track_supports_1);
    float spin = spin_cycle[i % 2];

    frame.scale = 12000.f;
    frame.position.y = 3500000.f + i * -70000.f;
    frame.material = tVec4f(0.7f, 1.f, 0, 0);
    frame.rotation =
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), spin) *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);

    commit(frame);
  }
}

void ProceduralGeneration::LoadMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  load_mesh(meshes.procedural_track_1, "/station-parts/track_1.obj", TOTAL_TRACK_PIECES);
  load_mesh(meshes.procedural_elevator_car, "/elevator_car.obj", TOTAL_ELEVATOR_CARS);
  load_mesh(meshes.procedural_elevator_car_light, "/elevator_car_lights.obj", TOTAL_ELEVATOR_CARS);
  load_mesh(meshes.procedural_track_supports_1, "./track_supports_1.obj", 1000);
}

void ProceduralGeneration::RemoveAutoPlacedObjects(Tachyon* tachyon, State& state) {
  for (auto& list : state.auto_placed_object_lists) {
    for (auto& object_id : list.object_ids) {
      remove(list.mesh_index, object_id);
    }
  }

  state.auto_placed_object_lists.clear();
}

void ProceduralGeneration::GenerateWorld(Tachyon* tachyon, State& state) {
  GenerateElevator(tachyon, state);
  GenerateElevatorCars(tachyon, state);
  // GenerateElevatorToruses(tachyon, state);
  // GenerateElevatorTrackFrames(tachyon, state);
}