#pragma once

#include "engine/tachyon.h"
#include "astro/game_state.h"
#include "astro/astrolabe.h"

namespace astro {
  namespace TimeEvolution {
    void StartAstroTraveling(Tachyon* tachyon, State& state, const float target_time);
    void HandleAstroTravel(State& state);
    void UpdateAstroTime(Tachyon* tachyon, State& state);
  }
}