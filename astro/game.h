#pragma once

#include "engine/tachyon_types.h"
#include "astro/game_state.h"

namespace astro {
  void InitGame(Tachyon* tachyon, State& state);
  void UpdateGame(Tachyon* tachyon, State& state, const float dt);
}