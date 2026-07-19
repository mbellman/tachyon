#include "astro/visual_effects.h"

using namespace astro;

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

static void UpdateDustClouds(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.dust_cloud);

  for (int i = state.dust_clouds.size() - 1; i >= 0; i--) {
    auto& cloud = state.dust_clouds[i];
    float time_since_spawned = time_since(cloud.spawn_time);

    if (time_since_spawned < 0.f) {
      // If the dust cloud has a delayed spawn time, move on to the next one
      continue;
    }

    float alpha = time_since_spawned / 0.75f;
    clamp_to_0(alpha);
    clamp_to_1(alpha);

    if (alpha == 1.f) {
      // Ignore and remove expired dust clouds
      RemoveFromArray(state.dust_clouds, i);

      continue;
    }

    auto& object = use_instance(meshes.dust_cloud);

    object.position = cloud.spawn_position;
    object.position.y += sqrtf(alpha) * 500.f;

    object.scale = tVec3f(500.f * (1.f - alpha));

    object.color = tVec4f(1.f, 1.f, 1.f, 0.7f);
    object.material = tVec4f(1.f, 0, 0, 1.f);

    commit(object);
  }
}

void VisualEffects::Update(Tachyon* tachyon, State& state) {
  UpdateDustClouds(tachyon, state);
}

void VisualEffects::SpawnDustCloud(Tachyon* tachyon, State& state, const tVec3f& position, const float delay) {
  if (state.dust_clouds.size() == 10) {
    // Disallow more dust clouds than there are allocated objects
    return;
  }

  DustCloud cloud;
  cloud.spawn_position = position;
  cloud.spawn_time = get_scene_time() + delay;

  state.dust_clouds.push_back(cloud);
}

void VisualEffects::SpawnDustCloudsAroundPlayer(Tachyon* tachyon, State& state) {
  for_range(1, 5) {
    float alpha = t_TAU * float(i) / 5.f;

    float x = sinf(alpha);
    float z = cosf(alpha);

    tVec3f spawn_position =
      state.player_position
      - tVec3f(0, 1500.f, 0)
      + tVec3f(1000.f * x, 0, 1000.f * z);

    VisualEffects::SpawnDustCloud(tachyon, state, spawn_position);
  }
}