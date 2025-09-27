#include "astro/spell_system.h"
#include "astro/entity_manager.h"

using namespace astro;

static void HandleActiveStunSpell(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  if (spells.stun_light_id == -1) {
    return;
  }

  auto& light = *get_point_light(spells.stun_light_id);
  float t = (tachyon->running_time - spells.last_stun_time) / 3.f;
  if (t > 1.f) t = 1.f;

  light.position = state.player_position + tVec3f(800.f, 1000.f, -800.f);
  light.position.y += sqrtf(t) * 1200.f;
  light.radius = 25000.f * Tachyon_EaseInOutf(t);
  light.color = tVec3f(1.f, 0.8f, 0.4f),
  light.power = 5.f * powf(1.f - t, 2.f);
}

static void HandleActiveHomingSpell(Tachyon* tachyon, State& state, const float dt) {
  auto& spells = state.spells;

  if (spells.homing_light_ids[0] == -1) {
    return;
  }

  auto* target_entity = EntityManager::FindEntity(state, spells.homing_target_entity);
  
  if (target_entity == nullptr) {
    return;
  }

  float t = tachyon->running_time - spells.last_homing_time;

  for (int32 i = 0; i < 3; i++) {
    float speed_alpha = t - float(i) * 0.4f;
    if (speed_alpha < 0.f) speed_alpha = 0.f;

    auto& light = *get_point_light(spells.homing_light_ids[i]);
    float speed = Tachyon_Lerpf(2000.f, 10000.f, speed_alpha);
    // @todo target_entity->visible_position
    tVec3f target_direction = target_entity->position - light.position;

    if (speed_alpha > 0.f) {
      light.position += target_direction.unit() * speed * dt;
    }

    light.radius = 3000.f;
    light.color = tVec3f(0.1f, 0.3f, 1.f);
    light.power = 5.f;
  }
}

void SpellSystem::CastStun(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.last_stun_time = tachyon->running_time;
  spells.stun_light_id = create_point_light();
}

void SpellSystem::CastHoming(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.last_homing_time = tachyon->running_time;

  // Create lights
  for (int32 i = 0; i < 3; i++) {
    spells.homing_light_ids[i] = create_point_light();

    auto& light = *get_point_light(spells.homing_light_ids[i]);

    light.position = state.player_position;
    light.power = 0.f;
  }

  float closest = std::numeric_limits<float>::max();

  // @todo factor/introduce a targeting system
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];
    float distance = (state.player_position - entity.position).magnitude();

    if (distance < closest) {
      closest = distance;

      spells.homing_target_entity.id = entity.id;
      spells.homing_target_entity.type = entity.type;
    }
  }
}

void SpellSystem::HandleSpells(Tachyon* tachyon, State& state, const float dt) {
  auto& spells = state.spells;

  HandleActiveStunSpell(tachyon, state);
  HandleActiveHomingSpell(tachyon, state, dt);
}