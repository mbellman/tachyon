#include "astro/control_system.h"
#include "astro/combat.h"
#include "astro/astrolabe.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"
#include "astro/items.h"
#include "astro/player_character.h"
#include "astro/sfx.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
#include "astro/time_evolution.h"
#include "astro/ui_system.h"

using namespace astro;

const static float dodge_cooldown_time = 0.3f;
const static float target_jump_cooldown_time = 0.5f;
const static float strong_attack_cooldown_time = 0.5f;

static float GetTurnDot(Tachyon* tachyon, State& state) {
  tVec2f stick_direction = tVec2f(tachyon->left_stick.x, tachyon->left_stick.y);

  if (stick_direction.x == 0.f && stick_direction.y == 0.f) {
    return 0.f;
  }

  stick_direction = stick_direction.unit();
  tVec3f facing_direction = state.player_facing_direction.xz().unit();

  return stick_direction.x * facing_direction.x + stick_direction.y * facing_direction.z;
}

// @todo PlayerCharacter::
static void HandleQuickManeuverAction(Tachyon* tachyon, State& state) {
  if (state.has_target) {
    auto& target = *EntityManager::FindEntity(state, state.target_entity);
    tVec3f target_direction = (target.visible_position - state.player_position).unit();
    tVec3f move_direction = state.player_velocity.unit();
    float target_dot = tVec3f::dot(move_direction, target_direction);

    if (target_dot > 0.96f) {
      PlayerCharacter::PerformTargetJumpAction(tachyon, state);

      return;
    }
  }

  PlayerCharacter::PerformStandardDodgeAction(tachyon, state);
}

// @todo continue to work on this
static void HandleExperimentalControls(Tachyon* tachyon, State& state, const float movement_speed) {
  tVec3f forward = state.player_facing_direction;
  tVec3f left = tVec3f::cross(state.player_facing_direction, tVec3f(0, 1.f, 0));

  state.player_velocity += state.player_facing_direction * -tachyon->left_stick.y * movement_speed * state.dt;
  state.player_velocity += left * tachyon->left_stick.x * movement_speed * state.dt;
}

static void HandlePlayerMovementControls(Tachyon* tachyon, State& state) {
  bool is_running = is_key_held(tKey::CONTROLLER_A) || is_key_held(tKey::SHIFT);

  // Disable movement during:
  if (
    // Astro travel
    abs(state.astro_turn_speed) > 0.1f ||
    // Wind chime actions
    (state.last_wind_chimes_action_time != 0.f && time_since(state.last_wind_chimes_action_time) < 4.25f) ||
    // Camera events
    state.camera_events.size() > 0 ||
    // Dodge actions
    time_since(state.last_dodge_time) < dodge_cooldown_time ||
    // Target jump actions
    time_since(state.last_target_jump_time) < target_jump_cooldown_time
  ) {
    return;
  }

  // Directional movement
  float movement_speed =
    is_running ? 14000.f :
    state.has_target ? 8000.f :
    4000.f;

  if (is_key_held(tKey::W)) {
    state.player_velocity += tVec3f(0, 0, -1.f) * movement_speed * state.dt;
  }

  if (is_key_held(tKey::A)) {
    state.player_velocity += tVec3f(-1.f, 0, 0) * movement_speed * state.dt;
  }

  if (is_key_held(tKey::D)) {
    state.player_velocity += tVec3f(1.f, 0, 0) * movement_speed * state.dt;
  }

  if (is_key_held(tKey::S)) {
    state.player_velocity += tVec3f(0, 0, 1.f) * movement_speed * state.dt;
  }

  state.player_velocity.x += tachyon->left_stick.x * movement_speed * state.dt;
  state.player_velocity.z += tachyon->left_stick.y * movement_speed * state.dt;

  // HandleExperimentalControls(tachyon, state, movement_speed);

  // Track run input timings to use for dodges
  if (did_press_key(tKey::CONTROLLER_A)) {
    state.last_run_input_time = get_scene_time();
  }

  // Tapping A/X while moving to perform quick maneuvers
  if (
    state.targetable_entities.size() > 0 &&
    did_release_key(tKey::CONTROLLER_A) &&
    time_since(state.last_run_input_time) < 0.3f &&
    time_since(state.last_damage_time) > 0.5f &&
    state.player_velocity.magnitude() > 300.f
  ) {
    HandleQuickManeuverAction(tachyon, state);
  }

  // Quick turning
  if (GetTurnDot(tachyon, state) < -0.5f && !state.has_target) {
    state.last_quick_turn_time = get_scene_time();
  }
}

