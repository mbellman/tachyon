#include "astro/player_wand.h"

using namespace astro;

void PlayerWand::Update(Tachyon* tachyon, State& state) {
  // @todo
}

bool PlayerWand::DidRecentlyPulse(Tachyon* tachyon, State& state) {
  return (
    state.last_wand_light_pulse_time != 0.f &&
    time_since(state.last_wand_light_pulse_time) < 4.f
  );
}

float PlayerWand::GetPulseRadius(Tachyon* tachyon) {
  auto& fx = tachyon->fx;

  if (fx.wand_pulse_alpha == 1.f) {
    return 0.f;
  }

  return sqrtf(fx.wand_pulse_alpha) * 30000.f;
}