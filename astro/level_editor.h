#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace LevelEditor {
    void OpenLevelEditor(Tachyon* tachyon, State& state);
    void HandleLevelEditor(Tachyon* tachyon, State& state, const float dt);
    void CloseLevelEditor(Tachyon* tachyon, State& state);
  }
}