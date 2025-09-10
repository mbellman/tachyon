#include "astro/time_evolution.h"

using namespace astro;

static void TimeEvolveOakTrees(Tachyon* tachyon, State& state) {
  // @todo
}

static void TimeEvolveWillowTrees(Tachyon* tachyon, State& state) {
  // @todo
}

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  state.astro_time += 0.01f * dt;

  TimeEvolveOakTrees(tachyon, state);
  TimeEvolveWillowTrees(tachyon, state);
}