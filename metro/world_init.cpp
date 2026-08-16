#include "engine/tachyon.h"

#include "metro/world_init.h"
#include "metro/background_bicycles.h"
#include "metro/serialization.h"
#include "metro/utilities.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

#define METRO_MODEL(path, total) MODEL_MESH("./metro/3d_models/" path, total)

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

// @todo move to static_entities.cpp
static void LoadStaticEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // Platforms
  {
    meshes.platform = CUBE_MESH(500);
  }

  // Ramps
  {
    meshes.ramp = METRO_MODEL("static_entities/ramp.obj", 500);
  }

  // Walkway segments
  {
    meshes.walkway_segment = CUBE_MESH(500);
    meshes.walkway_plane   = PLANE_MESH(500);
  }
}

static void CreateVertexStreams(Tachyon* tachyon, State& state) {
  state.meshes.walkway_stream = Tachyon_AddVertexStream(tachyon);
}

static void LoadGameMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @temporary
  meshes.dev_cube = CUBE_MESH(10);
  meshes.dev_sphere = SPHERE_MESH(1000, 12);

  mesh(meshes.dev_sphere).shadow_cascade_ceiling = 0;

  // @temporary
  // @todo use a skinned mesh
  meshes.dev_mannequin = METRO_MODEL("dev_mannequin.obj", 1);

  Debug::Init(tachyon);
  LoadCommonBikeMeshes(tachyon, state);
  LoadStaticEntityMeshes(tachyon, state);
  CreateVertexStreams(tachyon, state);

  Tachyon_InitializeObjects(tachyon);

  Debug::CreateObjects(tachyon);

  // Provision planes for dynamic walkways
  // @temporary
  // @todo use dynamic geometry
  {
    for_range(1, 500) {
      create(state.meshes.walkway_plane);
    }

    reset_instances(state.meshes.walkway_plane);
  }

  // @temporary
  create(meshes.dev_mannequin);
}

static void LoadGameWorld(Tachyon* tachyon, State& state) {
  Serialization::LoadWorldData(tachyon, state, "test_world.lvl");

  // @temporary
  {
    auto& scene = tachyon->scene;

    scene.primary_light_color = tVec3f(1.f, 0.8f, 0.6f);
    scene.primary_light_direction = tVec3f(0.2f, -1.f, 0.7f);

    scene.sky_light_color = tVec3f(0.1f, 0.2f, 0.5f);
    scene.sky_light_direction = tVec3f(0, -1.f, 0);
  }

  // @temporary
  {
    auto& road = create(state.meshes.dev_cube);

    road.position = tVec3f(0, -8000.f, -10000.f);
    road.scale = tVec3f(450000.f, 5010.f, 40000.f);
    road.color = 0x1120;
    road.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(road);

  }

  // @temporary
  {
    auto& road = create(state.meshes.dev_cube);

    road.position = tVec3f(0, -250000.f, -200000.f);
    road.scale = tVec3f(450000.f, 5010.f, 40000.f);
    road.color = 0x1120;
    road.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(road);
  }

  // @temporary
  state.player_position = tVec3f(0, -1100.f, 0.f);
  state.previous_player_position = state.player_position;

  // @temporary
  {
    auto& player = objects(state.meshes.dev_mannequin)[0];

    player.position = state.player_position;
    player.scale = tVec3f(2000.f);
    player.rotation = Quaternion::FromDirection(Z_BACKWARD, Y_UP);

    commit(player);
  }
}

void World::Init(Tachyon* tachyon, State& state) {
  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}