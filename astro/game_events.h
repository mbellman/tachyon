#pragma once

#include <string>

#include "astro/game_state.h"

namespace astro {
  namespace GameEvents {
    void ProcessEvent(Tachyon* tachyon, State& state, const std::string& event_trigger);
  }
}