// @disabled
// @todo remove
static void HandleAstroControls(Tachyon* tachyon, State& state) {
  const float astro_travel_rate = 0.8f;
  const float astro_slowdown_rate = 3.f;

  const float max_astro_time = Astrolabe::GetMaxAstroTime(state);
  const float min_astro_time = Astrolabe::GetMinAstroTime(state);
  const float max_turn_speed = Astrolabe::GetMaxTurnSpeed();

  float previous_astro_turn_speed = state.astro_turn_speed;

  bool started_turning = (
    (tachyon->left_trigger > 0.f && tachyon->right_trigger == 0.f && state.last_frame_left_trigger == 0.f) ||
    (tachyon->right_trigger > 0.f && tachyon->left_trigger == 0.f && state.last_frame_right_trigger == 0.f)
  );

  state.last_frame_left_trigger = tachyon->left_trigger;
  state.last_frame_right_trigger = tachyon->right_trigger;

  bool stopped_turning = false;

  // Track the initial time when we start turning
  // so we can determine when to start slowing down
  // before hitting min/max time
  if (started_turning) {
    state.astro_time_at_start_of_turn = state.astro_time;
    state.game_time_at_start_of_turn = get_scene_time();
    state.astro_particle_spawn_position = state.player_position;
    state.is_astrolabe_stopped = false;

    if (state.has_target) {
      // Force deselection of the current target
      Targeting::DeselectCurrentTarget(tachyon, state);

      // Zero out player velocity to prevent changing the facing direction,
      // e.g. if we were backwalking away from the enemy when we started
      // the astro turn, and our velocity vector would otherwise swing us
      // around awkwardly
      state.player_velocity = tVec3f(0.f);
    }
  }

  // Handle reverse/forward turn actions
  state.astro_turn_speed -= tachyon->left_trigger * astro_travel_rate * state.dt;
  state.astro_turn_speed += tachyon->right_trigger * astro_travel_rate * state.dt;

  // Prevent time changes past max time
  if (state.astro_time >= max_astro_time && state.astro_turn_speed > 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn >= -1.f) {
      UISystem::ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");

      state.game_time_at_start_of_turn = 0.f;

      if (started_turning) {
        Sfx::PlaySound(SFX_ASTRO_DISABLED, 0.8f);
      }
    }
  }

  // Prevent time changes before min time
  if (state.astro_time <= min_astro_time && state.astro_turn_speed < 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn <= min_astro_time + 1.f) {
      UISystem::ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");

      state.game_time_at_start_of_turn = 0.f;

      if (started_turning) {
        Sfx::PlaySound(SFX_ASTRO_DISABLED, 0.8f);
      }
    }
  }

  // Slow down toward max time
  if (state.astro_turn_speed > 0.f) {
    float slowdown_threshold = max_astro_time - (max_astro_time - state.astro_time_at_start_of_turn) * 0.2f;

    if (slowdown_threshold < max_astro_time - 10.f) {
      // Enforce a limit on how far away the slowdown threshold is
      // from the stopping value
      slowdown_threshold = max_astro_time - 10.f;
    }

    if (slowdown_threshold > max_astro_time - 8.f) {
      slowdown_threshold = max_astro_time - 8.f;
    }

    if (state.astro_time > slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = max_astro_time - slowdown_threshold;
      float slowdown_factor = 20.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * state.dt;

      if (abs(max_astro_time - state.astro_time) < 2.f) {
        UISystem::ShowDialogue(tachyon, state, "The astrolabe stopped turning.");

        stopped_turning = true;
      }
    }
  }

  // Slow down toward min time
  if (state.astro_turn_speed < 0.f) {
    float slowdown_threshold = min_astro_time + abs(state.astro_time_at_start_of_turn - min_astro_time) * 0.2f;

    if (slowdown_threshold > min_astro_time + 10.f) {
      // Enforce a limit on how far away the slowdown threshold is
      // from the stopping value
      slowdown_threshold = min_astro_time + 10.f;
    }

    if (slowdown_threshold < min_astro_time + 8.f) {
      slowdown_threshold = min_astro_time + 8.f;
    }

    if (state.astro_time < slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = abs(min_astro_time - slowdown_threshold);
      float slowdown_factor = 20.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * state.dt;

      if (
        state.astro_time_at_start_of_turn > min_astro_time + 1.f &&
        abs(state.astro_time - min_astro_time) < 2.f
      ) {
        UISystem::ShowDialogue(tachyon, state, "The astrolabe stopped turning.");

        stopped_turning = true;
      }
    }
  }

  // Advance (or reverse) astro time
  state.astro_time += state.astro_turn_speed * 100.f * state.dt;

  // Clamp to min/max astro time
  if (state.astro_time > max_astro_time) state.astro_time = max_astro_time;
  if (state.astro_time < min_astro_time) state.astro_time = min_astro_time;

  if (tachyon->left_trigger == 0.f && tachyon->right_trigger == 0.f) {
    // Reduce turn rate gradually
    state.astro_turn_speed *= 1.f - astro_slowdown_rate * state.dt;

    // Stop turning at a low enough speed
    if (abs(state.astro_turn_speed) < 0.01f) {
      state.astro_turn_speed = 0.f;
    }
  }
  else if (state.astro_turn_speed > max_turn_speed) {
    // Cap forward speed
    state.astro_turn_speed = max_turn_speed;
  }
  else if (state.astro_turn_speed < -max_turn_speed) {
    // Cap reverse speed
    state.astro_turn_speed = -max_turn_speed;
  }

  // Check for stopped astro turns
  if (
    // Turning against min/max astro time
    (started_turning && state.astro_turn_speed == 0.f) ||

    // Turning slowed down below a given threshold. The threshold
    // is a bit high (0.1) so that the "stopped" sound begins playing
    // sooner, rather than waiting for full deceleration to 0.
    (
      (abs(previous_astro_turn_speed) >= 0.1f && abs(state.astro_turn_speed) < 0.1f) &&
      (tachyon->left_trigger == 0.f || state.astro_time <= min_astro_time) &&
      (tachyon->right_trigger == 0.f || state.astro_time >= max_astro_time)
    ) ||

    // Turn speed is 0, but not set in state as stopped. If we merely
    // tap the trigger buttons, we may not speed up enough to hit the
    // above 0.1 threshold.
    (state.astro_turn_speed == 0.f && !state.is_astrolabe_stopped)
  ) {
    stopped_turning = true;
  }

  // Sound effects
  {
    if (stopped_turning && !state.is_astrolabe_stopped) {
      Sfx::FadeOutSound(SFX_ASTRO_TRAVEL, 2000);

      if (state.is_astro_traveling) {
        Sfx::PlaySound(SFX_ASTRO_END, 1.f);
      }

      state.is_astrolabe_stopped = true;
      state.is_astro_traveling = false;
      state.time_warp_end_radius = 0.f;
    }
    else if (started_turning) {
      Sfx::FadeOutSound(SFX_ASTRO_END, 500);
      Sfx::PlaySound(SFX_ASTRO_TRAVEL, 0.8f);

      state.is_astrolabe_stopped = false;
      state.is_astro_traveling = true;
      state.time_warp_start_radius = 0.f;
    }
  }
}

