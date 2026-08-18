#include "engine/tachyon.h"

#include "metro/background_bicycles.h"
#include "metro/bikes/common_bike.h"

using namespace metro;

void BackgroundBicycles::Update(Tachyon* tachyon, State& state) {
  profile("BackgroundBicycles::Update()");

  int32 total_common_bikes = 0;

  for (auto& bike : state.bicycles) {
    switch (bike.type) {
      case COMMON_BIKE:
        CommonBike::Update(tachyon, state, bike, total_common_bikes++);
        break;
      default:
        break;
    }
  }
}

void BackgroundBicycles::SpawnBicycle(Tachyon* tachyon, State& state, Bicycle& bike) {
  // Precompute rotation
  bike.flat_rotation = Quaternion::FromDirection(bike.facing_direction, Y_UP);

  // Initialize previous position
  bike.previous_position = bike.position;

  switch (bike.type) {
    case COMMON_BIKE:
      CommonBike::Spawn(tachyon, state, bike);
      break;
    default:
      break;
  }

  state.bicycles.push_back(bike);
}

void BackgroundBicycles::DestroyBicycle(Tachyon* tachyon, State& state, Bicycle& bike) {
  switch (bike.type) {
    case COMMON_BIKE:
      CommonBike::Destroy(tachyon, state, bike);
      break;
    default:
      break;
  }

  for_reversed(state.bicycles) {
    if (bike.id == state.bicycles[i].id) {
      state.bicycles.erase(state.bicycles.begin() + i);

      break;
    }
  }
}