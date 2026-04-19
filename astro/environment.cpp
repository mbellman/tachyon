#include "astro/environment.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

static uint16 Hash(uint16 x) {
  x ^= x >> 8;
  x *= 0x352d;
  x ^= x >> 7;
  x *= 0xa68b;
  x ^= x >> 8;

  return x;
}

static float HashToFloat(uint16 h) {
  return h / float(0xFFFF);
}

static float Wrap(float value, float min, float max, float range) {
  if (value < 0.f) {
    return max + fmodf(value, range);
  } else {
    return min + fmodf(value, range);
  }
}

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
  profile("HandleDustMotes()");

  auto& meshes = state.meshes;
  float scene_time = get_scene_time();

  const float max_particles = objects(meshes.dust_mote).total;

  const float scales[] = {
    20.f,
    25.f,
    35.f
  };

  reset_instances(meshes.dust_mote);

  for_entities(state.fog_spawns) {
    auto& entity = state.fog_spawns[i];

    // Ignore inactive/out-of-range fog spawns
    if (!IsDuringActiveTime(entity, state)) continue;
    if (!IsInRangeX(entity, state, 40000.f)) continue;
    if (!IsInRangeZ(entity, state, 40000.f)) continue;

    // Don't generate any particles past the limit
    if (count_used_instances(meshes.dust_mote) == max_particles) return;

    for_range(1, 100) {
      auto& dust = use_instance(meshes.dust_mote);

      uint16 seed = (uint16) i;
      auto hash_x = Hash(seed);
      auto hash_z = Hash(seed + 100);
      float x = HashToFloat(hash_x) * 36000.f;
      float z = HashToFloat(hash_z) * 36000.f;

      // Base position
      dust.position.x = entity.position.x - 18000.f + x;
      dust.position.y = entity.position.y;
      dust.position.z = entity.position.z - 18000.f + z;

      // Oscillation
      dust.position.x += 500.f * sinf(0.5f * scene_time + x);
      dust.position.y += 300.f * sinf(scene_time + x);

      dust.scale = tVec3f(scales[i % 3]);
      dust.color = tVec4f(1.f);
      dust.material = tVec4f(1.f, 0, 0, 1.f);

      commit(dust);
    }
  }
}

void Environment::Init(Tachyon* tachyon, State& state) {
  InitStrayLeaves(tachyon, state);
}

void Environment::HandleEnvironment(Tachyon* tachyon, State& state) {
  HandleStrayLeaves(tachyon, state);
  HandleDustMotes(tachyon, state);
}