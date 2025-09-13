#include "astro/time_evolution.h"

using namespace astro;

static void TimeEvolveOakTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for (int i = 0; i < state.oak_trees.size(); i++) {
    const auto& tree = state.oak_trees[i];

    auto& trunk = objects(meshes.oak_tree_trunk)[i];

    trunk.scale = tree.scale * tVec3f(0.2f, 1.f, 0.2f);

    commit(trunk);
  }
}

static void TimeEvolveWillowTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for (int i = 0; i < state.willow_trees.size(); i++) {
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
}