// @temporary
static void HandleDayNightControls(Tachyon* tachyon, State& state) {
  // Triangle
  if (did_press_key(tKey::CONTROLLER_Y)) {
    state.is_nighttime = !state.is_nighttime;
  }
}

// @todo Magic:: (???) (or elsewhere???)
static bool TestWindChimesAction(Tachyon* tachyon, State& state) {
  const float distance_threshold = 5000.f;

  float scene_time = get_scene_time();
  float player_speed = state.player_velocity.magnitude();

  for (auto& entity : state.wind_chimes) {
    float distance = tVec3f::distance(entity.position, state.player_position);

    if (
      distance < distance_threshold &&
      time_since(entity.game_activation_time) > 1.f
    ) {
      // @todo factor
      entity.game_activation_time = scene_time;

      state.astro_particle_spawn_position = entity.position;
      state.last_wind_chimes_action_time = scene_time;
      state.last_used_wind_chimes_id = entity.id;

      return true;
    }
  }

  return false;
}

static void HandleWandControls(Tachyon* tachyon, State& state) {
  if (abs(state.astro_turn_speed) != 0.f) {
    return;
  }

  // O
  if (did_press_key(tKey::CONTROLLER_B)) {
    if (Items::HasItem(state, ITEM_STUN_SPELL)) {
      SpellSystem::CastStun(tachyon, state);

      Sfx::PlaySound(SFX_SPELL_STUN);
    }
  }

  // Square
  if (did_press_key(tKey::CONTROLLER_X)) {
    // If we're performing a wind chimes action, stop here
    if (TestWindChimesAction(tachyon, state)) return;

    if (Items::HasItem(state, ITEM_HOMING_SPELL)) {
      // @todo magic weapons
      SpellSystem::CastHoming(tachyon, state);
    }
    else if (time_since(state.last_wand_swing_time) > 0.7f) {
      // Before we have magic weapons, swing the wand as a melee weapon
      state.last_wand_swing_time = get_scene_time();
      state.last_wand_bounce_time = 0.f;

      Combat::HandleWandSwing(tachyon, state);
    }
  }

  // Triangle
  if (did_press_key(tKey::CONTROLLER_Y)) {
    // @todo
  }
}

