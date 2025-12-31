#include "astro/procedural_trees.h"

using namespace astro;

void ProceduralGeneration::GenerateTreeMushrooms(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.tree_mushroom);

  for (uint16 i = 0; i < 200; i++) {
    create(state.meshes.tree_mushroom);
  }
}

void ProceduralGeneration::UpdateTreeMushrooms(Tachyon* tachyon, State& state) {
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

        auto& mushroom = objects(state.meshes.tree_mushroom)[total_mushrooms++];

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

  mesh(state.meshes.tree_mushroom).lod_1.instance_count = total_mushrooms;
}