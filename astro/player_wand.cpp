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

WandPulse PlayerWand::GetPulse(Tachyon* tachyon) {
  auto& fx = tachyon->fx;

  return {
    .position = fx.wand_pulse_position,
    .radius = sqrtf(fx.wand_pulse_alpha) * 30000.f
  };
}