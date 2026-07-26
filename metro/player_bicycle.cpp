#include "engine/tachyon.h"

#include "metro/player_bicycle.h"
#include "metro/bikes/common_bike.h"
#include "metro/utilities.h"

using namespace metro;

void PlayerBicycle::Update(Tachyon* tachyon, State& state) {
  profile("PlayerBicycle::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    return;
  }

  switch (active_bike->type) {
    case BicycleType::COMMON_BIKE:
      CommonBike::HandlePhysics(tachyon, state, *active_bike);
      CommonBike::Update(tachyon, state, *active_bike, state.player_bike_index);
      break;
    default:
      break;
  }
}