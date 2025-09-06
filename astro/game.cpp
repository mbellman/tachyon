#include "engine/tachyon.h"
#include "astro/game.h"

using namespace astro;

void astro::InitGame(Tachyon* tachyon, State& state) {
  state.meshes.cube = Tachyon_AddMesh(
    tachyon,
    Tachyon_CreateCubeMesh(),
    1
  );

  Tachyon_InitializeObjects(tachyon);

  create(state.meshes.cube);
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& cube = objects(state.meshes.cube)[0];

  cube.position = tVec3f(0, -2000.f, -5000.f);
  cube.scale = tVec3f(1000.f);

  commit(cube);

  tachyon->scene.directional_light_direction = tVec3f(1.f, -1.f, -0.2f);
}