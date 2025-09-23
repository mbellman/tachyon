#include "astro/time_evolution.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

void TimeEvolution::HandleAstroTime(Tachyon* tachyon, State& state, const float dt) {
  // @todo allow this once the appropriate item is obtained
  if (state.astro_time < 0.f) {
    state.astro_time += 0.01f * dt;
  }

  for_all_entity_types() {
    EntityDispatcher::TimeEvolve(tachyon, state, type);
  }

  // @temporary
  // @todo unit() this in the renderer
  tachyon->scene.primary_light_direction = tVec3f(1.f - state.astro_time / 200.f, -1.f, -0.2f).unit();
}