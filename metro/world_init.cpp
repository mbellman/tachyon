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

  meshes.common_frame      = MODEL_MESH("./metro/3d_models/common_bike/frame.obj", 10);
  meshes.common_fork       = MODEL_MESH("./metro/3d_models/common_bike/fork.obj", 10);
  meshes.common_handlebars = MODEL_MESH("./metro/3d_models/common_bike/handlebars.obj", 10);
  meshes.common_grips      = MODEL_MESH("./metro/3d_models/common_bike/grips.obj", 10);
  meshes.common_seatpost   = MODEL_MESH("./metro/3d_models/common_bike/seatpost.obj", 10);
  meshes.common_saddle     = MODEL_MESH("./metro/3d_models/common_bike/saddle.obj", 10);
  meshes.common_wheel      = MODEL_MESH("./metro/3d_models/common_bike/wheel.obj", 20);
  meshes.common_spokes     = MODEL_MESH("./metro/3d_models/common_bike/spokes.obj", 20);

  mesh(meshes.common_spokes).shadow_cascade_ceiling = 1;
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
    cube.scale = tVec3f(50000.f, 5000.f, 50000.f);
    cube.color = tVec3f(0.8f);

    commit(cube);
  }

  // @temporary
  state.player_bike_id = 2;

  // @temporary
  {
    Bicycle bike;
    bike.id            = 1;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(-5000.f, -2220.f, -10000.f);
    bike.frame_color   = 0xFFF8;
    bike.grips_color   = tVec3f(0.1f);
    bike.saddle_color  = tVec3f(0.1f, 0, 0);
    bike.wheel_color   = tVec3f(0.2f);

    bike.facing_direction = tVec3f(0, 0, -1.f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }

  // @temporary
  {
    Bicycle bike;
    bike.id            = 2;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(0, -2220.f, -10000.f);
    bike.frame_color   = tVec3f(0.5f, 1.f, 0.4f);
    bike.grips_color   = tVec3f(0.1f);
    bike.saddle_color  = tVec3f(0.2f);
    bike.wheel_color   = tVec3f(1.f, 0.9f, 0.7f);

    bike.facing_direction = tVec3f(0, 0, -1.f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }

  // @temporary
  {
    Bicycle bike;
    bike.id            = 3;
    bike.type          = BicycleType::COMMON_BIKE;
    bike.position      = tVec3f(5000.f, -2220.f, -10000.f);
    bike.frame_color   = tVec3f(1.f, 0.2f, 0.4f);
    bike.grips_color   = tVec3f(0.1f);
    bike.saddle_color  = tVec3f(0.1f, 0, 0);
    bike.wheel_color   = tVec3f(1.f, 0.9f, 0.7f);

    bike.facing_direction = tVec3f(0, 0, -1.f);

    BackgroundBicycles::SpawnBicycle(tachyon, state, bike);
  }
}

void World::Init(Tachyon* tachyon, State& state) {
  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}