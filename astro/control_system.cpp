#include "astro/control_system.h"
#include "astro/astrolabe.h"
#include "astro/entity_manager.h"
#include "astro/items.h"
#include "astro/sfx.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

using namespace astro;

static void HandlePlayerMovementControls(Tachyon* tachyon, State& state, const float dt) {
  const float dodge_cooldown_time = 0.3f;

  bool is_running = is_key_held(tKey::CONTROLLER_A) || is_key_held(tKey::SHIFT);

  if (state.is_astrolabe_stopped && time_since(state.last_dodge_time) > dodge_cooldown_time) {
    // Directional movement
    float movement_speed = is_running ? 14000.f : 4000.f;

    if (is_key_held(tKey::W)) {
      state.player_velocity += tVec3f(0, 0, -1.f) * movement_speed * dt;
    }

    if (is_key_held(tKey::A)) {
      state.player_velocity += tVec3f(-1.f, 0, 0) * movement_speed * dt;
    }

    if (is_key_held(tKey::D)) {
      state.player_velocity += tVec3f(1.f, 0, 0) * movement_speed * dt;
    }

    if (is_key_held(tKey::S)) {
      state.player_velocity += tVec3f(0, 0, 1.f) * movement_speed * dt;
    }

    state.player_velocity.x += tachyon->left_stick.x * movement_speed * dt;
    state.player_velocity.z += tachyon->left_stick.y * movement_speed * dt;

    // Double-tapping A/X to escape enemies
    if (did_press_key(tKey::CONTROLLER_A)) {
      if (
        time_since(state.last_run_input_time) < 0.3f &&
        state.has_target
      ) {
        auto& target = *EntityManager::FindEntity(state, state.target_entity);
        tVec3f unit_velocity = state.player_velocity.unit();
        tVec3f unit_target_to_player = (state.player_position - target.visible_position).unit();

        // Require that we're actually dodging/dashing away from the enemy to deselect it
        if (tVec3f::dot(unit_velocity, unit_target_to_player) > 0.7f) {
          Targeting::DeselectCurrentTarget(tachyon, state);
        }
      }

      state.last_run_input_time = get_scene_time();
    }

    // Tapping A/X quickly to dodge
    if (
      state.targetable_entities.size() > 0 &&
      did_release_key(tKey::CONTROLLER_A) &&
      time_since(state.last_run_input_time) < 0.3f
    ) {
      state.player_velocity *= 3.5f;
      state.last_dodge_time = get_scene_time();
    }
  }

  // Speed limiting
  // @todo move elsewhere
  {
    state.player_velocity *= 1.f - 6.f * dt;

    float speed = state.player_velocity.magnitude();
    float max_speed = is_running ? 1300.f : 550.f;

    if (
      speed > max_speed &&
      time_since(state.last_dodge_time) > dodge_cooldown_time
    ) {
      tVec3f unit_velocity = state.player_velocity / speed;

      state.player_velocity = unit_velocity * max_speed;
    }
  }
}

