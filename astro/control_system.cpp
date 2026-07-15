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
#include "astro/wand_abilities.h"

using namespace astro;

const static float dodge_cooldown_time = 0.3f;
const static float target_jump_cooldown_time = 0.5f;
const static float strong_attack_cooldown_time = 0.5f;

// @todo move to engine
static float Dot(const tVec2f& a, const tVec2f& b) {
  return a.x * b.x + a.y * b.y;
}

static float GetTurnDot(Tachyon* tachyon, State& state) {
  tVec2f stick_direction = tachyon->left_stick;

  if (stick_direction.x == 0.f && stick_direction.y == 0.f) {
    return 0.f;
  }

  stick_direction = stick_direction.unit();
  tVec3f facing_direction = state.player_facing_direction.xz().unit();

  return Dot(
    stick_direction,
    tVec2f(facing_direction.x, facing_direction.z)
  );
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

static void HandlePlayerMovementControls(Tachyon* tachyon, State& state) {
  // Disable movement during:
  if (
    // Astro travel
    abs(state.astro_turn_speed) > 0.1f ||
    // Wind chime actions
    (state.last_wind_chimes_action_time != 0.f && time_since(state.last_wind_chimes_action_time) < 4.f) ||
    // Camera events
    state.camera_events.size() > 0 ||
    // Dodge actions
    time_since(state.last_dodge_time) < dodge_cooldown_time ||
    // Target jump actions
    time_since(state.last_target_jump_time) < target_jump_cooldown_time ||
    // Climbing off ladders
    PlayerCharacter::IsClimbingOffLadder(tachyon, state)
  ) {
    return;
  }

  // Ladder controls
  // @todo factor
  {
    if (state.is_on_ladder) {
      float climb_direction = (
        // Up
        tachyon->left_stick.y < 0.f ? 1.f :
        // Down
        tachyon->left_stick.y > 0.f ? -1.f :
        0.f
      );

      float t = fmodf(state.player.rig.next_animation_time, 8.f) / 8.f;
      float alpha = 2.f * t * t_TAU;

      state.player.climb_speed = climb_direction * 4000.f * (0.5f + 0.5f * sinf(alpha));

      if (climb_direction == -1.f && is_key_held(tKey::CONTROLLER_A)) {
        state.player.climb_speed = -8000.f;
      }

      // Null out climbing speed if:
      if (
        // We're beginning our climb down onto something
        state.player.is_turning_to_climb_down ||
        state.player.is_starting_climb_down ||
        // We're trying to climb down immediately after climbing onto the bottom of something
        (climb_direction == -1.f && !state.player.is_starting_climb_down && time_since(state.player.last_climbing_start_time) < 0.5f)
      ) {
        state.player.climb_speed = 0.f;
      }

      state.player_position.y += state.player.climb_speed * state.dt;

      return;
    } else {
      state.player.climb_speed = 0.f;
    }
  }

  bool is_holding_run_button = is_key_held(tKey::CONTROLLER_A) || is_key_held(tKey::SHIFT);

  // Directional movement
  {
    float acceleration =
      is_holding_run_button ? 12000.f :
      state.has_target ? 8000.f :
      4000.f;

    if (time_since(state.last_quick_turn_time) < 0.5f) {
      // In the moments after a quick turn, ramp up from partial to full acceleration
      acceleration *= 0.75f + 0.25f * time_since(state.last_quick_turn_time) / 0.5f;
    }

    // Make sure we can't accelerate beyond our current movement speed.
    // In HandleSpeedDampening(), we actually perform hard speed limiting
    // and slowdown behavior.
    float controlled_speed_limit =
      (is_holding_run_button && state.is_moving_down_slope) ? PlayerCharacter::MAX_RUN_SPEED_DOWN_SLOPES :
      is_holding_run_button ? PlayerCharacter::MAX_RUN_SPEED :
      state.has_target ? PlayerCharacter::MAX_COMBAT_WALK_SPEED :
      PlayerCharacter::MAX_WALK_SPEED;

    if (state.player_velocity.magnitude() < controlled_speed_limit) {
      if (is_key_held(tKey::W)) {
        state.player_velocity += tVec3f(0, 0, -1.f) * acceleration * state.dt;
      }

      if (is_key_held(tKey::A)) {
        state.player_velocity += tVec3f(-1.f, 0, 0) * acceleration * state.dt;
      }

      if (is_key_held(tKey::D)) {
        state.player_velocity += tVec3f(1.f, 0, 0) * acceleration * state.dt;
      }

      if (is_key_held(tKey::S)) {
        state.player_velocity += tVec3f(0, 0, 1.f) * acceleration * state.dt;
      }

      state.player_velocity.x += tachyon->left_stick.x * acceleration * state.dt;
      state.player_velocity.z += tachyon->left_stick.y * acceleration * state.dt;

      // Check our speed after acceleration and cap it at the controlled speed limit
      // if we went over. This allows us to lock our speed to predefined limits, but
      // also allows us to decelerate from run to walking speed if we stop running,
      // without immediately clamping to walk speed.
      float updated_speed = state.player_velocity.magnitude();

      if (updated_speed > controlled_speed_limit) {
        state.player_velocity = (state.player_velocity / updated_speed) * controlled_speed_limit;
      }
    }
  }

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
  if (
    is_holding_run_button &&
    !state.has_target &&
    GetTurnDot(tachyon, state) < -0.7f &&
    get_speed_ratio() < 0.2f
  ) {
    state.last_quick_turn_time = get_scene_time();
  }

  float speed = get_speed();

  // Start quick slowdown actions
  if (
    !state.did_jump_off_ledge &&
    speed > PlayerCharacter::MAX_WALK_SPEED &&
    !is_moving_left_stick()
  ) {
    state.player.is_doing_quick_slowdown = true;
  }

  // Stop quick slowdown actions
  if (is_moving_left_stick() || speed < 10.f) {
    state.player.is_doing_quick_slowdown = false;
  }
}

// @todo dev mode only
static void HandleAstroControls(Tachyon* tachyon, State& state) {
  if (did_press_key(tKey::NUM_1)) {
    state.astro_time = astro_time_periods.future;
    state.target_astro_time = state.astro_time;
  }

  if (did_press_key(tKey::NUM_2)) {
    state.astro_time = astro_time_periods.present;
    state.target_astro_time = state.astro_time;
  }

  if (did_press_key(tKey::NUM_3)) {
    state.astro_time = astro_time_periods.past;
    state.target_astro_time = state.astro_time;
  }

  if (did_press_key(tKey::NUM_4)) {
    state.astro_time = astro_time_periods.distant_past;
    state.target_astro_time = state.astro_time;
  }
}

// @temporary
static void HandleDayNightControls(Tachyon* tachyon, State& state) {
  // Triangle
  if (did_press_key(tKey::CONTROLLER_Y)) {
    state.is_nighttime = !state.is_nighttime;
  }
}


static void HandleWandControls(Tachyon* tachyon, State& state) {
  if (abs(state.astro_turn_speed) != 0.f) return;

  bool has_wand = Items::HasItem(state, MAGIC_WAND);
  float scene_time = get_scene_time();

  // O
  if (did_press_key(tKey::CONTROLLER_B)) {
    if (Items::HasItem(state, ITEM_STUN_SPELL)) {
      SpellSystem::CastStun(tachyon, state);

      Sfx::PlaySound(SFX_SPELL_STUN);
    }
  }

  // Pressing Square
  if (has_wand && did_press_key(tKey::CONTROLLER_X)) {
    if (state.targetable_entities.size() > 0) {
      if (Items::HasItem(state, ITEM_HOMING_SPELL)) {
        // @todo magic weapons
        SpellSystem::CastHoming(tachyon, state);
      }
      else if (time_since(state.last_break_attack_time) < 0.7f) {
        // Break follow-up
        // @todo
        console_log("Break follow-up");
      }
      else if (time_since(state.last_wand_swing_time) < 0.7f) {
        // Follow-up
        // @todo
        console_log("Follow-up");
      }
      else {
        // Before we have magic weapons, swing the wand as a melee weapon
        state.last_wand_swing_time = scene_time;
        state.last_wand_bounce_time = 0.f;

        Combat::HandleWandSwing(tachyon, state);
      }
    }
  }

  // Holding Square
  if (
    has_wand && is_key_held(tKey::CONTROLLER_X) &&
    state.targetable_entities.size() == 0
  ) {
    if (!state.is_holding_up_wand) {
      state.player.last_wand_start_time = scene_time;
    }

    state.is_holding_up_wand = true;
    state.player.last_wand_held_time = scene_time;

    // Reset certain properties relating to movement/stance to avoid
    // complicating certain animation details
    state.player.last_stopped_moving_time = 0.f;
    state.player_idle_stance = 1;

    // Perform wand pulses after holding the wand for a bit
    if (
      time_since(state.player.last_wand_start_time) > 0.4f &&
      time_since(state.last_wand_light_pulse_time) > 4.f
    ) {
      state.last_wand_light_pulse_time = scene_time;

      Sfx::PlaySound(SFX_LIGHT_PULSE, 0.8f);

      // @todo factor
      {
        float r = Tachyon_GetRandom();

        if (r < 0.33f) Sfx::PlaySound(SFX_WAND_HINT_1, 1.f);
        else if (r < 0.66f) Sfx::PlaySound(SFX_WAND_HINT_2, 1.f);
        else Sfx::PlaySound(SFX_WAND_HINT_3, 1.f);
      }
    }

    // Check for hints shortly after wand pulsing
    float time_since_pulse = time_since(state.last_wand_light_pulse_time);

    if (
      time_since_pulse > 0.2f &&
      time_since_pulse < 0.3f
    ) {
      WandAbilities::CheckForHints(tachyon, state);
    }
  } else {
    state.is_holding_up_wand = false;
    state.player.last_wand_start_time = 0.f;
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

static void HandleSpeedDampening(Tachyon* tachyon, State& state) {
  bool is_dodging = time_since(state.last_dodge_time) < dodge_cooldown_time;
  bool is_target_jumping = time_since(state.last_target_jump_time) < target_jump_cooldown_time;
  bool is_doing_wind_chimes_action = time_since(state.last_wind_chimes_action_time) < 4.f;
  bool is_climbing_off_ladder = PlayerCharacter::IsClimbingOffLadder(tachyon, state);

  if (state.is_on_ladder) {
    state.player_velocity *= 1.f - 20.f * state.dt;
  }
  else if (is_climbing_off_ladder) {
    if (state.did_climb_down) {
      state.player_velocity *= 1.f - 6.f * state.dt;
    } else {
      state.player_velocity *= 1.f - 6.f * state.dt;
    }
  }
  else if (is_target_jumping) {
    state.player_velocity *= 1.f - 2.f * state.dt;
  }
  else if (is_dodging) {
    state.player_velocity *= 1.f - 4.f * state.dt;
  }
  else if (is_doing_wind_chimes_action) {
    state.player_velocity *= 1.f - 6.f * state.dt;
  }
  else {
    // Standard slowdown
    state.player_velocity *= 1.f - 5.f * state.dt;
  }

  float speed = state.player_velocity.magnitude();
  float speed_limit = PlayerCharacter::MAX_RUN_SPEED;

  if (speed > speed_limit && !is_dodging && !is_target_jumping && !is_climbing_off_ladder) {
    tVec3f unit_velocity = state.player_velocity / speed;

    state.player_velocity = unit_velocity * speed_limit;
  }

  // While moving, reset the last stopped moving time
  if (speed > 0.f) {
    state.player.last_stopped_moving_time = 0.f;
  }

  if (
    speed < 50.f &&
    speed != 0.f &&
    !is_moving_left_stick()
  ) {
    state.player_velocity = tVec3f(0.f);

    if (!state.is_on_ladder && !is_climbing_off_ladder) {
      // As long as we're not climbing or climbing off a ladder,
      // count this as a coming to rest action, which triggers
      // various idle character animations.
      state.player.last_stopped_moving_time = get_scene_time();

      // Also reset the last climbing time so the animation system
      // doesn't think we were just climbing something, and suppress
      // the desired idle animation
      state.player.last_climbing_time = 0.f;
    }
  }
}

static void UpdatePlayerPosition(State& state) {
  // @todo remove this * 5.f bit and properly manage velocity
  state.player_position += state.player_velocity * 5.f * state.dt;
}

void ControlSystem::HandleControls(Tachyon* tachyon, State& state) {
  HandleSpeedDampening(tachyon, state);

  // Only allow character controls if:
  if (
    // We do not have, or have dismissed blocking dialogue
    (!state.has_blocking_dialogue || state.dismissed_blocking_dialogue) &&
    // We aren't doing a strong attack
    time_since(state.last_strong_attack_time) > 0.5f &&
    // We aren't currently recovering from being hit
    time_since(state.last_damage_time) > 0.5f &&
    // We aren't dead
    state.player_hp > 0.f
  ) {
    HandlePlayerMovementControls(tachyon, state);
    HandleAstroControls(tachyon, state); // @temporary
    HandleDayNightControls(tachyon, state);
    HandleWandControls(tachyon, state);
    HandleTargetingControls(tachyon, state);
  }

  UpdatePlayerPosition(state);
}