#include "astro/time_evolution.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

static float GetLivingEntityProgress(State& state, const GameEntity& entity, const float lifetime) {
  float entity_age = state.astro_time - entity.astro_start_time;
  if (entity_age < 0.f) return 0.f;
  if (entity_age > lifetime) return 1.f;

  return entity_age / lifetime;
}

static void TimeEvolveShrubs(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  const float lifetime = 60.f;

  for_entities(state.shrubs) {
    const auto& shrub = state.shrubs[i];
    float life_progress = GetLivingEntityProgress(state, shrub, lifetime);

    // @todo factor
    auto& branches = objects(meshes.shrub_branches)[i];

    branches.scale = shrub.scale * sinf(life_progress * 0.75f * t_PI);
    branches.position.y = -1500.f + branches.scale.y;

    commit(branches);
  }
}

static void TimeEvolveOakTrees(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  const float lifetime = 200.f;

  // @todo @optimize only iterate over on-screen/in-range entities
  // once that list is built
  for_entities(state.oak_trees) {
    const auto& tree = state.oak_trees[i];
    float life_progress = GetLivingEntityProgress(state, tree, lifetime);

    // @todo factor
    auto& trunk = objects(meshes.oak_tree_trunk)[i];
    float trunk_height = 1.f - powf(1.f - life_progress, 4.f);
    float trunk_thickness = -(cosf(t_PI * life_progress) - 1.f) / 2.f;

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
  for_entities(state.willow_trees) {
    const auto& tree = state.oak_trees[i];

    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.scale = tree.scale;

    commit(trunk);
  }
}

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  state.astro_time += 0.01f * dt;

  TimeEvolveShrubs(tachyon, state);
  TimeEvolveOakTrees(tachyon, state);
  TimeEvolveWillowTrees(tachyon, state);

  // @temporary
  // @todo unit() this in the renderer
  tachyon->scene.primary_light_direction = tVec3f(1.f - state.astro_time / 200.f, -1.f, -0.2f).unit();
}