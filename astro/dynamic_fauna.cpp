#include "astro/dynamic_fauna.h"

using namespace astro;

/**
 * Butterflies
 * -----------
 */
static void HandleButterflySpawning(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  if (state.butterflies.size() == 0) {
    Butterfly butterfly;
    butterfly.position = state.player_position + tVec3f(3000.f, 1000.f, 2000.f);
    butterfly.left_wing = create(meshes.butterfly_left_wing).object_id;
    butterfly.right_wing = create(meshes.butterfly_right_wing).object_id;

    state.butterflies.push_back(butterfly);
  }
}

static void HandleButterflies(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  float scene_time = get_scene_time();

  HandleButterflySpawning(tachyon, state);

  for (auto& butterfly : state.butterflies) {
    auto& left_wing = objects(meshes.butterfly_left_wing).getByIdFast(butterfly.left_wing);
    auto& right_wing = objects(meshes.butterfly_right_wing).getByIdFast(butterfly.right_wing);

    left_wing.position = butterfly.position;
    left_wing.color = tVec4f(1.f, 0.7f, 0.7f, 0.4f);
    left_wing.scale = tVec3f(200.f);
    left_wing.material = tVec4f(0.5f, 0, 0, 1.f);

    right_wing.position = butterfly.position;
    right_wing.color = tVec4f(1.f, 0.7f, 0.7f, 0.4f);
    right_wing.scale = tVec3f(200.f);
    right_wing.material = tVec4f(0.5f, 0, 0, 1.f);

    // Wing flapping
    {
      float angle = 0.35f * sinf(25.f * scene_time);

      left_wing.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), angle);
      right_wing.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -angle);

      left_wing.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), scene_time);
      right_wing.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), scene_time);
    }

    commit(left_wing);
    commit(right_wing);
  }
}

void DynamicFauna::HandleBehavior(Tachyon* tachyon, State& state) {
  profile("DynamicFauna::HandleBehavior()");

  HandleButterflies(tachyon, state);
}