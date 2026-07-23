#include "engine/tachyon.h"

#include "metro/background_bicycles.h"
#include "metro/bikes/common_bike.h"

using namespace metro;

void BackgroundBicycles::Update(Tachyon* tachyon, State& state) {
  profile("BackgroundBicycles::Update()");

  int32 total_common_bikes = 0;

  for (auto& bicycle : state.bicycles) {
    switch (bicycle.type) {
      case BicycleType::COMMON_BIKE:
        CommonBike::Update(tachyon, state, bicycle, total_common_bikes++);
        break;
      default:
        break;
    }
  }
}

void BackgroundBicycles::SpawnBicycle(Tachyon* tachyon, State& state, const Bicycle& bike) {
  switch (bike.type) {
    case BicycleType::COMMON_BIKE:
      CommonBike::Spawn(tachyon, state, bike);
      break;
    default:
      break;
  }

  state.bicycles.push_back(bike);
}