#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace UISystem {
    void StartDialogueSet(State& state, const std::string& set_name);
    void ShowDialogue(Tachyon* tachyon, State& state, const std::string& message);
    void ShowTransientDialogue(Tachyon* tachyon, State& state, const std::string& message);
    void ShowBlockingDialogue(Tachyon* tachyon, State& state, const std::string& message);
    void HandleDialogue(Tachyon* tachyon, State& state);
  }
}