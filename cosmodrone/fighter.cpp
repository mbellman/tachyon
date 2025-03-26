#include "cosmodrone/bullets.h"
#include "cosmodrone/fighter.h"

using namespace Cosmodrone;

static void HandleMachineGunFire(Tachyon* tachyon, State& state) {
  Bullets::FireBullet(tachyon, state);
}

static void HandleMissileFire(Tachyon* tachyon, State& state) {
  Bullets::FireMissile(tachyon, state);
}

void Fighter::HandleInputs(Tachyon* tachyon, State& state) {
  if (is_left_mouse_held_down()) {
    HandleMachineGunFire(tachyon, state);
  }

  if (did_right_click_down()) {
    HandleMissileFire(tachyon, state);
  }
}

bool Fighter::IsDoingQuickReversal(const State& state) {
  return state.current_game_time - state.last_fighter_reversal_time < 2.f;
}