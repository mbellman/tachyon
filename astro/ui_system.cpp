#include "astro/ui_system.h"

using namespace astro;

void UISystem::ShowDialogue(Tachyon* tachyon, State& state, const char* message) {
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