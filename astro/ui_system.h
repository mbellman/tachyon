#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace UISystem {
    void ShowDialogue(Tachyon* tachyon, State& state, const std::string& message);
  }
}