#include "metro/game.h"
#include "engine/tachyon.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

static void LoadGameMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @temporary
  meshes.cube = CUBE_MESH(10);

  // @temporary
  meshes.bicycle = MODEL_MESH("./metro/3d_models/bicycle.obj", 1);

  Tachyon_InitializeObjects(tachyon);
}

static void LoadGameWorld(Tachyon* tachyon, State& state) {
  // @temporary
  {
    auto& cube = create(state.meshes.cube);

    cube.position = tVec3f(0, -10000.f, -15000.f);
    cube.scale = tVec3f(5000.f);

    commit(cube);
  }

  // @temporary
  {
    auto& bicycle = create(state.meshes.bicycle);

    bicycle.position = tVec3f(0, -4000.f, -15000.f);
    bicycle.scale = tVec3f(2000.f);

    commit(bicycle);
  }
}

void metro::Init(Tachyon* tachyon, State& state) {
  console_log("metro::Init()");

  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  // @todo HandleFrameStart()
  {
    state.dt = dt;

    tachyon->scene.scene_time += state.dt;
  }

  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  // @temporary
  {
    auto& bicycle = objects(state.meshes.bicycle)[0];

    bicycle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), get_scene_time());

    commit(bicycle);
  }
}