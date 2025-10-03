#include "astro/spell_system.h"
#include "astro/entity_manager.h"

using namespace astro;

/**
 * ----------------------------
 * Stun spell helper functions.
 * ----------------------------
 */
static void HandleActiveStunSpell(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  if (spells.stun_light_id == -1) {
    return;
  }

  auto& light = *get_point_light(spells.stun_light_id);
  float t = (tachyon->running_time - spells.stun_start_time) / 3.f;
  if (t > 1.f) t = 1.f;
  t = sqrtf(t);

  light.position = state.player_position + tVec3f(800.f, 1000.f, -800.f);
  light.position.y += sqrtf(t) * 1200.f;
  light.radius = 25000.f * Tachyon_EaseInOutf(t);
  light.color = tVec3f(1.f, 0.8f, 0.4f),
  light.power = 5.f * powf(1.f - t, 2.f);
}

/**
 * ----------------------------
 * Homing spell helper functions.
 * ----------------------------
 */
static void HandleHomingSpellCircling(Tachyon* tachyon, State& state, HomingOrb& orb, tPointLight& light, const int index) {
  auto& spells = state.spells;
  float time_since_casting = tachyon->running_time - spells.homing_start_time;

  // Define an adjusted time value for homing lights, base on index,
  // so each one can behave in succession
  float t = time_since_casting - float(index) * 0.4f;
  if (t < 0.f) t = 0.f;

  float alpha = t;
  float clamped_alpha = alpha > 1.f ? 1.f : alpha;

  float theta = -alpha * t_TAU * 0.5f;
  float circle_radius = 1000.f + sqrtf(clamped_alpha) * 1000.f;

  tVec3f start = spells.homing_start_direction;
  tVec3f offset = start;

  offset.x = start.x * cosf(theta) - start.z * sinf(theta);
  offset.z = start.x * sinf(theta) + start.z * cosf(theta);

  light.position = state.player_position + offset * circle_radius;
  light.radius = 3000.f * clamped_alpha;

  if (state.target_entity.type != UNSPECIFIED) {
    // When we have a target, wait until we're within angular range
    // and then fire the orb toward it
    auto& target = *EntityManager::FindEntity(state, state.target_entity);
    tVec3f light_to_target = target.visible_position - light.position;

    // Get the light -> target line angle
    float target_angle = atan2f(light_to_target.z, light_to_target.x) + t_HALF_PI;

    // Get the current angle of the circling orb. We have to use the
    // starting direction angle and the current theta to determine
    // its proper world/global angle.
    float base_angle = atan2f(spells.homing_start_direction.z, spells.homing_start_direction.x);
    float global_angle = base_angle + theta;

    float angle_delta = fmod(abs(target_angle - global_angle), t_TAU);

    if (angle_delta < 0.1f) {
      // Fire toward the target entity
      orb.is_targeting = true;
      orb.targeting_start_time = tachyon->running_time;
    }
  }

  if (t > 5.f) {
    // @todo use a dissipation effect
    remove_point_light(light);
  }
}

static void HandleHomingSpellTargeting(Tachyon* tachyon, State& state, HomingOrb& orb, tPointLight& light, const float dt) {
  float t = tachyon->running_time - orb.targeting_start_time;
  float speed = Tachyon_Lerpf(5000.f, 16000.f, t);
  auto& target_entity = *EntityManager::FindEntity(state, state.target_entity);

  tVec3f light_to_target = target_entity.visible_position - light.position;
  float target_distance = light_to_target.magnitude();
  tVec3f unit_light_to_target = light_to_target / target_distance;

  light.position += unit_light_to_target * speed * dt;
  light.radius = 3000.f;

  if (target_distance < 200.f) {
    // @todo use a dissipation effect
    remove_point_light(light);
  }
}

static void HandleActiveHomingSpell(Tachyon* tachyon, State& state, const float dt) {
  auto& spells = state.spells;

  if (
    spells.homing_orbs[0].light_id == -1 &&
    spells.homing_orbs[1].light_id == -1 &&
    spells.homing_orbs[2].light_id == -1
  ) {
    return;
  }

  // @todo state.game_time - spells.homing_start_time
  float time_since_casting = tachyon->running_time - spells.homing_start_time;

  for (int32 i = 0; i < 3; i++) {
    auto& orb = spells.homing_orbs[i];
    auto* light_pointer = get_point_light(orb.light_id);

    if (light_pointer == nullptr) {
      // Clear out any references to removed lights
      orb.light_id = -1;

      continue;
    }

    auto& light = *light_pointer;

    // Set static light parameters
    light.color = tVec3f(0.1f, 0.3f, 1.f);
    light.power = 3.f;

    if (!orb.is_targeting) {
      HandleHomingSpellCircling(tachyon, state, orb, light, i);
    }
    else if (state.target_entity.type != UNSPECIFIED) {
      HandleHomingSpellTargeting(tachyon, state, orb, light, dt);
    }
  }
}

void SpellSystem::CastStun(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.stun_start_time = tachyon->running_time;
  spells.stun_light_id = create_point_light();
}

void SpellSystem::CastHoming(Tachyon* tachyon, State& state) {
  auto& spells = state.spells;

  spells.homing_start_time = tachyon->running_time;
  // @todo ensure that homing spells can find a target
  // if one hasn't been selected yet, but stick to the
  // first target they do select
  spells.homing_target_entity = state.target_entity;
  spells.homing_start_direction = state.player_facing_direction;

  // Create homing orbs
  for (int32 i = 0; i < 3; i++) {
    auto& orb = spells.homing_orbs[i];
    auto* existing_light = get_point_light(orb.light_id);

    // Removing existing orb lights, if any
    if (existing_light != nullptr) {
      // @todo use a dissipation effect
      remove_point_light(*existing_light);
    }

    orb.light_id = create_point_light();
    orb.is_targeting = false;
    orb.targeting_start_time = 0.f;

    auto& light = *get_point_light(orb.light_id);

    light.position = state.player_position;
    light.power = 0.f;
  }
}

void SpellSystem::HandleSpells(Tachyon* tachyon, State& state, const float dt) {
  auto& spells = state.spells;

  HandleActiveStunSpell(tachyon, state);
  HandleActiveHomingSpell(tachyon, state, dt);
}