#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace UISystem {
    void ShowDialogue(Tachyon* tachyon, State& state, const char* message);
    void ShowBlockingDialogue(Tachyon* tachyon, State& state, const char* message);
    void HandleDialogue(Tachyon* tachyon, State& state);
  }
}