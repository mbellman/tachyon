#include "astro/time_evolution.h"
#include "astro/astrolabe.h"
#include "astro/entity_dispatcher.h"
#include "astro/sfx.h"

using namespace astro;

static tVec3f GetLightColor(const float astro_time, bool is_nighttime) {
  if (is_nighttime) {
    // @temporary
    return tVec3f(0.05f, 0.05f, 0.2f);
  }

  auto& periods = astro_time_periods;

  tVec3f present_color = tVec3f(0.5f, 0.1f, 0.5f);
  tVec3f past_color = tVec3f(0.8f, 0.2f, 0.1f);
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

static void StopAstroTraveling(State& state) {
  state.astro_time = state.target_astro_time;
  state.astro_turn_speed = 0.f;
  state.time_warp_end_radius = 0.f;

  Sfx::FadeOutSound(SFX_ASTRO_TRAVEL, 2000);
  Sfx::PlaySound(SFX_ASTRO_END, 1.f);
}

void TimeEvolution::StartAstroTraveling(Tachyon* tachyon, State& state, const float target_time) {
  state.target_astro_time = target_time;
  state.game_time_at_start_of_turn = get_scene_time();
  state.time_warp_start_radius = 0.f;

  Sfx::FadeOutSound(SFX_ASTRO_END, 500);
  Sfx::PlaySound(SFX_ASTRO_TRAVEL, 0.8f);
}

void TimeEvolution::HandleAstroTravel(State& state) {
  const float astro_travel_rate = 0.8f;

  const float max_astro_time = Astrolabe::GetMaxAstroTime(state);
  const float min_astro_time = Astrolabe::GetMinAstroTime(state);
  const float max_astro_turn_speed = Astrolabe::GetMaxTurnSpeed();

  if (state.astro_time > state.target_astro_time) {
    // Travel backward
    state.astro_turn_speed -= astro_travel_rate * state.dt;

    // Start slowing down travel speed as we approach the target time (backward)
    float slowdown_threshold = state.target_astro_time + 8.f;

    if (state.astro_time < slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = abs(min_astro_time - slowdown_threshold);
      float slowdown_factor = 20.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * state.dt;
    }
  }
  else if (state.astro_time < state.target_astro_time) {
    // Travel forward
    state.astro_turn_speed += astro_travel_rate * state.dt;

    // Start slowing down travel speed as we approach the target time (forward)
    float slowdown_threshold = state.target_astro_time - 8.f;

    if (state.astro_time > slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = max_astro_time - slowdown_threshold;
      float slowdown_factor = 20.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * state.dt;
    }
  }

  // Limit astro turn speed
  if (state.astro_turn_speed > max_astro_turn_speed) {
    state.astro_turn_speed = max_astro_turn_speed;
  } else if (state.astro_turn_speed < -max_astro_turn_speed) {
    state.astro_turn_speed = -max_astro_turn_speed;
  }

  if (state.astro_turn_speed != 0.f) {
    // Update astro time
    state.astro_time += state.astro_turn_speed * 100.f * state.dt;
  }

  if (state.astro_turn_speed > 0.f && state.astro_time > state.target_astro_time) {
    StopAstroTraveling(state);
  }

  if (state.astro_turn_speed < 0.f && state.astro_time < state.target_astro_time) {
    StopAstroTraveling(state);
  }
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
      scene.sky_light_color = tVec3f(0.4f, 0.5f, 1.f) * 0.2f;
    }
  }
}