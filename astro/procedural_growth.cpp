#include "astro/procedural_growth.h"
#include "astro/astrolabe.h"

using namespace astro;

static void UpdateTreeMushrooms(Tachyon* tachyon, State& state) {
  profile("UpdateTreeMushrooms()");

  uint16 total_mushrooms = 0;

  const static tVec3f xz_positions[] = {
    tVec3f(0, 0, 1.f),
    tVec3f(0, 0, 1.f),
    tVec3f(0, 0, 1.f),
    tVec3f(1.f, 0, 1.f).unit(),
    tVec3f(1.f, 0, 1.f).unit(),
    tVec3f(-1.f, 0, 1.f).unit(),
    tVec3f(-1.f, 0, 1.f).unit(),
    tVec3f(-1.f, 0, 1.f).unit()
  };

  const static float y_positions[] = {
    1500.f,
    300.f,
    700.f,
    1100.f,
    -100.f
  };

  const static Quaternion rotations[] = {
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 2.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 3.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 4.f)
  };

  const static float scales[] = {
    400.f,
    300.f,
    500.f,
    320.f,
    350.f
  };

  reset_instances(state.meshes.tree_mushroom);

  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];

    // @todo factor
    if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
    if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

    float group_growth_duration = state.astro_time - (entity.astro_start_time + 200.f);

    if (group_growth_duration > 0.f) {
      for (int i = 0; i < 5; i++) {
        float mushroom_growth_duration = group_growth_duration - (float(i) * 5.f);

        // Don't spawn mushrooms before they've started growing
        if (mushroom_growth_duration < 0.f) continue;

        auto& mushroom = use_instance(state.meshes.tree_mushroom);

        float scale_alpha = mushroom_growth_duration / 10.f;
        if (scale_alpha > 1.f) scale_alpha = 1.f;

        float scale = scales[((i + entity.id)) % 5] * scale_alpha;

        mushroom.position = entity.position;
        mushroom.position += xz_positions[(i + entity.id) % 8] * entity.visible_scale.x * 0.32f;
        mushroom.position.y += y_positions[((i + entity.id)) % 5];
        mushroom.position.x += fmodf(abs(entity.position.x) + 70.f * float(i), 300.f) - 150.f;

        mushroom.rotation = rotations[i];
        mushroom.scale = tVec3f(scale);

        float emissivity = state.is_nighttime ? 0.3f : 0.f;

        mushroom.color = tVec4f(0.9f, 0.6f, 0.3f, emissivity);
        mushroom.material = tVec4f(1.f, 0, 0, 0.4f);

        commit(mushroom);
      }
    }
  }
}

// @todo @optimize precompute leaf + flower positions
// and use one of several preset vine patterns
static void UpdateWhiteVines(Tachyon* tachyon, State& state) {
  profile("UpdateWhiteVines()");

  auto& meshes = state.meshes;

  reset_instances(meshes.vine_leaf);
  reset_instances(meshes.vine_flower);

  uint16 total_visible_oak_trunks = mesh(meshes.oak_tree_trunk).lod_1.instance_count;

  for (uint16 i = 0; i < total_visible_oak_trunks; i++) {
    auto& trunk = objects(meshes.oak_tree_trunk)[i];

    // Skip generating vines on trunks out of screen range.
    // We don't do separate culling for the shadow pass, so some
    // trunks may be "visible" offscreen but not close enough
    // to the player for extra visual effects to matter.
    if (abs(state.player_position.x - trunk.position.x) > 20000.f) continue;
    if (abs(state.player_position.z - trunk.position.z) > 15000.f) continue;

    int total_leaves = int((state.astro_time - astro_time_periods.past) / 2.5f);

    for (int j = 0; j < total_leaves; j++) {
      auto& leaf = use_instance(meshes.vine_leaf);

      float leaf_age = (total_leaves - j) * 2.5f;
      float growth_factor = leaf_age / 10.f;
      if (growth_factor > 1.f) growth_factor = 1.f;

      float angle = sinf(float(j) * 0.5f);
      float tilt = 0.5f * cosf(float(j));

      Quaternion rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI + angle) *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), tilt)
      );

      tVec3f offset;
      offset.x = trunk.scale.x * 0.38f * sinf(angle);
      offset.y = -trunk.scale.y * 0.5f;
      offset.z = trunk.scale.z * 0.38f * cosf(angle);

      leaf.position = trunk.position + offset;
      leaf.position.y += float(j) * 250.f;
      leaf.scale = tVec3f(250.f * growth_factor);
      leaf.rotation = rotation;
      leaf.color = tVec3f(0.1f, 0.3f, 0.2f);
      leaf.material = tVec4f(0.4f, 0, 0, 1.f);

      commit(leaf);

      // Add flowers on some leaves
      if (j % 3 == 0) {
        auto& flower = use_instance(meshes.vine_flower);

        flower.position = leaf.position;
        flower.scale = tVec3f(250.f * growth_factor);
        flower.rotation = rotation;
        flower.color = tVec4f(1.f, 1.f, 1.f, 0.4f);
        flower.material = tVec4f(0.5f, 0, 0, 1.f);

        commit(flower);
      }
    }
  }
}

