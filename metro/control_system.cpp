#include "engine/tachyon.h"

#include "metro/control_system.h"

using namespace metro;

const static auto GAMEPAD_X = tKey::CONTROLLER_A;
const static auto GAMEPAD_O = tKey::CONTROLLER_B;
const static auto GAMEPAD_SQUARE = tKey::CONTROLLER_X;
const static auto GAMEPAD_TRIANGLE = tKey::CONTROLLER_Y;

static bool DidPressPedalKey(Tachyon* tachyon) {
  if (did_press_key(GAMEPAD_X)) {
    return true;
  }

  // @todo keyboard support (?)

  return false;
}

// @todo move to utilities or elsewhere
static Bicycle* GetActiveBicycle(State& state) {
  for (auto& bike : state.bicycles) {
    if (bike.id == state.player_bike_id) {
      return &bike;
    }
  }

  return nullptr;
}

void ControlSystem::Update(Tachyon* tachyon, State& state) {
  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    // @todo handle character controls
    return;
  }

  // Handle bicycle controls
  // @todo factor
  // @todo speed dampening
  // @todo steering
  {
    auto& bike = *active_bike;

    if (DidPressPedalKey(tachyon)) {
      bike.velocity += bike.facing_direction * 50000.f * state.dt;
    }

    bike.position += bike.velocity * state.dt;
  }
}