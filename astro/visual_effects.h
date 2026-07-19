#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace VisualEffects {
    void Update(Tachyon* tachyon, State& state);
    void SpawnDustCloud(Tachyon* tachyon, State& state, const tVec3f& position, const float delay = 0.f);
    void SpawnDustCloudsAroundPlayer(Tachyon* tachyon, State& state);
  }
}