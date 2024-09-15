#pragma once

#include "engine/tachyon.h"

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace Editor {
    void InitializeEditor(Tachyon* tachyon, State& state);
    void HandleEditor(Tachyon* tachyon, State& state, const float dt);
    void EnableEditor(Tachyon* tachyon, State& state);
    void DisableEditor(Tachyon* tachyon, State& state);
  }
}