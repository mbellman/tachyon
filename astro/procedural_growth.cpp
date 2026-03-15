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

    for (int i = 0; i < total_leaves; i++) {
      auto& leaf = use_instance(meshes.vine_leaf);

      float leaf_age = (total_leaves - i) * 2.5f;
      float growth_factor = leaf_age / 10.f;
      if (growth_factor > 1.f) growth_factor = 1.f;

      float angle = sinf(float(i) * 0.5f);
      float tilt = 0.5f * cosf(float(i));

      Quaternion rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI + angle) *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), tilt)
      );

      tVec3f offset;
      offset.x = trunk.scale.x * 0.38f * sinf(angle);
      offset.y = -trunk.scale.y * 0.5f;
      offset.z = trunk.scale.z * 0.38f * cosf(angle);

      leaf.position = trunk.position + offset;
      leaf.position.y += float(i) * 250.f;
      leaf.scale = tVec3f(250.f * growth_factor);
      leaf.rotation = rotation;
      leaf.color = tVec3f(0.1f, 0.3f, 0.2f);
      leaf.material = tVec4f(0.4f, 0, 0, 1.f);

      commit(leaf);

      // Add flowers on some leaves
      if (i % 3 == 0) {
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

static void UpdateTreeFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateTreeFlowers()");

  auto& meshes = state.meshes;

  reset_instances(meshes.oak_flowers);

  uint16 total_visible_oak_leaves = mesh(meshes.oak_tree_leaves).lod_1.instance_count;

  for (uint16 i = 0; i < total_visible_oak_leaves; i++) {
    auto& leaves = objects(meshes.oak_tree_leaves)[i];

    if (abs(state.player_position.x - leaves.position.x) > 20000.f) continue;
    if (abs(state.player_position.z - leaves.position.z) > 15000.f) continue;

    // @temporary
    auto& flowers = use_instance(meshes.oak_flowers);

    flowers.position = leaves.position;
    flowers.scale = leaves.scale;
    flowers.rotation = leaves.rotation;
    flowers.color = tVec4f(1.f, 1.f, 1.f, 0.4f);
    flowers.material = tVec4f(0.5f, 0, 0, 1.f);

    commit(flowers);
  }
}

void ProceduralBehavior::Growth::Update(Tachyon* tachyon, State& state) {
  UpdateTreeMushrooms(tachyon, state);
  UpdateWhiteVines(tachyon, state);
  UpdateTreeFlowers(tachyon, state);
}