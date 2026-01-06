#pragma once

#include <string>

#include "astro/game_state.h"

namespace astro {
  namespace GameEvents {
    void StartEvent(Tachyon* tachyon, State& state, const std::string& event_trigger);
    void HandleEvents(Tachyon* tachyon, State& state);
  }
}