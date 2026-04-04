#include "astro/ui_system.h"

using namespace astro;

template<class T>
static bool MapHasKey(const std::unordered_map<std::string, T>& map, const std::string& key) {
  return map.find(key) != map.end();
}

static void CompleteCurrentDialogueSet(State& state) {
  state.current_dialogue_set = "";
  state.current_dialogue_step = 0;
}

static void HandleCurrentDialogueSet(Tachyon* tachyon, State& state) {
  auto& dialogue_set = state.npc_dialogue[state.current_dialogue_set];

  // Handle advancing dialogue
  if (did_press_key(tKey::CONTROLLER_A)) {
    if (dialogue_set.random) {
      CompleteCurrentDialogueSet(state);

      return;
    } else {
      // Show next dialogue line
      state.current_dialogue_step++;
    }
  }

  // Handle dialogue ending
  if (state.current_dialogue_step > int32(dialogue_set.lines.size() - 1)) {
    CompleteCurrentDialogueSet(state);

    return;
  }

  // Show current line
  auto& current_dialogue_line = dialogue_set.lines[state.current_dialogue_step];

  // @todo handle event triggers

  UISystem::ShowBlockingDialogue(tachyon, state, current_dialogue_line);
}

static void InitiateDialogueSet(State& state, DialogueSet& dialogue_set) {
  if (dialogue_set.random) {
    // For random dialogue, start on a random step in the sequence
    state.current_dialogue_step = Tachyon_GetRandom(0, dialogue_set.lines.size() - 1);
  } else if (dialogue_set.invoked) {
    // If the set was invoked before, begin on the first line for returning interactions
    state.current_dialogue_step = dialogue_set.returning_first_line_index;
  } else {
    // Start from the beginning
    state.current_dialogue_step = 0;
  }

  dialogue_set.invoked = true;
}

void UISystem::StartDialogueSet(State& state, const std::string& set_name) {
  if (!MapHasKey(state.npc_dialogue, set_name)) {
    // @todo dev mode only
    console_log("Dialogue set '" + set_name + "' not found.");

    return;
  }

  auto& dialogue_set = state.npc_dialogue[set_name];

  state.current_dialogue_set = set_name;

  if (dialogue_set.invoked) {
    // Check to see if secondary dialogue is defined for the subject;
    // if so, use that dialogue instead. Dialogue sets can be chained
    // using additional "+" qualifiers for the secondary set names.
    std::string secondary_set_name = set_name + "+";

    if (MapHasKey(state.npc_dialogue, secondary_set_name)) {
      // If there is a follow-up dialogue set available after having
      // invoked the dialogue first, use that instead
      UISystem::StartDialogueSet(state, secondary_set_name);

      return;
    }
  }

  InitiateDialogueSet(state, state.npc_dialogue[set_name]);
}

void UISystem::ShowDialogue(Tachyon* tachyon, State& state, const std::string& message, const float duration) {
  if (
    // Don't show new dialogue if we currently have blocking dialogue
    state.has_blocking_dialogue ||
    // Don't re-show currently-displayed dialogue
    (
      state.dialogue_message == message &&
      time_since(state.dialogue_start_time) < state.dialogue_duration
    )
  ) {
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = get_scene_time();
  state.dialogue_duration = duration;
}

void UISystem::ShowTransientDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (state.has_blocking_dialogue) {
    // Require blocking dialogue to be dismissed beforehand
    return;
  }

  state.dialogue_message = message;
  state.dialogue_duration = 5.f;

  // Trigger the dialogue fade-out immediately.
  // The dialogue will only remain while the method
  // is continuously invoked. As soon as it stops
  // invoking, the dialogue will fade out.
  state.dialogue_start_time = get_scene_time() - 4.f;
}

void UISystem::ShowBlockingDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (state.dialogue_message == message) {
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = get_scene_time();
  state.dialogue_duration = 5.f;
  state.has_blocking_dialogue = true;
  state.dismissed_blocking_dialogue = false;
}

