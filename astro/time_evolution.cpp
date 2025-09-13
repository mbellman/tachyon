#include "astro/time_evolution.h"

using namespace astro;

static void TimeEvolveOakTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  const float lifetime = 200.f;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for (uint16 i = 0; i < (uint16)state.oak_trees.size(); i++) {
    const auto& tree = state.oak_trees[i];

    // @todo cleanup
    float entity_age = state.astro_time - tree.astro_time_when_born;
    if (entity_age < 0.f) entity_age = 0.f;
    if (entity_age > lifetime) entity_age = lifetime;

    float life_progression = entity_age / lifetime;

    // @todo factor
    auto& trunk = objects(meshes.oak_tree_trunk)[i];
    float trunk_height = 1.f - powf(1.f - life_progression, 4.f);
    float trunk_thickness = -(cosf(t_PI * life_progression) - 1.f) / 2.f;

    trunk.scale = tree.scale * tVec3f(
      0.02f + 0.18f * trunk_thickness,
      trunk_height,
      0.02f + 0.18f * trunk_thickness
    );

    trunk.position.y = tree.position.y - tree.scale.y * (1.f - trunk_height);

    commit(trunk);
  }
}

static void TimeEvolveWillowTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for (uint16 i = 0; i < (uint16)state.willow_trees.size(); i++) {
    const auto& tree = state.oak_trees[i];

    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.scale = tree.scale;

    commit(trunk);
  }
}

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  state.astro_time += 0.01f * dt;

  TimeEvolveOakTrees(tachyon, state);
  TimeEvolveWillowTrees(tachyon, state);

  // @temporary
  // @todo unit() this in the renderer
  tachyon->scene.primary_light_direction = tVec3f(1.f - state.astro_time / 200.f, -1.f, -0.2f).unit();
}