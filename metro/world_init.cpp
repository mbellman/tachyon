#include "engine/tachyon.h"

#include "metro/world_init.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)

#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

static void LoadCommonBikeMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.common_frame =    MODEL_MESH("./metro/3d_models/common_bike/frame.obj", 1);
  meshes.common_skeleton = MODEL_MESH("./metro/3d_models/common_bike/skeleton.obj", 1);
  meshes.common_handles =  MODEL_MESH("./metro/3d_models/common_bike/handles.obj", 1);
  meshes.common_seat =     MODEL_MESH("./metro/3d_models/common_bike/seat.obj", 1);
  meshes.common_wheels =   MODEL_MESH("./metro/3d_models/common_bike/wheels.obj", 1);
}

// @todo create a proper bike spawning system
static void SpawnCommonBike(Tachyon* tachyon, State& state, const tVec3f& position) {
  auto& meshes = state.meshes;

  auto& frame = create(meshes.common_frame);
  auto& skeleton = create(meshes.common_skeleton);
  auto& handles = create(meshes.common_handles);
  auto& seat = create(meshes.common_seat);
  auto& wheels = create(meshes.common_wheels);

  frame.position = position;
  skeleton.position = position;
  handles.position = position;
  seat.position = position;
  wheels.position = position;

  frame.scale = tVec3f(2000.f);
  skeleton.scale = tVec3f(2000.f);
  handles.scale = tVec3f(2000.f);
  seat.scale = tVec3f(2000.f);
  wheels.scale = tVec3f(2000.f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);
  commit(wheels);
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
    cube.scale = tVec3f(5000.f);

    commit(cube);
  }

  // @temporary
  SpawnCommonBike(tachyon, state, tVec3f(0, -2000.f, -10000.f));
}

void World::Init(Tachyon* tachyon, State& state) {
  LoadGameMeshes(tachyon, state);
  LoadGameWorld(tachyon, state);
}