#include "astro/particles.h"

using namespace astro;

static inline float pow10(const float t) {
  float p5 = t * t * t * t * t;

  return p5 * p5;
}

static void HandleAstroParticles(Tachyon* tachyon, State& state) {
  const float angle_offsets[] = {
    0.6f,
    -0.8f,
    0.4f,
    -0.5f
  };

  const float heights[] = {
    3000.f,
    7500.f,
    3800.f,
    5200.f
  };

  const float radii[] = {
    500.f,
    300.f,
    600.f
  };

  float max_particle_intensity;

  float turn_duration = time_since(state.game_time_at_start_of_turn);
  turn_duration -= 0.3f;
  if (turn_duration < 0.f) turn_duration = 0.f;

  if (state.game_time_at_start_of_turn != 0.f && turn_duration > 0.f) {
    float lifetime_alpha = turn_duration * 0.25f;
    if (lifetime_alpha > 1.f) lifetime_alpha = 1.f;

    max_particle_intensity = Tachyon_Lerpf(2.f, 0.f, lifetime_alpha);
    max_particle_intensity = sqrtf(max_particle_intensity);
  } else {
    max_particle_intensity = 0.f;
  }

  float rotation_rate = state.astro_turn_speed < 0.f ? 0.1f : -0.1f;

  for (size_t i = 0; i < state.astro_light_ids.size(); i++) {
    int32 light_id = state.astro_light_ids[i];
    auto& light = *get_point_light(light_id);

    float spread_level = floorf(float(i) / 10.f);

    float angle_alpha = (float(i) / 10.f);

    float angle =
      // Base angular position
      t_TAU * angle_alpha +
      // Offset angles with each spread level
      spread_level +
      // Subtle random offsets
      angle_offsets[i % 4] +
      // Rotation
      rotation_rate * get_scene_time();

    float max_spread = 6000.f + 1500.f * spread_level;
    float t = turn_duration - spread_level * 0.2f;
    if (t < 0.f) t = 0.f;
    float spread = max_spread * (t / (t + 0.3f));
    float spread_alpha = spread / max_spread;

    light.power = spread_alpha * max_particle_intensity;
    light.radius = radii[i % 3];
    light.glow_power = 2.f * spread_alpha * max_particle_intensity;
    light.color = tVec3f(1.f, 0.7f, 0.2f);

    light.position = state.player_position;
    light.position.y -= 1500.f - heights[i % 4] * pow10(spread_alpha);
    light.position.x += spread * sinf(angle);
    light.position.z += spread * cosf(angle);
  }
}

void Particles::HandleParticles(Tachyon* tachyon, State& state) {
  profile("HandleAstroParticles()");

  HandleAstroParticles(tachyon, state);
}