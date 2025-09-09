#include "astro/game.h"
#include "astro/mesh_library.h"
#include "astro/object_manager.h"

using namespace astro;

static void UpdatePlayer(Tachyon* tachyon, State& state) {
  auto& player = objects(state.meshes.player)[0];
  
  player.position = state.player_position;
  // @temporary
  player.scale = tVec3f(600.f);
  player.scale.y = 1200.f;
  player.color = tVec3f(0, 0.2f, 1.f);

  commit(player);
}

static void UpdateWaterPlane(Tachyon* tachyon, State& state) {
  auto& water_plane = objects(state.meshes.water_plane)[0];

  // @temporary
  water_plane.position = tVec3f(0, -2500.f, 0);
  water_plane.scale = tVec3f(40000.f, 1.f, 40000.f);
  water_plane.color = tVec3f(0, 0.2f, 0.5f);

  commit(water_plane);
}

static void UpdateGroundPlane(Tachyon* tachyon, State& state) {
  auto& ground_plane = objects(state.meshes.ground_plane)[0];

  // @temporary
  ground_plane.position = tVec3f(0, -1500.f, 2500.f);
  ground_plane.scale = tVec3f(20000.f, 1.f, 5000.f);
  ground_plane.color = tVec3f(0.4f, 0.5f, 0.1f);

  commit(ground_plane);
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  astro::AddMeshes(tachyon, state);

  Tachyon_InitializeObjects(tachyon);

  astro::CreateObjects(tachyon, state);
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& scene = tachyon->scene;

  UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateGroundPlane(tachyon, state);

  // @temporary
  // @todo unit() this in the renderer
  scene.primary_light_direction = tVec3f(1.f, -1.f, -0.2f).unit();
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