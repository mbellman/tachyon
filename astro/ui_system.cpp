#include "astro/ui_system.h"

using namespace astro;

static void CompleteCurrentDialogueSet(State& state) {
  state.current_dialogue_set = "";
  state.current_dialogue_step = 0;
}

static void HandleCurrentDialogueSet(Tachyon* tachyon, State& state) {
  auto& dialogue_set = state.npc_dialogue[state.current_dialogue_set];

  if (did_press_key(tKey::CONTROLLER_A)) {
    if (dialogue_set.random) {
      CompleteCurrentDialogueSet(state);

      return;
    } else {
      // Show next dialogue line
      state.current_dialogue_step++;
    }
  }

  if (state.current_dialogue_step > dialogue_set.lines.size() - 1) {
    CompleteCurrentDialogueSet(state);

    return;
  }

  auto& current_dialogue_line = dialogue_set.lines[state.current_dialogue_step];

  UISystem::ShowBlockingDialogue(tachyon, state, current_dialogue_line);
}

void UISystem::StartDialogueSet(State& state, const std::string set_name) {
  if (state.npc_dialogue.find(set_name) != state.npc_dialogue.end()) {
    state.current_dialogue_set = set_name;
    state.current_dialogue_step = 0;

    auto& dialogue_set = state.npc_dialogue[set_name];

    if (dialogue_set.random) {
      // For random dialogue, start on a random step in the sequence
      state.current_dialogue_step = Tachyon_GetRandom(0, dialogue_set.lines.size() - 1);
    }
  } else if (set_name != "") {
    // @todo dev mode only
    console_log("Dialogue set '" + set_name + "' not found.");
  }
}

void UISystem::ShowDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (
    // Don't show new dialogue if we currently have blocking dialogue
    state.has_blocking_dialogue ||
    // Don't re-show currently-displayed dialogue
    (
      state.dialogue_message == message &&
      time_since(state.dialogue_start_time) < 6.f
    )
  ) {
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = get_scene_time();
}

void UISystem::ShowTransientDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (state.has_blocking_dialogue) {
    // Require blocking dialogue to be dismissed beforehand
    return;
  }

  state.dialogue_message = message;

  // Trigger the dialogue fade-out immediately.
  // The dialogue will only remain while the method
  // is continuously invoked.
  state.dialogue_start_time = get_scene_time() - 5.f;
}

void UISystem::ShowBlockingDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (state.dialogue_message == message) {
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = get_scene_time();
  state.has_blocking_dialogue = true;
  state.dismissed_blocking_dialogue = false;
}

void UISystem::HandleDialogue(Tachyon* tachyon, State& state) {
  float dialogue_age = time_since(state.dialogue_start_time);

  if (state.has_blocking_dialogue && !state.dismissed_blocking_dialogue) {
    // Keep the dialogue "alive" by continually updating its start time
    if (dialogue_age > 1.f) {
      state.dialogue_start_time = get_scene_time() - 1.f;
    }

    if (did_press_key(tKey::CONTROLLER_A)) {
      state.dismissed_blocking_dialogue = true;

      // Trigger the dialogue fade-out immediately
      state.dialogue_start_time = get_scene_time() - 5.f;
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

  if (state.current_dialogue_set != "") {
    HandleCurrentDialogueSet(tachyon, state);
  }
}