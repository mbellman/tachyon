#include "astro/game.h"
#include "astro/mesh_library.h"
#include "astro/object_manager.h"

using namespace astro;

void astro::InitGame(Tachyon* tachyon, State& state) {
  astro::AddMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);

  astro::CreateObjects(tachyon, state);
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& scene = tachyon->scene;

  // @temporary
  auto& cube = objects(state.meshes.cube)[0];

  cube.position = state.player_position;
  cube.scale = tVec3f(600.f);
  cube.scale.y = 1200.f;
  cube.color = tVec3f(0, 0.2f, 0.8f);

  commit(cube);

  // @temporary
  auto& plane = objects(state.meshes.plane)[0];

  plane.position = tVec3f(0, -1500.f, 0);
  plane.scale = tVec3f(20000.f, 1.f, 20000.f);

  commit(plane);

  // @temporary
  // @todo unit() this in the renderer
  scene.directional_light_direction = tVec3f(1.f, -1.f, -0.2f).unit();
  scene.camera.position = tVec3f(0, 10000.f, 10000.f);
  scene.camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);

  // @temporary
  if (is_key_held(tKey::ARROW_UP) || is_key_held(tKey::W)) {
    state.player_position += tVec3f(0, 0, -1.f) * 4000.f * dt;
  }

  if (is_key_held(tKey::ARROW_LEFT) || is_key_held(tKey::A)) {
    state.player_position += tVec3f(-1.f, 0, 0) * 4000.f * dt;
  }

  if (is_key_held(tKey::ARROW_RIGHT) || is_key_held(tKey::D)) {
    state.player_position += tVec3f(1.f, 0, 0) * 4000.f * dt;
  }

  if (is_key_held(tKey::ARROW_DOWN) || is_key_held(tKey::S)) {
    state.player_position += tVec3f(0, 0, 1.f) * 4000.f * dt;
  }

  state.player_position.x += tachyon->left_stick.x * 6000.f * dt;
  state.player_position.z += tachyon->left_stick.y * 6000.f * dt;
}