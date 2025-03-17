#include "cosmodrone/bullets.h"
#include "cosmodrone/fighter.h"

using namespace Cosmodrone;

static void HandleMachineGunFire(Tachyon* tachyon, State& state) {
  // @todo
  console_log("Machine gun!");
}

static void HandleMissileFire(Tachyon* tachyon, State& state) {
  // @todo
  console_log("Missile!");
}

void Fighter::HandleInputs(Tachyon* tachyon, State& state) {
  if (is_left_mouse_held_down()) {
    HandleMachineGunFire(tachyon, state);
  }

  if (did_right_click_down()) {
    HandleMissileFire(tachyon, state);
  }
}