static void HandleAstroControls(Tachyon* tachyon, State& state, const float dt) {
  const float astro_start_rate = 0.1f;
  const float astro_travel_rate = 0.8f;
  const float astro_slowdown_rate = 3.f;

  const float max_astro_time = Astrolabe::GetMaxAstroTime(state);
  const float min_astro_time = Astrolabe::GetMinAstroTime(state);
  const float max_turn_speed = Astrolabe::GetMaxTurnSpeed();

  float previous_astro_turn_speed = state.astro_turn_speed;

  bool started_turning = (
    (tachyon->left_trigger > 0.f && state.last_frame_left_trigger == 0.f) ||
    (tachyon->right_trigger > 0.f && state.last_frame_right_trigger == 0.f)
  );

  state.last_frame_left_trigger = tachyon->left_trigger;
  state.last_frame_right_trigger = tachyon->right_trigger;

  if (Targeting::IsInCombatMode(state)) {
    if (started_turning) {
      Sfx::PlaySound(SFX_ASTRO_DISABLED, 0.8f);
    }

    return;
  }

  bool stopped_turning = false;

  // Track the initial time when we start turning
  // so we can determine when to start slowing down
  // before hitting min/max time
  if (started_turning) {
    state.astro_time_at_start_of_turn = state.astro_time;
    state.game_time_at_start_of_turn = get_scene_time();
    state.is_astrolabe_stopped = false;

    // Force deselection of the current target, if any
    Targeting::DeselectCurrentTarget(tachyon, state);
  }

  float astro_acceleration = 0.f;

  if (time_since(state.game_time_at_start_of_turn) < 0.4f) {
    astro_acceleration = astro_start_rate;
  }
  else if (abs(state.astro_turn_speed) != 0.f) {
    astro_acceleration = astro_travel_rate;
  }

  // Handle reverse/forward turn actions
  state.astro_turn_speed -= tachyon->left_trigger * astro_acceleration * dt;
  state.astro_turn_speed += tachyon->right_trigger * astro_acceleration * dt;

  // Prevent time changes past max time
  if (state.astro_time >= max_astro_time && state.astro_turn_speed > 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn >= -1.f) {
      // @todo fix sound not playing
      UISystem::ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
    }
  }

  // Prevent time changes before min time
  if (state.astro_time <= min_astro_time && state.astro_turn_speed < 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn <= min_astro_time + 1.f) {
      // @todo fix sound not playing
      UISystem::ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
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

      state.astro_turn_speed *= 1.f - slowdown_factor * dt;

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

      state.astro_turn_speed *= 1.f - slowdown_factor * dt;

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
  state.astro_time += state.astro_turn_speed * 100.f * dt;

  // Clamp to min/max astro time
  if (state.astro_time > max_astro_time) state.astro_time = max_astro_time;
  if (state.astro_time < min_astro_time) state.astro_time = min_astro_time;

  if (tachyon->left_trigger == 0.f && tachyon->right_trigger == 0.f) {
    // Reduce turn rate gradually
    state.astro_turn_speed *= 1.f - astro_slowdown_rate * dt;

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
      Sfx::FadeOutSound(SFX_ASTRO_TRAVEL, 500);

      if (state.is_astro_traveling) {
        Sfx::PlaySound(SFX_ASTRO_END, 1.f);
      }

      state.is_astrolabe_stopped = true;
      state.is_astro_traveling = false;
      state.time_warp_end_radius = 0.f;
    }
    else if (started_turning) {
      Sfx::FadeOutSound(SFX_ASTRO_END, 500);
      Sfx::PlaySound(SFX_ASTRO_BEGIN, 0.8f);

      state.is_astrolabe_stopped = false;
      state.is_astro_traveling = false;
    }
    else if (
      astro_acceleration == astro_travel_rate &&
      !state.is_astro_traveling &&
      !state.is_astrolabe_stopped
    ) {
      Sfx::PlaySound(SFX_ASTRO_TRAVEL, 0.8f);

      state.is_astro_traveling = true;
      state.time_warp_start_radius = 0.f;
    }
  }
}

static void HandleSpellControls(Tachyon* tachyon, State& state) {
  if (abs(state.astro_turn_speed) > 0.18f) {
    return;
  }

  // O
  if (did_press_key(tKey::CONTROLLER_B)) {
    SpellSystem::CastStun(tachyon, state);

    Sfx::PlaySound(SFX_SPELL_STUN, 0.5f);
  }

  // Square
  if (did_press_key(tKey::CONTROLLER_X)) {
    SpellSystem::CastHoming(tachyon, state);
  }

  // Triangle
  if (did_press_key(tKey::CONTROLLER_Y)) {
    // @todo
  }
}

static void HandleTargetingControls(Tachyon* tachyon, State& state) {
  // @todo remove click handling
  if (did_press_key(tKey::CONTROLLER_L1) || did_right_click_down()) {
    Targeting::SelectPreviousAccessibleTarget(tachyon, state);
  }

  // @todo remove click handling
  if (did_press_key(tKey::CONTROLLER_R1) || did_left_click_down()) {
    Targeting::SelectNextAccessibleTarget(tachyon, state);
  }
}

void ControlSystem::HandleControls(Tachyon* tachyon, State& state, const float dt) {
  HandlePlayerMovementControls(tachyon, state, dt);
  HandleAstroControls(tachyon, state, dt);
  HandleSpellControls(tachyon, state);
  HandleTargetingControls(tachyon, state);
}