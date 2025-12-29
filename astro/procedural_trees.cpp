#include "astro/procedural_trees.h"

using namespace astro;

void ProceduralGeneration::GenerateTreeMushrooms(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.tree_mushroom);

  for (uint16 i = 0; i < 100; i++) {
    create(state.meshes.tree_mushroom);
  }
}

void ProceduralGeneration::UpdateTreeMushrooms(Tachyon* tachyon, State& state) {
  profile("UpdateTreeMushrooms()");

  uint16 total_mushrooms = 0;

  static Quaternion rotations[] = {
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 2.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 3.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 4.f)
  };

  static float scales[] = {
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

    if (state.astro_time > entity.astro_start_time + 200.f) {
      for (int i = 0; i < 5; i++) {
        auto& mushroom = objects(state.meshes.tree_mushroom)[total_mushrooms++];
        float scale = scales[((i + entity.id)) % 5];

        mushroom.position = entity.position;
        mushroom.position.z += entity.visible_scale.x * 0.32f;
        mushroom.position.y += 1000.f;
        mushroom.position.y -= 300.f * float(i);
        mushroom.position.x += fmodf(abs(entity.position.x) + 70.f * float(i), 300.f) - 150.f;

        mushroom.rotation = rotations[i];
        mushroom.scale = tVec3f(scale);

        mushroom.color = tVec3f(0.9f, 0.6f, 0.3f);
        mushroom.material = tVec4f(1.f, 0, 0, 0.4f);

        commit(mushroom);
      }
    }
  }

  mesh(state.meshes.tree_mushroom).lod_1.instance_count = total_mushrooms;
}