// @todo @optimize precompute flower positions/rotations/etc. and use different presets
static void UpdateTreeFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateTreeFlowers()");

  auto& meshes = state.meshes;

  // Parameters for each of the four flower groups
  const static int flower_counts[] = {
    30,
    25,
    15,
    8
  };

  const static float angle_offsets[] = {
    0.f,
    t_HALF_PI,
    t_PI,
    t_PI + t_HALF_PI
  };

  const static float radius_factors[] = {
    1.4f,
    1.2f,
    1.f,
    0.5f
  };

  const static float y_offsets[] = {
    -250.f,
    250.f,
    750.f,
    1250.f
  };

  // Varying offset radii
  const static float offset_radii[] = {
    1.f,
    0.7f,
    0.5f,
    0.8f,
    0.7f,
    1.f,
    0.8f,
    0.6f,
    0.5f,
    0.9f
  };

  // Varying scales
  const static float scales[] = {
    400.f,
    200.f,
    275.f
  };

  // Varying growth start times
  const static float growth_start_times[] = {
    -36.f,
    -32.f,
    -25.f,
    -18.f
  };

  uint16 max_flower_count = objects(meshes.tree_flower).total;
  uint16 total_visible_oak_leaves = mesh(meshes.oak_tree_leaves).lod_1.instance_count;

  reset_instances(meshes.tree_flower);

  for (uint16 i = 0; i < total_visible_oak_leaves; i++) {
    auto& leaves = objects(meshes.oak_tree_leaves)[i];

    if (abs(state.player_position.x - leaves.position.x) > 15000.f) continue;
    if (abs(state.player_position.z - leaves.position.z) > 12000.f) continue;

    tVec3f base_position = leaves.position + tVec3f(0, leaves.scale.y * 2.f, 0);

    for_range(0, 3) {
      int flower_count = flower_counts[i];
      float angle_offset = angle_offsets[i];
      float y_offset = y_offsets[i];
      float radius_factor = radius_factors[i];

      for (uint16 j = 0; j < flower_count; j++) {
        if (count_used_instances(meshes.tree_flower) >= max_flower_count) {
          break;
        }

        auto& flower = use_instance(meshes.tree_flower);

        float angle = t_TAU * (float(j) / float(flower_count)) + angle_offset;
        float radius = leaves.scale.x * offset_radii[j % 10] * radius_factor;
        float tilt = 0.8f * (radius / leaves.scale.x);

        tVec3f offset;
        offset.x = radius * sinf(angle);
        offset.y = y_offset;
        offset.z = radius * cosf(angle);

        Quaternion rotation = (
          Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle) *
          Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), tilt)
        );

        float growth_time = growth_start_times[j % 4];
        float growth_factor = (state.astro_time - growth_time) / 20.f;
        if (growth_factor < 0.f) growth_factor = 0.f;

        float max_scale = scales[j % 3];
        float scale = max_scale * (1.f - 1.f / expf(3.5f * growth_factor));

        flower.scale = tVec3f(
          scale,
          3.f * scale,
          scale
        );

        flower.position = base_position + offset;
        flower.rotation = rotation;
        flower.color = tVec4f(1.f, 1.f, 1.f, 0.4f);
        flower.material = tVec4f(0.5f, 0, 0, 1.f);

        commit(flower);
      }
    }
  }
}

void ProceduralBehavior::Growth::Update(Tachyon* tachyon, State& state) {
  UpdateTreeMushrooms(tachyon, state);
  UpdateWhiteVines(tachyon, state);
  UpdateTreeFlowers(tachyon, state);
}