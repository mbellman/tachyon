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
  t = sqrtf(t);

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

  float time_since_casting = tachyon->running_time - spells.last_homing_time;

  for (int32 i = 0; i < 3; i++) {
    auto& light = *get_point_light(spells.homing_light_ids[i]);

    // Set static light parameters
    light.color = tVec3f(0.1f, 0.3f, 1.f);
    light.power = 5.f;

    // Define an adjusted time value for each light so they can behave in succession
    float t = time_since_casting - float(i) * 0.4f;
    if (t < 0.f) t = 0.f;

    // Circle the player
    if (t < 2.f) {
      float alpha = t / 2.f;
      float theta = -alpha * t_TAU * 1.25f;
      float circle_radius = sqrtf(alpha) * 2000.f;

      const static tVec3f start = { 0.f, 0.f, 1.f };
      tVec3f offset = start;

      offset.x = start.x * cosf(theta) - start.z * sinf(theta);
      offset.z = start.x * sinf(theta) + start.z * cosf(theta);

      light.position = state.player_position + offset * circle_radius;
      light.radius = 3000.f * alpha;
    }

    // Target the enemy
    // @todo disappear once the lights hit the target
    else {
      float alpha = t - 2.f;
      float speed = Tachyon_Lerpf(7000.f, 16000.f, alpha);

      // @todo use player direction
      tVec3f player_forward = tVec3f(0, 0, -1.f);
      // @todo target_entity->visible_position
      tVec3f light_to_target = target_entity->position - light.position;
      tVec3f direction = tVec3f::lerp(player_forward.unit(), light_to_target.unit(), alpha);

      light.position += direction.unit() * speed * dt;
      light.radius = 3000.f;
    }
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