#include "astro/time_evolution.h"
#include "astro/astrolabe.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

static tVec3f GetLightColor(const float astro_time) {
  auto& periods = astro_time_periods;

  tVec3f present_color = tVec3f(1.f, 0.4f, 0.8f);
  tVec3f past_color = tVec3f(1.f, 0.8f, 0.2f);
  tVec3f distant_past_color = tVec3f(0.6f, 0.5f, 2.f);

  if (astro_time <= periods.present && astro_time > periods.past) {
    float age_duration = periods.present - periods.past;
    float alpha = (astro_time - periods.past) / age_duration;

    return tVec3f::lerp(past_color, present_color, alpha);
  }

  if (astro_time <= periods.past && astro_time > periods.distant_past) {
    float age_duration = periods.past - periods.distant_past;
    float alpha = (astro_time - periods.distant_past) / age_duration;

    return tVec3f::lerp(distant_past_color, past_color, alpha);
  }

  if (astro_time < periods.distant_past) {
    // @todo lerp between very distant past -> distant past
    return distant_past_color;
  }

  return present_color;
}

void TimeEvolution::UpdateAstroTime(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateAstroTime()");

  for_all_entity_types() {
    EntityDispatcher::TimeEvolve(tachyon, state, type, dt);
  }

  // @todo factor
  auto& light_direction = tachyon->scene.primary_light_direction;
  auto& light_color = tachyon->scene.primary_light_color;

  tVec3f start_direction = tVec3f(-1.f, -1.f, 0.2f);
  tVec3f end_direction = tVec3f(0.2f, -1.f, 1.f);

  float alpha = -1.f * (state.astro_time / 250.f);

  light_color = GetLightColor(state.astro_time);

  // @todo unit() this in the renderer
  light_direction = tVec3f::lerp(start_direction, end_direction, alpha).unit();
}