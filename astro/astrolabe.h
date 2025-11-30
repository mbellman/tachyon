#pragma once

#include "engine/tachyon_types.h"
#include "astro/game_state.h"

namespace astro {
  struct TimePeriods {
    // Age of ???
    float future = 76.f;

    // Age of Auspice
    float present = 0.f;

    // Age of Consortium
    float past = -76.f;

    // Age of Machination
    float distant_past = -157.f;

    // Age of Yore
    float very_distant_past = -230.f;
  };

  static TimePeriods astro_time_periods;

  namespace Astrolabe {
    void Update(Tachyon* tachyon, State& state);
    float GetMaxAstroTime(const State& state);
    float GetMinAstroTime(const State& state);
    float GetMaxTurnSpeed();
  }
}