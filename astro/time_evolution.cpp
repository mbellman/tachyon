#include "astro/time_evolution.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

// @todo remove once we add entity_behaviors/WillowTree.h
static void TimeEvolveWillowTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for_entities(state.willow_trees) {
    const auto& tree = state.willow_trees[i];

    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.scale = tree.scale;

    commit(trunk);
  }
}

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  state.astro_time += 0.01f * dt;

  for_all_entity_types() {
    EntityDispatcher::TimeEvolve(tachyon, state, type);
  }

  // @temporary
  // @todo unit() this in the renderer
  tachyon->scene.primary_light_direction = tVec3f(1.f - state.astro_time / 200.f, -1.f, -0.2f).unit();
}