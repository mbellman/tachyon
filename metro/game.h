#pragma once

#include "metro/game_state.h"

namespace metro {
  void Init(Tachyon* tachyon, State& state);
  void Update(Tachyon* tachyon, State& state, const float dt);
}