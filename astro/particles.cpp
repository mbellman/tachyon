#include "astro/particles.h"

using namespace astro;

const static float angle_offsets[] = {
  0.6f,
  -0.8f,
  0.4f,
  -0.5f
};

const static float heights[] = {
  8000.f,
  18000.f,
  9500.f,
  12000.f
};

const static float radii[] = {
  500.f,
  200.f,
  400.f,
  700.f
};

const static float lifetimes[] = {
  4.f,
  4.6f,
  5.2f,
  5.8f
};

static inline float pow10(const float t) {
  float p5 = t * t * t * t * t;

  return p5 * p5;
}

static void HandleAstroParticles(Tachyon* tachyon, State& state) {
  float turn_duration = time_since(state.game_time_at_start_of_turn);
  turn_duration -= 0.3f;
  if (turn_duration < 0.f) turn_duration = 0.f;

  float rotation_rate = state.last_astro_turn_direction * -0.1f;

  for (size_t i = 0; i < state.astro_light_ids.size(); i++) {
    int32 light_id = state.astro_light_ids[i];
    auto& light = *get_point_light(light_id);
    int i_mod_4 = i % 4;

    float spread_level = floorf(float(i) / 10.f);
    float angle_alpha = float(i) / 10.f;

    float angle =
      // Base angular position
      t_TAU * angle_alpha +
      // Offset angles with each spread level
      spread_level +
      // Subtle random offsets
      angle_offsets[i_mod_4] +
      // Rotation
      rotation_rate * get_scene_time();

    float max_spread = 12000.f - 800.f * spread_level;
    float t = turn_duration - spread_level * 0.1f;
    if (t < 0.f) t = 0.f;
    float spread = max_spread * (t / (t + 0.3f));
    float spread_progress = spread / max_spread;

    float max_particle_intensity;

    if (state.game_time_at_start_of_turn != 0.f && turn_duration > 0.f) {
      float lifetime = lifetimes[i_mod_4];
      float lifetime_alpha = turn_duration * (1.f / lifetime);
      if (lifetime_alpha > 1.f) lifetime_alpha = 1.f;

      max_particle_intensity = Tachyon_Lerpf(2.f, 0.f, lifetime_alpha);
      max_particle_intensity = sqrtf(max_particle_intensity);
    } else {
      max_particle_intensity = 0.f;
    }

    light.power = spread_progress * max_particle_intensity;
    light.radius = radii[i_mod_4];
    light.glow_power = 2.f * spread_progress * max_particle_intensity;
    light.color = tVec3f(1.f, 0.7f, 0.2f);

    light.position = state.player_position_at_start_of_turn;
    light.position.y -= 1500.f - heights[i_mod_4] * pow10(spread_progress);
    light.position.x += spread * sinf(angle);
    light.position.z += spread * cosf(angle);
  }
}

void Particles::InitParticles(Tachyon* tachyon, State& state) {
  for (int i = 0; i < 70; i++) {
    state.astro_light_ids.push_back(create_point_light());
  }
}

void Particles::HandleParticles(Tachyon* tachyon, State& state) {
  profile("HandleAstroParticles()");

  HandleAstroParticles(tachyon, state);
}