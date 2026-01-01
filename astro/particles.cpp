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

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

static bool EntityHasAmbientParticle(State& state, const GameEntity& entity) {
  for (auto& particle : state.ambient_particles) {
    if (particle.spawning_entity_id == entity.id) {
      return true;
    }
  }

  return false;
}

static void SpawnAmbientParticle(Tachyon* tachyon, State& state, const GameEntity& spawning_entity) {
  // Create + add particle
  AmbientParticle particle;
  particle.spawn_time = get_scene_time();
  particle.lifetime = Tachyon_GetRandom(3.f, 4.f);
  particle.spawn_position = spawning_entity.position;
  particle.light_id = create_point_light();
  particle.spawning_entity_id = spawning_entity.id;

  state.ambient_particles.push_back(particle);

  // Initialize light
  auto& light = *get_point_light(particle.light_id);

  light.position = spawning_entity.position;
  light.position.y += 2.f * spawning_entity.scale.y;

  light.radius = particle.radius;
  light.color = particle.color;
}

static void UpdateAmbientParticle(Tachyon* tachyon, State& state, AmbientParticle& particle) {
  auto& light = *get_point_light(particle.light_id);
  float alpha = time_since(particle.spawn_time) / particle.lifetime;
  float angle = alpha * t_PI + particle.spawn_time + particle.spawn_position.x;

  light.position.y += 500.f * state.dt;

  light.position.x = particle.spawn_position.x + 600.f * alpha * sinf(angle);
  light.position.z = particle.spawn_position.z + 600.f * alpha * cosf(angle);

  if (alpha < 0.5f) {
    light.power = alpha * 2.f;
  } else if (alpha < 1.f) {
    light.power = 1.f - 2.f * (alpha - 0.5f);
  } else {
    light.power = 0.f;
  }

  light.radius = 500.f + 500.f * light.power;
}

static void RemoveExpiredAmbientParticles(Tachyon* tachyon, State& state) {
  for (size_t i = 0; i < state.ambient_particles.size(); i++) {
    auto& particle = state.ambient_particles[i];

    if (time_since(particle.spawn_time) > particle.lifetime) {
      remove_point_light(particle.light_id);

      RemoveFromArray(state.ambient_particles, i);
    }
  }
}

static void SpawnNewAmbientParticles(Tachyon* tachyon, State& state) {
  for_entities(state.bellflowers) {
    auto& entity = state.bellflowers[i];

    if (abs(entity.position.x - state.player_position.x) > 20000.f) continue;
    if (abs(entity.position.z - state.player_position.z) > 20000.f) continue;

    if (!EntityHasAmbientParticle(state, entity)) {
      SpawnAmbientParticle(tachyon, state, entity);
    }
  }

  for_entities(state.starflowers) {
    auto& entity = state.starflowers[i];

    if (abs(entity.position.x - state.player_position.x) > 20000.f) continue;
    if (abs(entity.position.z - state.player_position.z) > 20000.f) continue;

    if (!EntityHasAmbientParticle(state, entity)) {
      SpawnAmbientParticle(tachyon, state, entity);
    }
  }
}

static void UpdateAllAmbientParticles(Tachyon* tachyon, State& state) {
  for (auto& particle : state.ambient_particles) {
    UpdateAmbientParticle(tachyon, state, particle);
  }
}

void Particles::InitParticles(Tachyon* tachyon, State& state) {
  for (int i = 0; i < 70; i++) {
    state.astro_light_ids.push_back(create_point_light());
  }
}

void Particles::HandleParticles(Tachyon* tachyon, State& state) {
  profile("HandleParticles()");

  HandleAstroParticles(tachyon, state);

  SpawnNewAmbientParticles(tachyon, state);
  UpdateAllAmbientParticles(tachyon, state);
  RemoveExpiredAmbientParticles(tachyon, state);
}