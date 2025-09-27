#include "astro/control_system.h"
#include "astro/spell_system.h"

using namespace astro;

// @todo move elsewhere
static void ShowDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (
    state.dialogue_message == message &&
    tachyon->running_time - state.dialogue_start_time < 6.f
  ) {
    // Don't re-show currently-displayed dialogue
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = tachyon->running_time;
}

static void HandlePlayerMovementControls(Tachyon* tachyon, State& state, const float dt) {
  if (
    tachyon->left_trigger == 0.f &&
    tachyon->right_trigger == 0.f &&
    abs(state.astro_turn_speed) < 0.1f
  ) {
    if (is_key_held(tKey::ARROW_UP) || is_key_held(tKey::W)) {
      state.player_velocity += tVec3f(0, 0, -1.f) * 10000.f * dt;
    }

    if (is_key_held(tKey::ARROW_LEFT) || is_key_held(tKey::A)) {
      state.player_velocity += tVec3f(-1.f, 0, 0) * 10000.f * dt;
    }

    if (is_key_held(tKey::ARROW_RIGHT) || is_key_held(tKey::D)) {
      state.player_velocity += tVec3f(1.f, 0, 0) * 10000.f * dt;
    }

    if (is_key_held(tKey::ARROW_DOWN) || is_key_held(tKey::S)) {
      state.player_velocity += tVec3f(0, 0, 1.f) * 10000.f * dt;
    }

    state.player_velocity.x += tachyon->left_stick.x * 10000.f * dt;
    state.player_velocity.z += tachyon->left_stick.y * 10000.f * dt;
  }

  state.player_velocity *= 1.f - 10.f * dt;
}

static void HandleAstroControls(Tachyon* tachyon, State& state, const float dt) {
  const float astro_turn_rate = 0.8f;
  const float astro_slowdown_rate = 3.f;

  // @todo increase this once the appropriate item is obtained
  float max_astro_time = 0.f;
  // @todo decrease this once the appropriate item is obtained
  float min_astro_time = -158.f;

  bool started_turning = (
    (state.last_frame_left_trigger == 0.f && tachyon->left_trigger != 0.f) ||
    (state.last_frame_right_trigger == 0.f && tachyon->right_trigger != 0.f)
  );

  // Track the initial time when we start turning
  if (started_turning) {
    state.astro_time_at_start_of_turn = state.astro_time;
  }

  // Handle reverse/forward turn actions
  state.astro_turn_speed -= tachyon->left_trigger * astro_turn_rate * dt;
  state.astro_turn_speed += tachyon->right_trigger * astro_turn_rate * dt;

  // Prevent time changes past max time
  if (state.astro_time > max_astro_time && state.astro_turn_speed > 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn >= -1.f) {
      ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
    }
  }

  // Prevent time changes before min time
  if (state.astro_time < min_astro_time && state.astro_turn_speed < 0.f) {
    state.astro_turn_speed = 0.f;

    if (state.astro_time_at_start_of_turn <= min_astro_time + 1.f) {
      ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
    }
  }

  // Slow down toward max time
  if (state.astro_turn_speed > 0.f) {
    float slowdown_threshold = max_astro_time - (max_astro_time - state.astro_time_at_start_of_turn) * 0.1f;

    if (slowdown_threshold < max_astro_time - 5.f) {
      // Enforce a limit on how far away the slowdown threshold is
      // from the stopping value
      slowdown_threshold = max_astro_time - 5.f;
    }

    if (state.astro_time > slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = max_astro_time - slowdown_threshold;
      float slowdown_factor = 40.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * dt;

      ShowDialogue(tachyon, state, "The astrolabe stopped turning.");
    }
  }

  // Slow down toward min time
  if (state.astro_turn_speed < 0.f) {
    float slowdown_threshold = min_astro_time + abs(state.astro_time_at_start_of_turn - min_astro_time) * 0.1f;

    if (slowdown_threshold > min_astro_time + 5.f) {
      // Enforce a limit on how far away the slowdown threshold is
      // from the stopping value
      slowdown_threshold = min_astro_time + 5.f;
    }

    if (state.astro_time < slowdown_threshold) {
      float threshold_distance = abs(slowdown_threshold - state.astro_time);
      float threshold_to_limit = abs(min_astro_time - slowdown_threshold);
      float slowdown_factor = 40.f * powf(threshold_distance / threshold_to_limit, 2.f);

      state.astro_turn_speed *= 1.f - slowdown_factor * dt;

      if (state.astro_time_at_start_of_turn > min_astro_time + 1.f) {
        ShowDialogue(tachyon, state, "The astrolabe stopped turning.");
      }
    }
  }

  state.astro_time += state.astro_turn_speed * 100.f * dt;

  // Reduce turn rate gradually
  state.astro_turn_speed *= 1.f - astro_slowdown_rate * dt;

  // Stop turning at a low enough speed
  if (abs(state.astro_turn_speed) < 0.001f) {
    state.astro_turn_speed = 0.f;
  }

  state.last_frame_left_trigger = tachyon->left_trigger;
  state.last_frame_right_trigger = tachyon->right_trigger;
}

static void HandleSpellControls(Tachyon* tachyon, State& state) {
  // O
  if (did_press_key(tKey::CONTROLLER_B)) {
    SpellSystem::CastStun(tachyon, state);
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

void ControlSystem::HandleControls(Tachyon* tachyon, State& state, const float dt) {
  HandlePlayerMovementControls(tachyon, state, dt);
  HandleAstroControls(tachyon, state, dt);
  HandleSpellControls(tachyon, state);
}