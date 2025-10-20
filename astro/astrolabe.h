#pragma once

#include "engine/tachyon_types.h"
#include "astro/game_state.h"

namespace astro {
  struct TimePeriods {
    float future = 76.f;
    float age_of_auspice = 0.f;
    float past = -76.f;
    float distant_past = -157.f;
    float age_of_yore = -230.f;
  };

  static TimePeriods astro_time_periods;

  namespace Astrolabe {
    void Update(Tachyon* tachyon, State& state);
    float GetMaxAstroTime(const State& state);
    float GetMinAstroTime(const State& state);
  }
}