void UISystem::HandleDialogue(Tachyon* tachyon, State& state) {
  const float max_overlay_opacity = 0.4f;

  auto& fx = tachyon->fx;
  float dialogue_age = time_since(state.dialogue_start_time);

  if (state.has_blocking_dialogue && !state.dismissed_blocking_dialogue) {
    // Keep the dialogue "alive" by continually updating its start time
    if (dialogue_age > 1.f) {
      state.dialogue_start_time = get_scene_time() - 1.f;
    }

    if (did_press_key(tKey::CONTROLLER_A)) {
      state.dismissed_blocking_dialogue = true;

      // Trigger the dialogue fade-out immediately
      state.dialogue_start_time = get_scene_time() - 4.f;
    }

    // Show dialogue overlay
    fx.dialogue_overlay_opacity = Tachyon_Lerpf(fx.dialogue_overlay_opacity, max_overlay_opacity, 5.f * state.dt);
  }

  if (state.dialogue_start_time != 0.f && dialogue_age < state.dialogue_duration) {
    float fade_out_time = state.dialogue_duration - 1.f;
    float alpha = 1.f;
    // Fade in over 0.2 seconds
    if (dialogue_age < 0.2f) alpha = dialogue_age / 0.2f;
    // Fade out over the last second of dialogue
    if (dialogue_age > fade_out_time) alpha = 1.f - (dialogue_age - fade_out_time);

    Tachyon_DrawUIText(tachyon, state.debug_text_large, {
      .screen_x = tachyon->window_width / 2,
      .screen_y = tachyon->window_height - 150,
      .centered = true,
      .alpha = alpha,
      .string = state.dialogue_message
    });

    // Show dialogue overlay
    fx.dialogue_overlay_opacity = Tachyon_Lerpf(fx.dialogue_overlay_opacity, max_overlay_opacity, 5.f * state.dt);
  } else {
    // Clear dialogue upon finishing
    state.dialogue_message = "";
    state.dialogue_start_time = 0.f;
    state.has_blocking_dialogue = false;
    state.dismissed_blocking_dialogue = false;

    // Hide dialogue overlay
    fx.dialogue_overlay_opacity = Tachyon_Lerpf(fx.dialogue_overlay_opacity, 0.f, 5.f * state.dt);
  }

  if (state.current_dialogue_set != "") {
    HandleCurrentDialogueSet(tachyon, state);
  }
}

void UISystem::HandleHUD(Tachyon* tachyon, State& state) {
  float player_speed = state.player_velocity.magnitude();

  if (
    player_speed > 1.f ||
    state.astro_turn_speed != 0.f ||
    state.targetable_entities.size() > 0 ||
    state.dialogue_message != ""
  ) {
    // Fade out
    state.ui.title_alpha = Tachyon_Lerpf(state.ui.title_alpha, 0.f, 10.f * state.dt);
  } else {
    // Fade in
    state.ui.title_alpha = Tachyon_Lerpf(state.ui.title_alpha, 1.f, state.dt);
  }

  // Current location
  Tachyon_DrawUIElement(tachyon, state.ui.divination_woodrealm_title, {
    .screen_x = int32(float(tachyon->window_width) * 0.155f),
    .screen_y = tachyon->window_height - 110,
    .centered = false,
    .alpha = state.ui.title_alpha
  });

  // Current astro age
  if (state.astro_time > -35.f) {
    Tachyon_DrawUIElement(tachyon, state.ui.present_age_title, {
      .screen_x = int32(float(tachyon->window_width) * 0.165f),
      .screen_y = tachyon->window_height - 185,
      .centered = false,
      .alpha = sqrtf(state.ui.title_alpha)
    });
  } else {
    Tachyon_DrawUIElement(tachyon, state.ui.past_age_title, {
      .screen_x = int32(float(tachyon->window_width) * 0.165f),
      .screen_y = tachyon->window_height - 185,
      .centered = false,
      .alpha = sqrtf(state.ui.title_alpha)
    });
  }
}