#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace Targeting {
    void HandleTargets(Tachyon* tachyon, State& state);
    void SetSpeakingEntity(State& state, EntityRecord& record);
    void SelectClosestAccessibleTarget(Tachyon* tachyon, State& state);
    void SelectNextClosestAccessibleTarget(Tachyon* tachyon, State& state);
    void DeselectCurrentTarget(Tachyon* tachyon, State& state);
  }
}