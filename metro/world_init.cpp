#include "engine/tachyon.h"

#include "metro/world_init.h"
#include "metro/background_bicycles.h"
#include "metro/collision.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

#define METRO_MODEL(path, total) MODEL_MESH("./metro/3d_models/" path, total)

static void LoadDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.debug_sphere = SPHERE_MESH(100, 12);
  meshes.debug_ring   = METRO_MODEL("ring.obj", 10);
  meshes.debug_plane  = PLANE_MESH(100);
  meshes.debug_line   = METRO_MODEL("debug_line.obj", 100);
  meshes.debug_cone   = METRO_MODEL("debug_cone.obj", 100);

  mesh(meshes.debug_sphere).shadow_cascade_ceiling = 0;
  mesh(meshes.debug_ring).shadow_cascade_ceiling = 0;
  mesh(meshes.debug_plane).shadow_cascade_ceiling = 0;
  mesh(meshes.debug_line).shadow_cascade_ceiling = 0;
  mesh(meshes.debug_cone).shadow_cascade_ceiling = 0;
}

static void LoadCommonBikeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.common_frame      = METRO_MODEL("common_bike/frame.obj", 10);
  meshes.common_fork       = METRO_MODEL("common_bike/fork.obj", 10);
  meshes.common_handlebars = METRO_MODEL("common_bike/handlebars.obj", 10);
  meshes.common_grips      = METRO_MODEL("common_bike/grips.obj", 10);
  meshes.common_seatpost   = METRO_MODEL("common_bike/seatpost.obj", 10);
  meshes.common_saddle     = METRO_MODEL("common_bike/saddle.obj", 10);
  meshes.common_crank      = METRO_MODEL("common_bike/crank.obj", 10);
  // @todo pedals
  meshes.common_wheel      = METRO_MODEL("common_bike/wheel.obj", 20);
  meshes.common_spokes     = METRO_MODEL("common_bike/spokes.obj", 20);

  mesh(meshes.common_spokes).shadow_cascade_ceiling = 1;
}

static void LoadStaticEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // Ramps
  {
    meshes.ramp = METRO_MODEL("static_entities/ramp.obj", 100);
  }
}

static void LoadGameMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @temporary
  meshes.dev_cube = CUBE_MESH(10);

  LoadDebugMeshes(tachyon, state);
  LoadCommonBikeMeshes(tachyon, state);
  LoadStaticEntityMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);

  // @todo move to Debug
  {
    for_range(1, 100) {
      create(meshes.debug_sphere);
      create(meshes.debug_plane);
      create(meshes.debug_line);
      create(meshes.debug_cone);
    }

    for_range(1, 10) {
      create(meshes.debug_ring);
    }
  }
}

static void LoadGameWorld(Tachyon* tachyon, State& state) {
  {
    auto& scene = tachyon->scene;

    scene.primary_light_color = tVec3f(1.f, 0.8f, 0.6f);
    scene.primary_light_direction = tVec3f(0.2f, -1.f, 0.7f);

    scene.sky_light_color = tVec3f(0.1f, 0.2f, 0.5f);
    scene.sky_light_direction = tVec3f(0, -1.f, 0);
  }

  // @temporary
  {
    auto& cube = create(state.meshes.dev_cube);

    cube.position = tVec3f(0, -8000.f, -10000.f);
    cube.scale = tVec3f(500000.f, 5000.f, 50000.f);
    cube.color = tVec3f(0.8f);

    commit(cube);

    Collision::AddFloorCollision(state, cube);

    auto& road = create(state.meshes.dev_cube);

    road.position = tVec3f(0, -8000.f, -10000.f);
    road.scale = tVec3f(450000.f, 5010.f, 40000.f);
    road.color = 0x1120;
    road.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(road);

  }

  // @temporary
  {
    auto& cube = create(state.meshes.dev_cube);

    cube.position = tVec3f(0, -250000.f, -200000.f);
    cube.scale = tVec3f(500000.f, 5000.f, 50000.f);
    cube.color = tVec3f(0.8f);

    commit(cube);

    Collision::AddFloorCollision(state, cube);

    auto& road = create(state.meshes.dev_cube);

    road.position = tVec3f(0, -250000.f, -200000.f);
    road.scale = tVec3f(450000.f, 5010.f, 40000.f);
    road.color = 0x1120;
    road.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(road);
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

  // @temporary
  {
    StaticEntity ramp;
    ramp.position = tVec3f(50000.f, -1500.f, -10000.f);
    ramp.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI);
    ramp.scale = tVec3f(3000.f);
    ramp.scale.y = 1500.f;
    ramp.color = tVec3f(0.5f);

    state.ramps.push_back(ramp);
  }
}

void World::Init(Tachyon* tachyon, State& state) {
  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}