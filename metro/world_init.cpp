#include "engine/tachyon.h"

#include "metro/world_init.h"
#include "metro/background_bicycles.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

static void LoadCommonBikeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.common_frame    = MODEL_MESH("./metro/3d_models/common_bike/frame.obj", 10);
  meshes.common_skeleton = MODEL_MESH("./metro/3d_models/common_bike/skeleton.obj", 10);
  meshes.common_handles  = MODEL_MESH("./metro/3d_models/common_bike/handles.obj", 10);
  meshes.common_seat     = MODEL_MESH("./metro/3d_models/common_bike/seat.obj", 10);
  meshes.common_wheels   = MODEL_MESH("./metro/3d_models/common_bike/wheels.obj", 10);
}

static void LoadGameMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @temporary
  meshes.cube = CUBE_MESH(10);

  // @temporary
  meshes.bicycle = MODEL_MESH("./metro/3d_models/bicycle.obj", 1);

  LoadCommonBikeMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);
}

static void LoadGameWorld(Tachyon* tachyon, State& state) {
  // @temporary
  {
    auto& cube = create(state.meshes.cube);

    cube.position = tVec3f(0, -8000.f, -10000.f);
    cube.scale = tVec3f(8000.f, 5000.f, 5000.f);

    commit(cube);
  }

  // @temporary
  {
    Bicycle bike;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(-5000.f, -2000.f, -10000.f);
    bike.frame_color   = 0xFFF8;
    bike.handles_color = tVec3f(0.1f);
    bike.seat_color    = tVec3f(0.1f, 0, 0);
    bike.wheel_color   = tVec3f(0.2f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }

  // @temporary
  {
    Bicycle bike;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(0, -2000.f, -10000.f);
    bike.frame_color   = tVec3f(0.5f, 1.f, 0.4f);
    bike.handles_color = tVec3f(0.1f);
    bike.seat_color    = tVec3f(0.2f);
    bike.wheel_color   = tVec3f(1.f, 0.9f, 0.7f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }

  // @temporary
  {
    Bicycle bike;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(5000.f, -2000.f, -10000.f);
    bike.frame_color   = tVec3f(1.f, 0.2f, 0.4f);
    bike.handles_color = tVec3f(0.1f);
    bike.seat_color    = tVec3f(0.1f, 0, 0);
    bike.wheel_color   = tVec3f(1.f, 0.9f, 0.7f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }
}

void World::Init(Tachyon* tachyon, State& state) {
  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}