static void HandleTargetingControls(Tachyon* tachyon, State& state) {
  float time_since_last_target_change = time_since(state.target_start_time);

  bool did_stick_select_previous_target = (
    time_since_last_target_change > 0.2f &&
    tachyon->right_stick.x < -0.6f
  );

  bool did_stick_select_next_target = (
    time_since_last_target_change > 0.2f &&
    tachyon->right_stick.x > 0.6f
  );

  if (did_press_key(tKey::CONTROLLER_L1) || did_stick_select_previous_target) {
    Targeting::SelectPreviousAccessibleTarget(tachyon, state);
  }

  if (did_press_key(tKey::CONTROLLER_R1) || did_stick_select_next_target) {
    Targeting::SelectNextAccessibleTarget(tachyon, state);
  }

  if (did_press_key(tKey::CONTROLLER_R3)) {
    if (state.has_target) {
      Targeting::DeselectCurrentTarget(tachyon, state);
    } else {
      Targeting::SelectNextAccessibleTarget(tachyon, state);
    }
  }
}

static void HandleSpeedLimiting(Tachyon* tachyon, State& state) {
  bool is_running = is_key_held(tKey::CONTROLLER_A) || is_key_held(tKey::SHIFT);
  bool is_dodging = time_since(state.last_dodge_time) < dodge_cooldown_time;
  bool is_target_jumping = time_since(state.last_target_jump_time) < target_jump_cooldown_time;
  bool is_doing_wind_chimes_action = time_since(state.last_wind_chimes_action_time) < 4.f;

  if (is_target_jumping) {
    state.player_velocity *= 1.f - 2.f * state.dt;
  }
  else if (is_dodging) {
    state.player_velocity *= 1.f - 4.f * state.dt;
  }
  else if (is_doing_wind_chimes_action) {
    state.player_velocity *= 1.f - 6.f * state.dt;
  }
  else {
    state.player_velocity *= 1.f - 3.f * state.dt;
  }

  float speed = state.player_velocity.magnitude();

  float max_speed =
    is_running ? PlayerCharacter::MAX_RUN_SPEED :
    state.has_target ? PlayerCharacter::MAX_COMBAT_WALK_SPEED :
    PlayerCharacter::MAX_WALK_SPEED;

  if (speed > max_speed && !is_dodging && !is_target_jumping) {
    tVec3f unit_velocity = state.player_velocity / speed;

    state.player_velocity = unit_velocity * max_speed;
  }
}

static void UpdatePlayerPosition(State& state) {
  // @todo remove this * 5.f bit and properly manage velocity
  state.player_position += state.player_velocity * 5.f * state.dt;
}

void ControlSystem::HandleControls(Tachyon* tachyon, State& state) {
  // Only allow character controls if:
  if (
    // We do not have, or have dismissed blocking dialogue
    (!state.has_blocking_dialogue || state.dismissed_blocking_dialogue) &&
    // We aren't doing a strong attack
    time_since(state.last_strong_attack_time) > 0.5f &&
    // We aren't dead
    state.player_hp > 0.f
  ) {
    HandlePlayerMovementControls(tachyon, state);
    // HandleAstroControls(tachyon, state);
    HandleDayNightControls(tachyon, state);
    HandleWandControls(tachyon, state);
    HandleTargetingControls(tachyon, state);
  }

  HandleSpeedLimiting(tachyon, state);
  UpdatePlayerPosition(state);
}