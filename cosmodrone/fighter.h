#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Fighter {
    void HandleInputs(Tachyon* tachyon, State& state);
    bool IsDoingQuickReversal(const State& state);
  }
}