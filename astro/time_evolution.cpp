#include "astro/time_evolution.h"

using namespace astro;

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  state.astro_time += 0.01f * dt;
}