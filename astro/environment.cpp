#include "astro/environment.h"

using namespace astro;

static void InitStrayLeaves(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  float player_x = state.player_position.x;
  float player_z = state.player_position.z;

  for (auto& leaf : objects(meshes.stray_leaf)) {
    leaf.position.x = Tachyon_GetRandom(player_x - 15000.f, player_x + 15000.f);
    leaf.position.y = state.player_position.y + Tachyon_GetRandom(3000.f, 8000.f);
    leaf.position.z = Tachyon_GetRandom(player_z - 12000.f, player_z + 12000.f);
  }
}

static void HandleStrayLeaves(Tachyon* tachyon, State& state) {
  profile("HandleStrayLeaves()");

  const static float speeds[] = {
    2800.f,
    2600.f,
    3000.f,
    2300.f
  };

  const static float rotation_speeds[] = {
    4.f,
    2.f,
    3.f,
    2.5f,
    4.5f,
    5.5f
  };

  auto& meshes = state.meshes;
  float scene_time = get_scene_time();
  // Scale down the leaves during astro travel
  float scale_factor = 1.f - 4.f * abs(state.astro_turn_speed);

  for (auto& leaf : objects(meshes.stray_leaf)) {
    float movement_speed = speeds[leaf.object_id % 4];
    float rotation_speed = rotation_speeds[leaf.object_id % 6];
    float t = scene_time + float(leaf.object_id);

    leaf.position.x += movement_speed * state.dt;
    leaf.position.y += 1000.f * sinf(t) * state.dt;
    leaf.position.z += 500.f * cosf(t) * state.dt;
    leaf.scale = tVec3f(150.f * scale_factor);

    leaf.rotation *= (
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_speed * state.dt) *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), rotation_speed * 0.7f * state.dt)
    );

    leaf.color = tVec3f(0.15f, 0.3f, 0.1f);
    leaf.material = tVec4f(0.9f, 0, 0, 1.f);

    if (leaf.position.z - state.player_position.z > 15000.f) {
      // Move leaves into view as we move north
      leaf.position.z = state.player_position.z - 15000.f;
    }

    if (state.player_position.z - leaf.position.z > 15000.f) {
      // Move leaves into view as we move south
      leaf.position.z = state.player_position.z + 15000.f;
    }

    if (state.player_position.x - leaf.position.x > 16000.f) {
      // Move leaves into view as we move east
      leaf.position.x = state.player_position.x + 14000.f;
    }

    if (leaf.position.x - state.player_position.x > 15000.f) {
      // Respawn to the left as leaves fly off the right side
      float player_z = state.player_position.z;

      leaf.position.x = state.player_position.x - 15000.f;
      leaf.position.y = state.player_position.y + Tachyon_GetRandom(3000.f, 8000.f);
      leaf.position.z = Tachyon_GetRandom(player_z - 12000.f, player_z + 12000.f);
    }

    commit(leaf);
  }
}

void HandleDustMotes(Tachyon* tachyon, State& state) {
  // @todo
}

void Environment::Init(Tachyon* tachyon, State& state) {
  InitStrayLeaves(tachyon, state);
}

void Environment::HandleEnvironment(Tachyon* tachyon, State& state) {
  HandleStrayLeaves(tachyon, state);
  HandleDustMotes(tachyon, state);
}