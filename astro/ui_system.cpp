#include "astro/ui_system.h"

using namespace astro;

void UISystem::ShowDialogue(Tachyon* tachyon, State& state, const char* message) {
  if (
    // Don't show new dialogue if we currently have blocking dialogue
    state.has_blocking_dialogue ||
    // Don't re-show currently-displayed dialogue
    state.dialogue_message == message &&
    tachyon->scene.scene_time - state.dialogue_start_time < 6.f
  ) {
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = tachyon->scene.scene_time;
}

void UISystem::ShowBlockingDialogue(Tachyon* tachyon, State& state, const char* message) {
  ShowDialogue(tachyon, state, message);

  state.has_blocking_dialogue = true;
  state.dismissed_blocking_dialogue = false;
}

void UISystem::HandleDialogue(Tachyon* tachyon, State& state) {
  float scene_time = tachyon->scene.scene_time;
  float dialogue_age = scene_time - state.dialogue_start_time;

  if (state.has_blocking_dialogue && !state.dismissed_blocking_dialogue) {
    // Keep the dialogue "alive" by continually updating its start time
    if (dialogue_age > 1.f) {
      state.dialogue_start_time = tachyon->scene.scene_time - 1.f;
    }

    if (did_press_key(tKey::CONTROLLER_A)) {
      state.dismissed_blocking_dialogue = true;

      // Trigger the dialogue fade-out immediately
      state.dialogue_start_time = scene_time - 5.f;
    }
  }

  if (dialogue_age < 6.f) {
    float alpha = 1.f;
    if (dialogue_age < 0.2f) alpha = dialogue_age * 5.f;
    if (dialogue_age > 5.f) alpha = 1.f - (dialogue_age - 5.f);

    Tachyon_DrawUIText(tachyon, state.debug_text_large, {
      .screen_x = tachyon->window_width / 2,
      .screen_y = tachyon->window_height - 150,
      .centered = true,
      .alpha = alpha,
      .string = state.dialogue_message
    });
  } else {
    state.dialogue_message = "";
    state.dialogue_start_time = 0.f;
    state.has_blocking_dialogue = false;
    state.dismissed_blocking_dialogue = false;
  }
}