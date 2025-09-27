#include "astro/spell_system.h"

using namespace astro;

void SpellSystem::CastStun(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.last_stun_time = tachyon->running_time;
  spells.stun_light_id = create_point_light();
}

void SpellSystem::CastHoming(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.last_homing_time = tachyon->running_time;
  spells.homing_light_id = create_point_light();

  auto& light = *get_point_light(spells.homing_light_id);

  light.position = state.player_position;

  // @temporary
  spells.homing_target_entity.id = -1;
}

void SpellSystem::HandleSpells(Tachyon* tachyon, State& state, const float dt) {
  auto& spells = state.spells;

  // Stun spells
  {
    if (spells.stun_light_id != -1) {
      auto& light = *get_point_light(spells.stun_light_id);
      float t = (tachyon->running_time - spells.last_stun_time) / 3.f;
      if (t > 1.f) t = 1.f;

      light.position = state.player_position + tVec3f(800.f, 1000.f, -800.f);
      light.position.y += sqrtf(t) * 1200.f;
      light.radius = 25000.f * Tachyon_EaseInOutf(t);
      light.color = tVec3f(1.f, 0.8f, 0.4f),
      light.power = 5.f * powf(1.f - t, 2.f);
    }
  }

  // Homing spells
  {
    if (spells.homing_light_id != -1) {
      auto& light = *get_point_light(spells.homing_light_id);

      light.position += tVec3f(0, 0, -1.f) * 10000.f * dt;
      light.radius = 10000.f;
      light.color = tVec3f(0.1f, 0.3f, 1.f);
      light.power = 5.f;
    }
  }
}