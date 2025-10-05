#include "astro/time_evolution.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

void TimeEvolution::UpdateAstroTime(Tachyon* tachyon, State& state, const float dt) {
  // @todo allow this once the appropriate item is obtained
  if (state.astro_time < 0.f) {
    state.astro_time += 0.01f * dt;
  }

  for_all_entity_types() {
    EntityDispatcher::TimeEvolve(tachyon, state, type, dt);
  }

  // @temporary
  auto& light_direction = tachyon->scene.primary_light_direction;
  auto& light_color = tachyon->scene.primary_light_color;
  
  tVec3f start_direction = tVec3f(-1.f, -1.f, 0.2f);
  tVec3f start_color =  tVec3f(1.f);

  tVec3f end_direction = tVec3f(0.2f, -1.f, 1.f);
  tVec3f end_color = tVec3f(1.f, 0.1f, 0.1f);

  float alpha = -1.f * (state.astro_time / 250.f);
  
  // @todo unit() this in the renderer
  light_direction = tVec3f::lerp(start_direction, end_direction, alpha).unit();
  light_color = tVec3f::lerp(start_color, end_color, alpha);
}