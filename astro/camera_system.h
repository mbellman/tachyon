#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace CameraSystem {
    void UpdateCamera(Tachyon* tachyon, State& state, const float dt);
  }
}