#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"

namespace astro {
  namespace Targeting {
    void HandleTargets(Tachyon* tachyon, State& state);
    void SetSpeakingEntity(State& state, EntityRecord& record);
    void SelectNextAccessibleTarget(Tachyon* tachyon, State& state);
    void SelectPreviousAccessibleTarget(Tachyon* tachyon, State& state);
    void DeselectCurrentTarget(Tachyon* tachyon, State& state);
    bool IsInCombatMode(State& state);
  }
}