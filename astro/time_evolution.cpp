#include "astro/time_evolution.h"
#include "astro/astrolabe.h"
#include "astro/entity_dispatcher.h"

using namespace astro;

static tVec3f GetLightColor(const float astro_time, bool is_nighttime) {
  if (is_nighttime) {
    // @temporary
    return tVec3f(0.05f, 0.05f, 0.2f);
  }

  auto& periods = astro_time_periods;

  tVec3f present_color = tVec3f(0.5f, 0.1f, 0.4f);
  tVec3f past_color = tVec3f(0.6f, 0.3f, 0.1f);
  tVec3f distant_past_color = tVec3f(0.6f, 0.5f, 2.f);

  if (astro_time < periods.present && astro_time >= periods.past) {
    float age_duration = periods.present - periods.past;
    float alpha = (astro_time - periods.past) / age_duration;

    return tVec3f::lerp(past_color, present_color, alpha);
  }

  if (astro_time < periods.past && astro_time >= periods.distant_past) {
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

void TimeEvolution::UpdateAstroTime(Tachyon* tachyon, State& state) {
  profile("UpdateAstroTime()");

  for_all_entity_types() {
    EntityDispatcher::TimeEvolve(tachyon, state, type);
  }

  // Water
  tachyon->fx.water_time += state.dt + 2.f * state.astro_turn_speed;

  // Directional light
  // @todo factor
  tVec3f start_direction = tVec3f(-1.f, -1.f, 0.2f);
  tVec3f end_direction = tVec3f(0.2f, -1.f, 1.f);

  float alpha = -1.f * (state.astro_time / 250.f);

  // Primary + ambient sky light
  {
    auto& scene = tachyon->scene;

    scene.primary_light_color = GetLightColor(state.astro_time, state.is_nighttime);

    // @todo unit() this in the renderer
    scene.primary_light_direction = tVec3f::lerp(start_direction, end_direction, alpha).unit();

    if (state.is_nighttime) {
      scene.sky_light_color = tVec3f(0, 0, 0.4f);
    } else {
      scene.sky_light_color = tVec3f(0.2f, 0.5f, 1.0f) * 0.2f;
    }
  }
}